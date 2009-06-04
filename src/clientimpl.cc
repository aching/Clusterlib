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

namespace clusterlib
{

Root *
ClientImpl::getRoot()
{
    return getDelegate()->getRoot();
}

void
ClientImpl::sendEvent(UserEventPayload *cepp)
{
    TRACE(CL_LOG, "sendEvent");

    m_queue.put(cepp);
}

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
             "this: 0x%x, thread: 0x%x",
             (int32_t) this,
             (uint32_t) pthread_self());

    bool endEventReceived = false;
    string rootKey = NotifyableKeyManipulator::createRootKey();
    while (endEventReceived != true) {
	uepp = m_queue.take();

        /*
         * Exit on NULL user event payload, this is a signal from
         * the Factory to terminate.
         */
	if (uepp == NULL) {
            throw InconsistentInternalStateException(
                "ConsumeUserEvents: Got a NULL UserEventPayload!");
	}

	LOG_DEBUG(CL_LOG,
                  "ConsumeUserEvents: Received user event 0x%x with "
                  "path %s and Event %d",
                  (int32_t) uepp,
                  uepp->getKey().c_str(),
                  uepp->getEvent());

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
             "this = 0x%x, thread = 0x%x",
             (int32_t) this,
             (uint32_t) pthread_self());
}

/*
 * Call all handlers for the given Notifyable and user Event.
 */
void
ClientImpl::dispatchHandlers(const string &key, Event e)
{
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
             "dispatchHandlers: Looking for handlers for event: %d on: %s",
             e, key.c_str());

    {
        Locker l1(getEventHandlersLock());

        /*
         * If there are no handlers registered for this key,
         * then punt.
         */
        if (range.first == m_eventHandlers.end()) {
            LOG_INFO(CL_LOG,
                     "dispatchHandlers: No handlers found for event %d on %s",
                     e, key.c_str());
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
             "dispatchEventHandlers: Found %d handlers for event %d on %s",
             counter, e, key.c_str());

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

    LOG_WARN(CL_LOG,
             "Registering handler for %s",
             uehp->getNotifyable()->getKey().c_str());

    Locker l1(getEventHandlersLock());

    m_eventHandlers.insert(pair<const string, UserEventHandler *>
                           (uehp->getNotifyable()->getKey(), uehp));
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

    for (ehIt = range.first; ehIt != range.second; ehIt++) {
        if ((*ehIt).second == uehp) {
            m_eventHandlers.erase(ehIt);
            return true;
        }
    }
    return false;
}

/*
 * Register a timer handler.
 */
TimerId
ClientImpl::registerTimer(TimerEventHandler *tehp,
                          uint64_t afterTime,
                          ClientData data)
{
    TRACE(CL_LOG, "registerTimer");

    return mp_f->registerTimer(tehp, afterTime, data);
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
