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
using namespace json;

namespace clusterlib {

void
DistributedLocks::acquire(const shared_ptr<Notifyable> &pNotifyableSP, 
                          const string &lockName,
                          DistributedLockType distributedLockType)
{
    TRACE(CL_LOG, "acquire");

    if (!acquireWaitUsecs(
            -1LL, pNotifyableSP, lockName, distributedLockType)) {
        throw InconsistentInternalStateException(
            "acquire: Impossible that acquireWaitUsecs failed!");
    }
}

bool
DistributedLocks::acquireWaitMsecs(
    int64_t msecTimeout, 
    const shared_ptr<Notifyable> &pNotifyableSP, 
    const string &lockName,
    DistributedLockType distributedLockType)
{
    TRACE(CL_LOG, "acquireWaitMsecs");

    return acquireWaitUsecs(((msecTimeout == -1) ? -1 : msecTimeout * 1000),
                            pNotifyableSP,
                            lockName,
                            distributedLockType);
}

bool
DistributedLocks::acquireWaitUsecs(
    int64_t usecTimeout,
    const shared_ptr<Notifyable> &pNotifyableSP, 
    const string &lockName,
    DistributedLockType distributedLockType)
{
    TRACE(CL_LOG, "acquireWaitUsecs");

    if (lockName.compare(CLString::NOTIFYABLE_LOCK) &&
        lockName.compare(CLString::OWNERSHIP_LOCK) &&
        lockName.compare(CLString::CHILD_LOCK) &&
        !NotifyableKeyManipulator::isValidNotifyableName(lockName)) {
        throw InvalidArgumentsException(
            string("acquireWaitUsecs: Invalid lockName=") + lockName);
    }

    if ((distributedLockType < DIST_LOCK_SHARED) ||
        (distributedLockType > DIST_LOCK_EXCL)) {
        ostringstream oss;
        oss << "acquireWaitUsecs: Invalid distributedLockType="
            << distributedLockType;
        throw InvalidArgumentsException(oss.str());
    }

    if (usecTimeout < -1) {
        ostringstream oss;
        oss << "acquireWaitUsecs: Cannot have usecTimeout < -1 (" 
            << usecTimeout << ")";
        throw InvalidArgumentsException(oss.str());
    }

    int64_t curUsecTimeout = 0;
    int64_t maxUsecs = 0;
    if (usecTimeout != -1) {
        maxUsecs = TimerService::getCurrentTimeUsecs() + usecTimeout;
    }
    else {
        curUsecTimeout = -1;
    }
    shared_ptr<NotifyableImpl> notifyableImplSP = 
        dynamic_pointer_cast<NotifyableImpl>(pNotifyableSP);
    if (notifyableImplSP == NULL) {
        InvalidArgumentsException("acquireWaitUsecs: Notifyable is NULL");
    }

    /* 
     * Acquiring locks is serialized for each thread on a lock
     * granular basis.  Releasing can happen in parallel and is safe.
     */
    Locker l(notifyableImplSP->getSyncDistLock());

    /**
     * Algorithm:
     *
     * Based on 
     * http://hadoop.apache.org/zookeeper/docs/r3.3.1/recipes.html#Shared+Locks
     *
     * 1. Lazily create the locks zookeeper node (might already exist).
     * 2. Lazily create the particular lockName zookeeper node
     *    (might already exist).
     * 3. Create my entry as 'locks'/'lockName/entry with sequence 
     *    and ephemeral flags set and the appropriate lock type as part of 
     *    the name - for instance 'myThreadId.writeLock'.
     * 4. Call getChildren() on the lockName node without setting the 
     *    watch flag.
     * 5. - (DIST_LOCK_SHARED) If there are no DIST_LOCK_EXCL
     *    entries above mine, I have the lock and will exit.
     *    - (DIST_LOCK_EXCL) If there are no entries above
     *    mine, I have the lock and will exit.
     * 6. - (DIST_LOCK_SHARED) Client calls exists() with the watch flag set on
     *    the path with the next lowest sequence number that is a 
     *    DIST_LOCK_EXCL or DIST_LOCK_EXCL.
     *    - (DIST_LOCK_EXCL) Client calls exists() with the 
     *    watch flag set on the path with the next lowest sequence number.
     * 7. If exists(), returns false, go to step 3. Otherwise, wait for a 
     *    notification for the pathname from the previous step before going 
     *    to step 3.
     */
    string locksKey = NotifyableKeyManipulator::createLocksKey(
        notifyableImplSP->getKey());

    LOG_DEBUG(CL_LOG, 
              "acquireWaitUsecs: Creating locks node %s", 
              locksKey.c_str());

    SAFE_CALL_ZK(getOps()->getRepository()->createNode(locksKey, "", 0, false),
                 "Creation of %s failed: %s",
                 locksKey.c_str(), 
                 true,
                 true);

    string lockKey = NotifyableKeyManipulator::createLockKey(
        notifyableImplSP->getKey(),
        lockName);

    LOG_DEBUG(CL_LOG, 
              "acquireWaitUsecs: Creating lock node %s", 
              lockKey.c_str());

    SAFE_CALL_ZK(getOps()->getRepository()->createNode(lockKey, "", 0, false),
                 "Creation of %s failed: %s",
                 lockKey.c_str(), 
                 true,
                 true);

    string lockNode = 
        NotifyableKeyManipulator::createLockNodeKey(
            notifyableImplSP->getKey(),
            lockName,
            distributedLockType);
    
    /*
     * If I already have the lock and this is the same lock type, just
     * increase the reference count.  Note that the
     * distributedLockType must the exactly the same as the one I am
     * requesting now in order to support re-entrant locks.
     */
    if (notifyableImplSP->isReentrantLockAttempt(
            lockName, lockKey, distributedLockType)) {
        if (CL_LOG->isInfoEnabled()) {
            int32_t refCount = -1;
            notifyableImplSP->getDistributedLockOwnerInfo(
                lockName, NULL, NULL, NULL, NULL, &refCount);
            LOG_DEBUG(CL_LOG, 
                      "acquireWaitUsecs: Re-entering already held lock on %s "
                      "(had %" PRId32 " references)", 
                      notifyableImplSP->getKey().c_str(),
                      refCount);
        }
        notifyableImplSP->incrDistributedLockOwnerCount(lockName);
        return true;
    }

    int64_t myBid = -1;
    string myBidThread;
    string createdPath;

    /*
     * Put the time of trying to acquire the lock into the data
     */
    JSONValue::JSONInteger jsonInteger = TimerService::getCurrentTimeMsecs();
    
    SAFE_CALL_ZK((myBid = getOps()->getRepository()->createSequence(
        lockNode, 
        JSONCodec::encode(jsonInteger),
        ZOO_EPHEMERAL, 
        false, 
        createdPath)),
                 "Bidding with lock of Notifyable %s to get lock failed: %s",
                 lockKey.c_str(),
                 false,
                 true);
    zk::ZooKeeperAdapter::splitSequenceNode(createdPath,
                                            &myBidThread,
                                            NULL);
    

    LOG_DEBUG(CL_LOG, 
              "acquireWaitUsecs: Creating lock entry node %s "
              "with createdPath %s", 
              lockNode.c_str(),
              createdPath.c_str());
    
    /*
     * See if I have the lock.  Find the bid that directly precedes my
     * bid.  If it doesn't exist, repeat
     */
    string precZkNode;
    int64_t tmpBid = -1, lowerBid = -1;
    string tmpBidThread, lowerBidThread;
    DistributedLockType tmpDistributedLockType;
    do {
        NameList childList;
        SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(lockKey,
                                                                childList),
                     "Getting bids for lock %s failed: %s",
                     lockKey.c_str(),
                     false,
                     true);
        NameList::iterator childListIt, lowerChildIt;
        tmpBid = -1;
        lowerBid = -1;
        for (childListIt = childList.begin(); 
             childListIt != childList.end(); 
             childListIt++) {
            /* Get the bid number from the path */
            zk::ZooKeeperAdapter::splitSequenceNode(*childListIt, 
                                                    &tmpBidThread,
                                                    &tmpBid);
            /* Get the lock type from the sequence name */
            tmpDistributedLockType = getDistributedLockType(*childListIt);
            
            LOG_DEBUG(CL_LOG, 
                      "acquireWaitUsecs: thread %" PRIu32 ", this %p "
                      "got bid %s with bid %" PRId64 " and lock type %s", 
                      ProcessThreadService::getTid(),
                      this,
                      childListIt->c_str(),
                      tmpBid,
                      distributedLockTypeToString(
                          tmpDistributedLockType).c_str());

            if ((!tmpBidThread.compare(myBidThread)) &&
                (myBid != tmpBid)) {
                ostringstream oss;
                oss << "acquireWaitUsecs: Impossible that " << myBidThread 
                    << " already has bid " << tmpBid 
                    << " and is adding myBid " << myBid;
                LOG_FATAL(CL_LOG, "%s", oss.str().c_str());
                throw InconsistentInternalStateException(oss.str());
            }

            /*
             * For shared locks, the only locks to consider above mine
             * are the exclusive ones or my own.
             */
            if (distributedLockType == DIST_LOCK_SHARED) {
                if ((tmpDistributedLockType != DIST_LOCK_EXCL) &&
                    (tmpBidThread.compare(myBidThread) != 0)) {
                    continue;
                }
            }

            /*
             * Try to get the bid that is highest one that is lower
             * than mine.  I.e.
             *
             *     a  < b < my bid < c,     -- pick b.
             * my bid < a < b      < c,     -- pick my bid
             *     a  < b < c      < my bid -- pick c
             *
             * Compare to my current lower bid.
             *
             * Rules:
             * 1) Only select an initial lowerBid if it is equal to or lower 
             *    than mine.
             *    - At this point, lowerBid must be <= myBid
             * 2) Choose a tmpBid if it is less than my bid and higher than 
             *    the lower bid
             *    - Makes sure that high bids are chosen if tmpBid isn't myBid
             * 3) If myBid is the lowerBid and there is a bid less than mine,
             *    than choose it.
             *    - Ensures that if my bid is the lowerBid, a lower one is 
             *      always chosen.
             */
            if (((lowerBid == -1) && (tmpBid <= myBid)) ||
                ((tmpBid < myBid) && (tmpBid > lowerBid)) ||
                ((tmpBid < myBid) && (myBid == lowerBid))) {
                LOG_DEBUG(CL_LOG, 
                          "acquireWaitUsecs: Replaced lowerBid %" PRId64 
                          " (from %s) with tmpBid %" PRId64 " (from %s), "
                          "myBid = %" PRId64,
                          lowerBid,
                          lowerBidThread.c_str(),
                          tmpBid,
                          tmpBidThread.c_str(),
                          myBid);
                lowerBidThread = tmpBidThread;
                lowerBid = tmpBid;
                lowerChildIt = childListIt;
            }
        }
        
        if (lowerBid < myBid) {
            bool exists = false;

            LOG_DEBUG(CL_LOG,
                      "acquireWaitUsecs: Waiting for lowerBid %" PRId64 
                      " (from %s) for mybid %" PRId64 " for thread %" PRIu32,
                      lowerBid,
                      lowerBidThread.c_str(),
                      myBid,
                      ProcessThreadService::getTid());

            /*
             * No children indicates that they have been removed.  No
             * lock can be acquired.
             */
            if (lowerBid == -1) {
                throw ObjectRemovedException("acquireWaitUsecs: No children!");
            }

            /*
             * Set up the waiting for the handler function on the lower child.
             */
            getOps()->getLockEventSignalMap()->addRefPredMutexCond(
                *lowerChildIt);
            CachedObjectEventHandler *handler = 
                getOps()->getCachedObjectChangeHandlers()->
                getChangeHandler(
                    CachedObjectChangeHandlers::PREC_LOCK_NODE_EXISTS_CHANGE);
            SAFE_CALL_ZK(
                (exists = getOps()->getRepository()->nodeExists(
                    (*lowerChildIt),
                    getOps()->getZooKeeperEventAdapter(),
                    handler)),
                "Checking for preceding lock node %s failed: %s",
                (*lowerChildIt).c_str(),
                false,
                true);

            /* 
             * Wait until it a signal from the from event handler
             */
            LOG_DEBUG(CL_LOG, 
                      "acquireWaitUsecs: Wait for handler? = %d", 
                      exists);
            if (exists) {
                if (curUsecTimeout != -1) {
                    /* Don't let curUsecTimeout go negative. */
                    curUsecTimeout = max(
                        maxUsecs - TimerService::getCurrentTimeUsecs(), 
                        static_cast<int64_t>(0));
                }
                LOG_DEBUG(CL_LOG, 
                          "acquireWaitUsecs: Going to wait for %" PRId64 
                          " usecs (%" PRId64 " usecs originally)", 
                          curUsecTimeout,
                          usecTimeout);
                getOps()->getLockEventSignalMap()->waitUsecsPredMutexCond(
                    *lowerChildIt, curUsecTimeout);                    
            }

            /*
             * Only clean up if we are the last thread to wait on this
             * conditional (otherwise, just decrease the reference
             * count).  Then try again if lowerBid != myBid.
             *
             * The lowerChild was deleted and can NEVER be recreated,
             * so it is safe to possibly remove the PredMutexCond if
             * it is the last thread waiting.
             */
            getOps()->getLockEventSignalMap()->removeRefPredMutexCond(
                *lowerChildIt);

            /* 
             * Give up and remove my bid since lock wasn't acquired
             * within the usecTimeout.
             */
            if ((usecTimeout != -1) &&
                (TimerService::compareTimeUsecs(maxUsecs) >= 0)) {
                bool deleted = false;
                SAFE_CALL_ZK((deleted = getOps()->getRepository()->deleteNode(
                    createdPath,
                    false,
                    -1)),
                             "acquireWaitUsecs: Trying to delete lock node: %s"
                             " failed: %s",
                             createdPath.c_str(),
                             false,
                             true);
                if (deleted == false) {
                    LOG_ERROR(CL_LOG,
                              "acquireWaitUsecs: Couldn't remove my bid %s",
                              createdPath.c_str());
                }
                return false;
            }
        }
        else if (lowerBid > myBid) {
            throw InconsistentInternalStateException(
                "acquireWaitUsecs: Impossible the lower bid is greater "
                "than my own.");
        }
    } while (lowerBid != myBid);

    /*
     * Remember the createPath node name and other lock information
     * for cleanup and remembering the lock type for re-entrant locks.
     */
    notifyableImplSP->setDistributedLockOwnerInfo(lockName,
                                                  lockKey,
                                                  createdPath,
                                                  distributedLockType);
    notifyableImplSP->incrDistributedLockOwnerCount(
        lockName);
    if (CL_LOG->isDebugEnabled()) {
        int32_t refCount = -1;
        notifyableImplSP->getDistributedLockOwnerInfo(
            lockName, NULL, NULL, NULL, NULL, &refCount);
        LOG_DEBUG(CL_LOG, 
                  "acquireWaitUsecs: Setting distributed lock key "
                  "of Notifyable %s with %s (%" PRId32 " references)", 
                  notifyableImplSP->getKey().c_str(),
                  createdPath.c_str(),
                  refCount);
    }

    /*
     * In order to guarantee that changes are seen from one locked
     * region to another locked region, synchronize must be used prior
     * to making any changes.
     */
    mp_ops->synchronize();

    return true;
}

void
DistributedLocks::release(const shared_ptr<Notifyable> &pNotifyableSP, 
                          const string &lockName)
{
    TRACE(CL_LOG, "release");

    shared_ptr<NotifyableImpl> notifyableImplSP = 
        dynamic_pointer_cast<NotifyableImpl>(pNotifyableSP);
    if (notifyableImplSP == NULL) {
        throw InvalidArgumentsException("release: Notifyable is NULL");
    }

    string ownerHostnamePidTid;
    string ownerLockNodeCreatedPath;
    DistributedLockType distributedLockType;
    int32_t ownerRefCount = -1;
    bool lockExists = notifyableImplSP->getDistributedLockOwnerInfo(
        lockName, 
        &ownerHostnamePidTid,
        NULL, 
        &ownerLockNodeCreatedPath, 
        &distributedLockType, 
        &ownerRefCount);
    if (lockExists == false) {
        throw InvalidMethodException(
            string("release: There is no known lock for ") + 
            notifyableImplSP->getKey());
    }

    string lockNode = 
        NotifyableKeyManipulator::createLockNodeKey(
            notifyableImplSP->getKey(),
            lockName,
            distributedLockType);

    LOG_DEBUG(CL_LOG, 
              "release: Looking for %s in %s (%" PRId32 " references)",
              lockNode.c_str(),
              ownerLockNodeCreatedPath.c_str(),
              ownerRefCount);

    /*
     * Make sure that I actually have the lock before I delete the
     * node.  Only delete the node if my reference count drops to 0.
     */
    if (ownerHostnamePidTid.compare(
            ProcessThreadService::getHostnamePidTid()) != 0) {
        throw InvalidMethodException(
            string("release: I don't have the lock on ") +
            ownerLockNodeCreatedPath + 
            string(" with my node ") + 
            lockNode);
    }

    int32_t refCount = notifyableImplSP->decrDistributedLockOwnerCount(
        lockName);
    if (refCount != 0) {
        return;
    }

    /* 
     * Delete the lock node here.
     */
    bool deleted = false;
    SAFE_CALL_ZK((deleted = getOps()->getRepository()->deleteNode(
        ownerLockNodeCreatedPath,
        false,
        -1)),
                 "release: Trying to delete lock node: %s failed: %s",
                 ownerLockNodeCreatedPath.c_str(),
                 false,
                 true);
    if (deleted == false) {
        /*
         * Make sure that this Notifyable is still alive?  Can't release
         * deleted Notifyables.
         */
        if (notifyableImplSP->getState() == Notifyable::REMOVED) {
            LOG_DEBUG(CL_LOG, 
                      "release: Lock on Notifyable %s will not be released "
                      "since it was removed",
                      notifyableImplSP->getKey().c_str());
            return;
        }

        /*
         * Also, possible that deletion failed since node was already
         * deleted.  Cannot be diffentiated from other failures
         * because of the deleteNode interface.  Should be fixed in
         * the future.
         */
    }
}

bool
DistributedLocks::hasLock(const shared_ptr<Notifyable> &pNotifyableSP, 
                          const string &lockName,
                          DistributedLockType *pDistributedLockType) 
{
    TRACE(CL_LOG, "hasLock");
    
    shared_ptr<NotifyableImpl> notifyableImplSP = 
        dynamic_pointer_cast<NotifyableImpl>(pNotifyableSP);
    if (notifyableImplSP == NULL) {
        throw InvalidArgumentsException("hasLock: Notifyable is NULL");
    }

    /* 
     * Locking operations are serialized for each thread on a lock
     * granular basis. 
     */
    Locker l(notifyableImplSP->getSyncDistLock());
    
    string ownerHostnamePidTid;
    bool lockExists = notifyableImplSP->getDistributedLockOwnerInfo(
        lockName, 
        &ownerHostnamePidTid,
        NULL, 
        NULL,
        pDistributedLockType, 
        NULL);
    if (lockExists == false) {
        return false;
    }

    if (ownerHostnamePidTid.compare(
            ProcessThreadService::getHostnamePidTid()) != 0) {
        return false;
    }
    else {
        return true;
    }
}

bool
DistributedLocks::getInfo(const shared_ptr<Notifyable> &pNotifyableSP,
                          const string &lockName, 
                          string *id, 
                          DistributedLockType *pDistributedLockType,
                          int64_t *msecs)
{
    TRACE(CL_LOG, "getInfo");

    shared_ptr<NotifyableImpl> notifyableImplSP = 
        dynamic_pointer_cast<NotifyableImpl>(pNotifyableSP);
    if (notifyableImplSP == NULL) {
        throw InvalidArgumentsException("release: Notifyable is NULL");
    }

    string lockKey = NotifyableKeyManipulator::createLockKey(
        notifyableImplSP->getKey(),
        lockName);

    NameList childList;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(lockKey,
                                                            childList),
                 "Getting bids for lock %s failed: %s",
                 lockKey.c_str(),
                 false,
                 true);
    if (childList.empty()) {
        return false;
    }

    if (id != NULL) {
        *id = *(childList.begin());
    }
    if (msecs != NULL) {
        bool exists = false;
        string encodedJsonInteger;
        SAFE_CALL_ZK(
            (exists = getOps()->getRepository()->getNodeData(
                *(childList.begin()),
                encodedJsonInteger)),
            "Checking for lock node %s failed: %s",
            childList.begin()->c_str(),
            false,
            true);
        if (exists) {
            JSONValue jsonValue = JSONCodec::decode(encodedJsonInteger);
            *msecs = jsonValue.get<JSONValue::JSONInteger>();
        }
        return exists;
    }
    return true;
}

const Mutex &
DistributedLocks::getWaitMapLock() const
{
    return m_waitMapLock;
}

FactoryOps *
DistributedLocks::getOps()
{
    return mp_ops;
}

DistributedLockType
DistributedLocks::getDistributedLockType(const string &sequenceKey)
{
    vector<string> components;
    split(components, 
          sequenceKey, 
          is_any_of(CLStringInternal::SEQUENCE_SPLIT));
    if (components.size() < 3) {
        throw InvalidArgumentsException(
            string("getDistributedLockType: Can't find components in ") +
            sequenceKey);
    }

    return distributedLockTypeFromString(components[components.size() - 2]);
}

}
