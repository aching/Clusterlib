/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "Event"

namespace clusterlib
{

/*
 * Mechanism for delivering user events and for waiting till a condition
 * is satisfied by events.
 */

bool
UserEventHandler::waitUntilCondition(uint64_t maxMs, bool interruptible)
{
    TRACE(CL_LOG, "waitUntilCondition");

    bool result = true;
    int64_t beginWait = TimerService::getCurrentTimeMsecs();
    int64_t endWait;
    uint64_t remainingMs = maxMs;
    uint64_t elapsedMs;

    do {
        /*
         * Wait for a return.
         */
        if (remainingMs == 0) {
            m_waitCond.wait(m_waitMutex);
        } else {
            result = m_waitCond.wait(m_waitMutex, remainingMs);
        }

        /*
         * If interrupts are allowed and we got one, returns false.
         */
        if (interruptible && (errno == EINTR)) {
            return false;
        }

        /*
         * If we timed out, return false.
         */
        if (result == false) {
            return false;
        }

        /*
         * If this is an infinite wait, then
         * return true, because we got signalled.
         */
        if (maxMs == 0) {
            return true;
        }

        /*
         * This is a time-limited wait, so compute how much longer to wait.
         */
        endWait = TimerService::getCurrentTimeMsecs();
        elapsedMs = (uint64_t) (endWait - beginWait);
        if (elapsedMs >= maxMs) {
            remainingMs = 0;
        } else {
            remainingMs = maxMs - elapsedMs;
        }

    } while (remainingMs > 0);

    /*
     * Returns whatever the cond.wait() returned, or the default
     * result value (true).
     */
    return result;
}

void
UserEventHandler::handleUserEventDelivery(Event e)
{
    TRACE(CL_LOG, "handleUserEventDelivery");

    /*
     * Deliver the event to user code.
     */
    handleUserEvent(e);

    /*
     * Prevent new waits while we check if this event
     * satisfies the waiters.
     */
    acquireLock();

    /*
     * See if a condition is satisfied by this event. If
     * yes, wake up everyone.
     */
    if (meetsCondition(e)) {
        resetCondition();
        notifyWaiters();
    }

    /*
     * Let any waiters run.
     */
    releaseLock();
}    

};       /* End of 'namespace clusterlib' */

