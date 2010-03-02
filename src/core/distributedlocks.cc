#include "clusterlibinternal.h"

using namespace std;

namespace clusterlib {

void
DistributedLocks::acquire(Notifyable *ntp, 
                          const string &lockName)
{
    TRACE(CL_LOG, "acquire");

    if (!acquireWaitUsecs(-1LL, ntp, lockName)) {
        throw InconsistentInternalStateException(
            "acquire: Impossible that acquireWaitUsecs failed!");
    }
}

bool
DistributedLocks::acquireWaitUsecs(int64_t usecTimeout,
                                   Notifyable *ntp, 
                                   const string &lockName)
{
    TRACE(CL_LOG, "acquireWaitUsecs");

    if (usecTimeout < -1) {
        stringstream ss;
        ss << "acquireWaitUsecs: Cannot have usecTimeout < -1 (" 
           << usecTimeout << ")";
        throw InvalidArgumentsException(ss.str());
    }

    int64_t curUsecTimeout = 0;
    int64_t maxUsecs = 0;
    if (usecTimeout != -1) {
        maxUsecs = TimerService::getCurrentTimeUsecs() + usecTimeout;
    }
    else {
        curUsecTimeout = -1;
    }
    NotifyableImpl *castedNtp = dynamic_cast<NotifyableImpl *>(ntp);
    if (castedNtp == NULL) {
        InvalidArgumentsException("acquireWaitUsecs: Notifyable is NULL");
    }

    /* 
     * Locking operations are serialized for each thread on a lock
     * granular basis.
     */
    Locker l(castedNtp->getSyncDistLock());

    /**
     * Algorithm:
     *
     * based on http://hadoop.apache.org/zookeeper/docs/current/recipes.html
     *
     * 1. Lazily create the lock node.
     * 2. Create my entry as 'lock node'/entry with sequence and ephemeral 
     *    flags set.
     * 3. Call getChildren() on the lock node without setting the watch flag.
     * 4. If my entry has the lowest sequence number suffix, have the lock, 
     *    exit
     * 5. The client calls exists() with the watch flag set on the path in the 
     *    lock directory with the next lowest sequence number.
     * 6. If exists(), returns false, go to step 3. Otherwise, wait for a 
     *    notification for the pathname from the previous step before going 
     *    to step 3.
     */
    string locksKey = NotifyableKeyManipulator::createLocksKey(
        castedNtp->getKey());

    LOG_DEBUG(CL_LOG, 
              "acquireWaitUsecs: Creating locks node %s", 
              locksKey.c_str());

    SAFE_CALL_ZK(getOps()->getRepository()->createNode(locksKey, "", 0, false),
                 "Creation of %s failed: %s",
                 locksKey.c_str(), 
                 true,
                 true);

    string lockKey = NotifyableKeyManipulator::createLockKey(
        castedNtp->getKey(),
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
            castedNtp->getKey(),
            lockName);
    
    /*
     * If I already have the lock, just increase the reference count.
     */
    string::size_type pos = 
        castedNtp->getDistributedLockOwner(
            lockName).find(lockNode);
    if (pos != string::npos) {
        LOG_WARN(CL_LOG, 
                 "acquireWaitUsecs: Already have the lock on %s (had "
                 "%d references)", 
                 castedNtp->getKey().c_str(),
                 castedNtp->getDistributedLockOwnerCount(
                     lockName));
        castedNtp->incrDistributedLockOwnerCount(
            lockName);
        return true;
    }

    int64_t myBid = -1;
    string myBidThread;
    string createdPath;
    SAFE_CALL_ZK((myBid = getOps()->getRepository()->createSequence(
                      lockNode, 
                      "", 
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
    do {
        NameList childList;
        SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(lockKey,
                                                                childList),
                     "Getting bids for group %s failed: %s",
                     lockKey.c_str(),
                     false,
                     true);
        NameList::iterator childListIt, lowerChildIt;
        tmpBid = -1;
        lowerBid = -1;
        for (childListIt = childList.begin(); 
             childListIt != childList.end(); 
             childListIt++) {
            /* 
             * Get the bid number from the path.
             */
            zk::ZooKeeperAdapter::splitSequenceNode(*childListIt, 
                                                    &tmpBidThread,
                                                    &tmpBid);

            LOG_DEBUG(CL_LOG, 
                      "acquireWaitUsecs: thread 0x%x, this 0x%x "
                      "got bid %s with bid %lld", 
                      static_cast<uint32_t>(pthread_self()),
                      reinterpret_cast<uint32_t>(this),
                      childListIt->c_str(),
                      tmpBid);
            if ((!tmpBidThread.compare(myBidThread)) &&
                 (myBid != tmpBid)) {
                stringstream ss;
                ss << "acquireWaitUsecs: Impossible that " << myBidThread 
                   << " already has bid " << tmpBid 
                   << " and is adding myBid " << myBid;
                LOG_FATAL(CL_LOG, "%s", ss.str().c_str());
                throw InconsistentInternalStateException(ss.str());
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
                          "acquireWaitUsecs: Replaced lowerBid %lld (from %s) "
                          "with tmpBid %lld (from %s), myBid = %lld",
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
                      "acquireWaitUsecs: Waiting for lowerBid %lld (from %s) "
                      "for mybid %lld for thread 0x%x",
                      lowerBid,
                      lowerBidThread.c_str(),
                      myBid,
                      (uint32_t) pthread_self());

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
                        maxUsecs - TimerService::getCurrentTimeUsecs(), 0LL);
                }
                LOG_DEBUG(CL_LOG, 
                          "acquireWaitUsecs: Going to wait for %lld "
                          "usecs (%lld usecs originally)", 
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
     * Remember the createPath node name so it can be cleaned up at release.
     */
    castedNtp->setDistributedLockOwner(lockName,
                                       createdPath);
    castedNtp->incrDistributedLockOwnerCount(
        lockName);
    LOG_DEBUG(CL_LOG, "acquireWaitUsecs: Setting distributed lock key "
              "of Notifyable %s with %s (%u references)",
              castedNtp->getKey().c_str(),
              createdPath.c_str(),
              castedNtp->getDistributedLockOwnerCount(
                  lockName));
    /*
     * In order to guarantee that changes are seen from one locked
     * region to another locked region, synchronize must be used prior
     * to making any changes.
     */
    mp_ops->synchronize();

    return true;
}

void
DistributedLocks::release(Notifyable *ntp, const string &lockName)
{
    TRACE(CL_LOG, "release");

    NotifyableImpl *castedNtp = dynamic_cast<NotifyableImpl *>(ntp);
    if (castedNtp == NULL) {
        throw InvalidArgumentsException("release: Notifyable is NULL");
    }

    /* 
     * Locking operations are serialized for each thread on a lock
     * granular basis. 
     */
    Locker l(castedNtp->getSyncDistLock());

    string lockNode = 
        NotifyableKeyManipulator::createLockNodeKey(
            castedNtp->getKey(),
            lockName);

    LOG_DEBUG(CL_LOG, "release: Looking for %s in %s (%d references)",
              lockNode.c_str(),
              castedNtp->getDistributedLockOwner(
                  lockName).c_str(),
              castedNtp->getDistributedLockOwnerCount(
                  lockName));

    /*
     * Make sure that I actually have the lock before I delete the
     * node.  Only delete the node if my reference count drops to 0.
     */
    string removeNode = castedNtp->getDistributedLockOwner(
        lockName);
    string::size_type pos = removeNode.find(lockNode);
    if (pos == string::npos) {
        throw InvalidMethodException(
            string("release: I don't have the lock on ") +
            removeNode + 
            string(" with my node ") + 
            lockNode);
    }

    int32_t refCount = castedNtp->decrDistributedLockOwnerCount(
        lockName);
    if (refCount != 0) {
        return;
    }
    
    castedNtp->setDistributedLockOwner(lockName, "");

    /* 
     * Delete the lock node here.
     */
    bool deleted = false;
    SAFE_CALL_ZK((deleted = getOps()->getRepository()->deleteNode(
                      removeNode,
                      false,
                      -1)),
                 "release: Trying to delete lock node: %s failed: %s",
                 removeNode.c_str(),
                 false,
                 true);
    if (deleted == false) {
        /*
         * Make sure that this Notifyable is still alive?  Can't release
         * deleted Notifyables.
         */
        if (castedNtp->getState() == Notifyable::REMOVED) {
            LOG_DEBUG(CL_LOG, 
                      "release: Lock on Notifyable %s will not be released "
                      "since it was removed",
                      castedNtp->getKey().c_str());
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
DistributedLocks::hasLock(Notifyable *ntp, const string &lockName)
{
    TRACE(CL_LOG, "hasLock");
    
    NotifyableImpl *castedNtp = dynamic_cast<NotifyableImpl *>(ntp);
    if (castedNtp == NULL) {
        throw InvalidArgumentsException("release: Notifyable is NULL");
    }

    /* 
     * Locking operations are serialized for each thread on a lock
     * granular basis. 
     */
    Locker l(castedNtp->getSyncDistLock());

    string lockOwner = castedNtp->getDistributedLockOwner(lockName);
    
    string myLockNodeKey = NotifyableKeyManipulator::createLockNodeKey(
        castedNtp->getKey(),
        lockName);

    LOG_DEBUG(CL_LOG, 
              "hasLock: Trying to find my lock node prefix (%s) in the "
              "lock owner (%s) for Notifyable (%s) with ref count (%d)",
              myLockNodeKey.c_str(),
              lockOwner.c_str(),
              ntp->getKey().c_str(),
              castedNtp->getDistributedLockOwnerCount(lockName));
    
    if (lockOwner.find(myLockNodeKey) != string::npos) {
        return true;
    }

    return false;
}

};
