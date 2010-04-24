/*
 * clientimpl.cc --
 *
 * Implementation of the Client class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace json;

namespace clusterlib
{

Root *
ClientImpl::getRoot()
{
    return getOps()->getRoot();
}

void
ClientImpl::sendEvent(UserEventPayload *cepp)
{
    TRACE(CL_LOG, "sendEvent");

    m_queue.put(cepp);
}

ClientImpl::~ClientImpl()
{
    /*
     * Wait till all events have been handled.
     */
    m_predMutexCond.predSignal();
    m_eventThread.Join();

    /*
     * Clean up.
     */
    cancelJSONRPCMethodHandler();
    cancelJSONRPCResponseHandler();
    Locker l(getEventHandlersLock());
    m_firstTimeEventHandlers.clear();
    m_eventHandlers.clear();
}

uint64_t
ClientImpl::fetchAndIncrRequestCounter()
{
    TRACE(CL_LOG, "fetchAndIncrRequestCounter");
    
    Locker l(&m_jsonRPCRequestCounterMutex);
    return m_jsonRPCRequestCounter++;
}

/**
 * Wait up to one second.
 */
static const int64_t eventMsecTimeout = 1000;

/*
 * The following method runs in the event handling thread,
 * invoking handlers for user events as they come in.
 */
void
ClientImpl::consumeUserEvents(void *param)
{
    TRACE(CL_LOG, "consumeUserEvents");

    UserEventPayload *uepp;

    LOG_INFO(CL_LOG,
             "Starting thread with ClientImpl::consumeUserEvents(), "
             "this: %p, thread: %" PRIu32,
             this,
             static_cast<uint32_t>(pthread_self()));

    bool endEventReceived = false;
    string rootKey = NotifyableKeyManipulator::createRootKey();
    while (!m_predMutexCond.predWaitMsecs(0) && (endEventReceived != true)) {
        /*
         * Run all the handlers that have initialRun true and then
         * move them into the regular user event multimap.
         */
        {
            Locker l1(getEventHandlersLock());
            Event userEventMask;
            vector<UserEventHandler *>::iterator ftEhIt;
            for (ftEhIt = m_firstTimeEventHandlers.begin();
                 ftEhIt != m_firstTimeEventHandlers.end();
                 ++ftEhIt) {
                userEventMask = (*ftEhIt)->getMask();
                (*ftEhIt)->handleUserEventDelivery(userEventMask);
                m_eventHandlers.insert(
                    pair<const string, UserEventHandler *>
                    ((*ftEhIt)->getNotifyable()->getKey(), (*ftEhIt)));
            }
            m_firstTimeEventHandlers.clear();
        }

        LOG_DEBUG(CL_LOG,
                  "consumeUserEvents: Waiting for %" PRId64 " msecs to take "
                  "from the event queue...",
                  eventMsecTimeout);
	if (!m_queue.takeWaitMsecs(eventMsecTimeout, uepp)) {
            continue;
        }

        /*
         * Exit on NULL user event payload, this is a signal from
         * the Factory to terminate.
         */
	if (uepp == NULL) {
            throw InconsistentInternalStateException(
                "ConsumeUserEvents: Got a NULL UserEventPayload!");
	}

	LOG_DEBUG(CL_LOG,
                  "ConsumeUserEvents: Received user event %p with "
                  "path %s and Event %s",
                  uepp,
                  uepp->getKey().c_str(),
                  UserEventHandler::getEventsString(uepp->getEvent()).c_str());

        /* Dispatch this event. */
        dispatchHandlers(uepp->getKey(), uepp->getEvent());

        /* Is this is the end event? */
        if ((uepp->getKey().compare(rootKey) == 0) && 
            (uepp->getEvent() == EN_ENDEVENT)) {
            endEventReceived = true;
        }

        /* Recycle the payload. */
	delete uepp;
    }

    LOG_INFO(CL_LOG,
             "Ending thread with ClientImpl::consumeUserEvents(): "
             "this = %p, thread = %" PRIu32,
             this,
             static_cast<uint32_t>(pthread_self()));
}

/*
 * Call all handlers for the given Notifyable and user Event.
 */
void
ClientImpl::dispatchHandlers(const string &key, Event e)
{
    TRACE(CL_LOG, "dispatchHandlers");

    if (key.empty()) {
        LOG_INFO(CL_LOG,
                 "dispatchHandlers: empty key, not dispatching");
        return;
    }

    EventHandlersMultimapRange range = m_eventHandlers.equal_range(key);
    EventHandlersMultimap copy;
    EventHandlersIterator ehIt;
    UserEventHandler *uehp;
    int32_t counter = 0;	/* Debug counter for # of handlers found. */

    LOG_INFO(CL_LOG,
             "dispatchHandlers: Looking for handlers for event: %s on: %s",
             UserEventHandler::getEventsString(e).c_str(), 
             key.c_str());

    {
        Locker l1(getEventHandlersLock());

        /*
         * If there are no handlers registered for this key,
         * then punt.
         */
        if (range.first == m_eventHandlers.end()) {
            LOG_INFO(CL_LOG,
                     "dispatchHandlers: No handlers found for event %s on %s",
                     UserEventHandler::getEventsString(e).c_str(), 
                     key.c_str());
            return;
        }

        /*
         * Make a copy of the range of registered handlers for
         * this Notifyable.
         */
        for (ehIt = range.first; ehIt != range.second; ehIt++) {
            /*
             * Sanity check -- the key must be the same.
             */
            if ((*ehIt).first != key) {
                LOG_FATAL(CL_LOG,
                          "Internal error: bad handler registration %s vs %s",
                          key.c_str(), (*ehIt).first.c_str());
                ::abort();
            }

            uehp = (*ehIt).second;

            /*
             * Sanity check -- the handler must not be NULL.
             */
            if (uehp == NULL) {
                LOG_FATAL(CL_LOG,
                          "Internal error: event handling registered "
                          "with NULL handler");
                ::abort();
            }

            /*
             * If this handler is not for the given event, then skip it.
             */
            if ((uehp->getMask() & e) == 0) {
                continue;
            }

            counter++;

            /*
             * Make a copy of the registration.
             */
            copy.insert(pair<const string, UserEventHandler *>(key, uehp));
        }
    }

    LOG_INFO(CL_LOG,
             "dispatchEventHandlers: Found %d handlers for event %s on %s",
             counter, 
             UserEventHandler::getEventsString(e).c_str(),  
             key.c_str());

    /*
     * If there are no handlers registered for this event on
     * the given Notifyable, then punt.
     */
    if (copy.begin() == copy.end()) {
        return;
    }

    /*
     * Iterate over the registered handlers.
     */
    for (ehIt = copy.begin(); ehIt != copy.end(); ehIt++) {
        uehp = (*ehIt).second;

        /*
         * Now call each handler.
         */
        uehp->handleUserEventDelivery(e);
    }
}

/*
 * Register a handler for a set of events on a given Notifyable. When
 * any of the events are triggered, the handleUserEvent method on
 * the supplied handler is called.
 */
void
ClientImpl::registerHandler(UserEventHandler *uehp)
{
    TRACE(CL_LOG, "registerHandler");

    LOG_INFO(CL_LOG,
             "Registering handler for %s",
             uehp->getNotifyable()->getKey().c_str());

    Locker l1(getEventHandlersLock());
    
    /*
     * If the user event handler is to be run immediately put in
     * m_firstTimeEventHandlers, otherwise add it normally to m_eventHandlers.
     */
    if (uehp->getInitialRun()) {
        m_firstTimeEventHandlers.push_back(uehp);
    }
    else {
        m_eventHandlers.insert(pair<const string, UserEventHandler *>
                               (uehp->getNotifyable()->getKey(), uehp));
    }
}

/*
 * Cancel a handler for a set of events on a given Notifyable.
 * When any of the events are triggered, the handler is no longer
 * called, but other registered handlers may still be called.
 *
 * Returns true if the specified handler was unregistered.
 */
bool
ClientImpl::cancelHandler(UserEventHandler *uehp)
{
    TRACE(CL_LOG, "cancelHandler");

    Locker l1(getEventHandlersLock());
    string key = uehp->getNotifyable()->getKey();
    EventHandlersMultimapRange range = m_eventHandlers.equal_range(key);
    EventHandlersIterator ehIt;

    vector<UserEventHandler *>::iterator ftEhIt;
    ftEhIt = find(m_firstTimeEventHandlers.begin(), 
                  m_firstTimeEventHandlers.end(), 
                  uehp);
    if (ftEhIt != m_firstTimeEventHandlers.end()) {
        m_firstTimeEventHandlers.erase(ftEhIt);
        return true;
    }

    for (ehIt = range.first; ehIt != range.second; ehIt++) {
        if ((*ehIt).second == uehp) {
            m_eventHandlers.erase(ehIt);
            return true;
        }
    }
    return false;
}

void
ClientImpl::registerJSONRPCResponseHandler(Queue *responseQueue,
                                           Queue *completedQueue)
{
    TRACE(CL_LOG, "registerJSONRPCResponseHandler");

    if (m_jsonRPCResponseHandler) {
        throw InvalidMethodException(
            "registerJSONRPCResponseHandler: Already registered!");
    }
    m_jsonRPCResponseHandler = new JSONRPCResponseHandler(
        responseQueue,
        completedQueue,
        this,
        getOps()->getResponseSignalMap());

    registerHandler(m_jsonRPCResponseHandler);
}

bool
ClientImpl::cancelJSONRPCResponseHandler()
{
    TRACE(CL_LOG, "cancelJSONRPCMethodHandler");

    if (m_jsonRPCResponseHandler) {
        bool ret = cancelHandler(m_jsonRPCResponseHandler);
        delete m_jsonRPCResponseHandler;
        m_jsonRPCResponseHandler = NULL;
        return ret;
    }
    else {
        return false;
    }
}

void
ClientImpl::registerJSONRPCMethodHandler(
    ClusterlibRPCManager *rpcManager)
{
    TRACE(CL_LOG, "registerJSONRPCMethodHandler");

    if (m_jsonRPCMethodHandler) {
        throw InvalidMethodException(
            "registerJSONRPCMethodHandler: Already registered!");
    }
    m_jsonRPCMethodHandler = new JSONRPCMethodHandler(rpcManager);

    registerHandler(m_jsonRPCMethodHandler);
}

bool
ClientImpl::cancelJSONRPCMethodHandler()
{
    TRACE(CL_LOG, "cancelJSONRPCMethodHandler");

    if (m_jsonRPCMethodHandler) {
        bool ret = cancelHandler(m_jsonRPCMethodHandler);
        delete m_jsonRPCMethodHandler;
        m_jsonRPCMethodHandler = NULL;
        return ret;
    }
    else {
        return false;
    }
}

/*
 * Register a timer handler.
 */
TimerId
ClientImpl::registerTimer(TimerEventHandler *tehp,
                          uint64_t afterMsecs,
                          ClientData data)
{
    TRACE(CL_LOG, "registerTimer");

    return mp_f->registerTimer(tehp, afterMsecs, data);
}

/*
 * Cancel a timer event. Returns true if the event was successfully
 * cancelled, false otherwise (if false is returned, its possible that
 * the timer event may still be delivered, or it has already been
 * delivered).
 */
bool
ClientImpl::cancelTimer(TimerId id)
{
    TRACE(CL_LOG, "cancelTimer");

    return mp_f->cancelTimer(id);
}

};	/* End of 'namespace clusterlib' */
