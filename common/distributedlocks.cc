#include "clusterlibinternal.h"

using namespace std;

namespace clusterlib {

void
DistributedLocks::acquire(NotifyableImpl *ntp)
{
    TRACE(CL_LOG, "acquire");

    if (ntp == NULL) {
        throw ClusterException("acquire: Notifyable is NULL");
    }

    /*
     * Cannot acquire lock on Root.
     */
    NotifyableImpl *parent = 
        dynamic_cast<NotifyableImpl *>(ntp->getMyParent());
    if (parent == NULL) {
        throw ClusterException("acquire: parent is NULL");
    }

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
     * 6. if exists( ) returns false, go to step 3. Otherwise, wait for a 
     *    notification for the pathname from the previous step before going 
     *    to step 3.
     */

    string lockKey = NotifyableKeyManipulator::createLockKey(parent->getKey());

    LOG_DEBUG(CL_LOG, "acquire: Creating lock node %s", lockKey.c_str());

    SAFE_CALL_ZK(getOps()->getRepository()->createNode(lockKey, "", 0),
                 "Creation of %s failed: %s",
                 lockKey.c_str(), 
                 true,
                 true);

    string lockNode = 
        NotifyableKeyManipulator::createLockNodeKey(parent->getKey());
    
    int64_t myBid = -1;
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

    LOG_DEBUG(CL_LOG, 
              "acquire: Creating lock entry node %s with createdPath %s", 
              lockNode.c_str(),
              createdPath.c_str());
    
    /*
     * See if I have the lock.  Find the bid that directly precedes my
     * bid.  If it doesn't exist, repeat
     */
    string precZkNode;
    int64_t tmpBid = -1, lowestBid = -1;
    do {
        NameList childList;
        SAFE_CALL_ZK((getOps()->getRepository()->getNodeChildren(childList,
                                                                 lockKey)),
                     "Getting bids for group %s failed: %s",
                     lockKey.c_str(),
                 false,
                 true);
        NameList::iterator childListIt, lowestChildIt;
        tmpBid = -1;
        lowestBid = -1;
        for (childListIt = childList.begin(); 
             childListIt != childList.end(); 
             childListIt++) {
            /*
             * Get only the bid number;
             */
            uint32_t bidSplitIndex = 
                childListIt->rfind(ClusterlibStrings::BID_SPLIT);
            if ((bidSplitIndex == string::npos) || 
                (bidSplitIndex == childListIt->size() - 1)) {
                throw ClusterException("acquire: Expecting a valid bid split");
            }

            /*
             * Ensure that this is a legal sequence number.
             */
            tmpBid = ::strtol(
                &(childListIt->c_str()[bidSplitIndex + 1]), NULL, 10);
            if (tmpBid < 0) {
                LOG_WARN(CL_LOG, 
                         "Expecting a valid number but got %s", 
                         &(childListIt->c_str()[bidSplitIndex + 1]));
                throw ClusterException("Expecting a valid number but got " +
                                       childListIt->substr(bidSplitIndex + 1));
            }

            LOG_DEBUG(CL_LOG, 
                      "acquire: got bid %s with bid %lld", 
                      childListIt->c_str(),
                      tmpBid);

            /*
             * Compare to my current lowest bid.
             */
            if ((lowestBid == -1) || (tmpBid < lowestBid)) {
                lowestBid = tmpBid;
                lowestChildIt = childListIt;
            }
        }
        
        if (lowestBid != myBid) {
            bool exists = false;
            precZkNode = lockKey;
            precZkNode.append(ClusterlibStrings::KEYSEPARATOR);
            precZkNode.append(*lowestChildIt);

            /*
             * Set up the waiting for the handler function.
             */
            WaitMap::iterator waitMapIt;
            PredMutexCond predMutexCond;
            {
                Locker l1(getWaitMapLock());
                waitMapIt = getWaitMap()->find(createdPath);
                if (waitMapIt != getWaitMap()->end()) {
                    throw ClusterException("acquire: Setting up waiting for "
                                           "the handler failed");
                }
                getWaitMap()->insert(make_pair(createdPath, &predMutexCond));
            }
            
            CachedObjectEventHandler *handler = 
                getOps()->getChangeHandlers()->getPrecLockNodeExistsHandler();
            SAFE_CALL_ZK(
                (exists = getOps()->getRepository()->nodeExists(
                    precZkNode,
                    getOps()->getZooKeeperEventAdapter(),
                    handler)),
                "Checking for preceding lock node %s failed: %s",
                precZkNode.c_str(),
                false,
                true);

            /* 
             * Wait until it a signal from the from event handler
             */
            if (exists) {
                waitMapIt->second->predWait();
            }

            /*
             * Clean up and try again
             */
            if (!exists) {
                Locker l1(getWaitMapLock());
                WaitMap::iterator waitMapIt = getWaitMap()->find(createdPath);
                if (waitMapIt == getWaitMap()->end()) {
                    throw ClusterException("acquire: Setting up waiting for "
                                           "the handler failed");
                }
                getWaitMap()->erase(waitMapIt);
            }
        }
    } while (lowestBid != myBid);

    /*
     * Remember the createPath node name so it can be cleaned up at release.
     */
    parent->setDistributedLockKey(createdPath);
}

void
DistributedLocks::release(NotifyableImpl *ntp)
{
    TRACE(CL_LOG, "release");

    if (ntp == NULL) {
        throw ClusterException("release: Notifyable is NULL");
    }

    /*
     * Cannot release lock on Root.
     */
    NotifyableImpl *parent = 
        dynamic_cast<NotifyableImpl *>(ntp->getMyParent());
    if (parent == NULL) {
        throw ClusterException("release: parent is NULL");
    }

    string lockNodeKey = 
        NotifyableKeyManipulator::createLockNodeKey(parent->getKey());

    LOG_DEBUG(CL_LOG, "release: Looking for %s in %s",
              lockNodeKey.c_str(),
              parent->getDistributedLockKey().c_str());

    /*
     * Make sure that I actually have the lock before I delete the node
     */
    string::size_type pos = parent->getDistributedLockKey().find(lockNodeKey);
    if (pos == string::npos) {
        throw ClusterException("release: I shouldn't have the lock");
    }
    
    /* 
     * Delete the lock node here.
     */
    bool deleted = false;
    SAFE_CALL_ZK((deleted = getOps()->getRepository()->deleteNode(
                      parent->getDistributedLockKey(),
                      false,
                      -1)),
                 "release: Trying to delete lock node: %s failed: %s",
                 parent->getDistributedLockKey().c_str(),
                 false,
                 true);
    if (deleted == false) {
        throw ClusterException(string("release: Delete of node") + 
                               parent->getDistributedLockKey() + 
                               "failed");
    }
}

};
