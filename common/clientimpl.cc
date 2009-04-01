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

/*
 * The following method runs in the event handling thread,
 * invoking handlers for events as they come in.
 */
void
ClientImpl::consumeClusterEvents()
{
    TRACE(CL_LOG, "consumeClusterEvents");

    ClusterEventPayload *cepp;
    
    LOG_INFO(CL_LOG,
             "Hello from consumeClusterEvents, this: 0x%x, thread: 0x%x",
             (int) this, (int) pthread_self());

    for (;;) {
	cepp = m_queue.take();

        /*
         * Exit on NULL cluster event payload, this is a signal from
         * the Factory to terminate.
         */
	if (cepp == NULL) {
	    return;
	}
	
        /*
         * Dispatch this event.
         */
        dispatchHandlers(cepp->getTarget(), cepp->getEvent());

        /*
         * Recycle the payload.
         */
	delete cepp;
    }
}

/*
 * Call all handlers for the given Notifyable and Event.
 */
void
ClientImpl::dispatchHandlers(Notifyable *np, Event e)
{
    string key = np->getKey();
    EventHandlersMultimapRange range = m_eventHandlers.equal_range(key);
    EventHandlersMultimap copy;
    EventHandlersIterator ehIt;
    ClusterEventHandler *cehp;

    {
        Locker l1(getEventHandlersLock());

        /*
         * If there are no handlers registered for this Notifyable
         * then punt.
         */
        if (range.first == m_eventHandlers.end()) {
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

            /*
             * Make a copy of the registration.
             */
            copy.insert(pair<const string, ClusterEventHandler *>(key, cehp));
        }
    }

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
         * Now call the handler.
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
