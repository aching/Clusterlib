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
ClientImpl::sendEvent(ClusterEventPayload *cepp)
{
    TRACE(CL_LOG, "sendEvent");

    m_queue.put(cepp);
}

/*
 * The following method runs in the event handling thread,
 * invoking handlers for events as they come in.
 */
void
ClientImpl::consumeClusterEvents(void *param)
{
    TRACE(CL_LOG, "consumeClusterEvents");

    ClusterEventPayload *cepp;

    LOG_INFO(CL_LOG,
             "Starting thread with ClientImpl::consumeClusterEvents(), "
             "this: 0x%x, thread: 0x%x",
             (int32_t) this,
             (uint32_t) pthread_self());

    for (;;) {
	cepp = m_queue.take();

        /*
         * Exit on NULL cluster event payload, this is a signal from
         * the Factory to terminate.
         */
	if (cepp == NULL) {
            break;
	}

	LOG_DEBUG(CL_LOG,
                  "ConsumeClusterEvents: Received user event 0x%x with "
                  "path %s and Event %d",
                  (int32_t) cepp,
                  cepp->getKey().c_str(),
                  cepp->getEvent());

        /*
         * Dispatch this event.
         */
        dispatchHandlers(cepp->getKey(), cepp->getEvent());

        /*
         * Recycle the payload.
         */
	delete cepp;
    }

    LOG_INFO(CL_LOG,
             "Ending thread with ClientImpl::consumeClusterEvents(): "
             "this = 0x%x, thread = 0x%x",
             (int32_t) this,
             (uint32_t) pthread_self());
}

/*
 * Call all handlers for the given Notifyable and Event.
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
    ClusterEventHandler *cehp;
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

            cehp = (*ehIt).second;

            /*
             * Sanity check -- the handler must not be NULL.
             */
            if (cehp == NULL) {
                LOG_FATAL(CL_LOG,
                          "Internal error: event handling registered "
                          "with NULL handler");
                ::abort();
            }

            /*
             * If this handler is not for the given event, then skip it.
             */
            if ((cehp->getMask() & e) == 0) {
                continue;
            }

            counter++;

            /*
             * Make a copy of the registration.
             */
            copy.insert(pair<const string, ClusterEventHandler *>(key, cehp));
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
        cehp = (*ehIt).second;

        /*
         * Now call each handler.
         */
        cehp->handleClusterEvent(e);
    }
}

/*
 * Register a handler for a set of events on a given Notifyable. When
 * any of the events are triggered, the handleClusterEvent method on
 * the supplied handler is called.
 */
void
ClientImpl::registerHandler(ClusterEventHandler *cehp)
{
    TRACE(CL_LOG, "registerHandler");

    Locker l1(getEventHandlersLock());

    cerr << "Registering handler for "
         << cehp->getNotifyable()->getKey()
         << endl;

    m_eventHandlers.insert(pair<const string, ClusterEventHandler *>
                           (cehp->getNotifyable()->getKey(), cehp));
}

/*
 * Cancel a handler for a set of events on a given Notifyable.
 * When any of the events are triggered, the handler is no longer
 * called, but other registered handlers may still be called.
 *
 * Returns true if the specified handler was unregistered.
 */
bool
ClientImpl::cancelHandler(ClusterEventHandler *cehp)
{
    TRACE(CL_LOG, "cancelHandler");

    Locker l1(getEventHandlersLock());
    string key = cehp->getNotifyable()->getKey();
    EventHandlersMultimapRange range = m_eventHandlers.equal_range(key);
    EventHandlersIterator ehIt;

    for (ehIt = range.first; ehIt != range.second; ehIt++) {
        if ((*ehIt).second == cehp) {
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
