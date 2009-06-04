/*
 * internalchangehandlers.cc --
 *
 * Implementation of change handlers that are not visisble to
 * clusterlib clients.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace boost;

namespace clusterlib
{

/*
 * Handle existence change for preceding lock node. This is called
 * whenever a preceding lock node being watched by a thread in this
 * abdicates.  All it does is signal the lock waiting on it to wake up
 * and try again.
 */
Event
InternalChangeHandlers::handlePrecLockNodeExistsChange(int32_t etype,
                                                       const string &key)
{
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

};	/* End of 'namespace clusterlib' */
