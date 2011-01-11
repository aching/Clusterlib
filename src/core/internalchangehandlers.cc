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

using namespace std;
using namespace boost;

namespace clusterlib {

Event
InternalChangeHandlers::handlePrecLockNodeExistsChange(int32_t etype,
                                                       const string &key)
{
    /*
     * Handle existence change for preceding lock node. This is called
     * whenever a preceding lock node being watched by a thread in this
     * abdicates.  All it does is signal the lock waiting on it to wake up
     * and try again.
     */

    TRACE(CL_LOG, "handlePrecLockNodeExistsChange");

    LOG_DEBUG(CL_LOG,
              "handlePrecLockNodeExistsChange: %s on key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              key.c_str());

    /*
     * This is the only expected event.
     */
    if (etype == ZOO_DELETED_EVENT) {
        /*
         * Notify the thread waiting to acquire the lock that this
         * lock node has finally been deleted.  Since this
         * PredMutexCond cannot be deleted, this process should be safe.
         */
        WaitMap::iterator waitMapIt;
        {
            Locker l1(getOps()->getDistributedLocks()->getWaitMapLock());
            waitMapIt = 
                getOps()->getDistributedLocks()->getWaitMap()->find(key);
            if (waitMapIt == 
                getOps()->getDistributedLocks()->getWaitMap()->end()) {
                throw InconsistentInternalStateException(
                    "handlePrecLockNodeExistsChange: Signalling"
                    " the thread waiting on acquire failed");
            }
        }

        waitMapIt->second->predSignal();

        return EN_LOCKNODECHANGE;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "handlePrecLockNodeExistsChange: non-ZOO_DELETED_EVENT "
                  "event %d called", etype);
        throw InconsistentInternalStateException(
            "handlePrecLockNodeExistsChange: "
            "non-ZOO_DELETED_EVENT called");
    }
    
    return EN_NOEVENT;
}

}	/* End of 'namespace clusterlib' */
