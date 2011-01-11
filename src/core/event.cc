/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#include "clusterlibinternal.h"

namespace clusterlib {

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
            result = m_waitCond.waitMsecs(m_waitMutex, remainingMs);
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

}       /* End of 'namespace clusterlib' */

