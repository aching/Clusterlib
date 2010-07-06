/*
 * notifyableimpl.cc
 *
 * Implementation of the notification classes outlined methods.
 *
 * ===========================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

#include "clusterlibinternal.h"
#include <limits>

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;

namespace clusterlib
{

const string Notifyable::PID_KEY = "_pid";

const string Notifyable::NOTIFYABLE_STATE_KEY = "_notifyableState";
const string Notifyable::NOTIFYABLE_STATE_PREPARING_VALUE = 
    "_notifyableStatePreparing";
const string Notifyable::NOTIFYABLE_STATE_READY_VALUE = 
    "_notifyableStateReady";
const string Notifyable::NOTIFYABLE_STATE_UNAVAILABLE_VALUE = 
    "_notifyableStateUnavailable";
const string Notifyable::NOTIFYABLE_STATE_UNUSED_VALUE = 
    "_notifyableStateUnused";
const string Notifyable::NOTIFYABLE_STATE_MAINTAIN_VALUE = 
    "_notifyableStateMaintain";
const string Notifyable::NOTIFYABLE_STATE_NONE_VALUE = 
    "_notifyableStateNone";

NameList
NotifyableImpl::getPropertyListNames() 
{
    TRACE(CL_LOG, "getPropertyListNames");

    throwIfRemoved();

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createPropertyListChildrenKey(getKey()),
        CachedObjectChangeHandlers::PROPERTYLISTS_CHANGE);
}

PropertyList *
NotifyableImpl::getPropertyList(const string &name, AccessType accessType)
{
    TRACE(CL_LOG, "getPropertyList");

    throwIfRemoved();

    return dynamic_cast<PropertyList *>(
        getOps()->getNotifyable(
            this,
            ClusterlibStrings::REGISTERED_PROPERTYLIST_NAME,
            name,
            accessType));
}

NameList
NotifyableImpl::getQueueNames() 
{
    TRACE(CL_LOG, "getQueueNames");

    throwIfRemoved();

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createQueueChildrenKey(getKey()),
        CachedObjectChangeHandlers::QUEUES_CHANGE);
}

Queue *
NotifyableImpl::getQueue(const std::string &queueName, AccessType accessType)
{
    TRACE(CL_LOG, "getQueue");

    throwIfRemoved();

    return dynamic_cast<Queue *>(
        getOps()->getNotifyable(this,
                                ClusterlibStrings::REGISTERED_QUEUE_NAME,
                                queueName,
                                accessType));
}

void
NotifyableImpl::releaseRef()
{
    TRACE(CL_LOG, "releaseRef");

    removeFromRemovedNotifyablesIfReleased(true);
}

Notifyable *
NotifyableImpl::getMyParent() const
{
    TRACE(CL_LOG, "getMyParent");

    throwIfRemoved();

    if (mp_parent == NULL) {
        throw InvalidArgumentsException(string("NULL parent for ") +
                                        getKey());
    }

    return dynamic_cast<Notifyable *>(mp_parent);
}

NotifyableList
NotifyableImpl::getMyChildren()
{
    TRACE(CL_LOG, "getMyChildren");

    throwIfRemoved();
    
    /*
     * Add the notifyables from this object and then the subclass
     * specific objects.
     */
    NotifyableList tmpList, finalList;
    tmpList = getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_PROPERTYLIST_NAME,
        getPropertyListNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());
    tmpList = getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_QUEUE_NAME,
        getQueueNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());

    tmpList = getChildrenNotifyables();
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());

    return finalList;
}

Application *
NotifyableImpl::getMyApplication()
{
    TRACE(CL_LOG, "getMyApplication");

    throwIfRemoved();

    /*
     * Try to find the object.
     *
     * An application is its own application, according to the
     * semantics implemented below.
     */
    string appKey = getKey();
    Application *myApp = NULL;
    do {
        myApp = dynamic_cast<Application *>(
            getOps()->getNotifyableFromKey(
                vector<string>(1, 
                               ClusterlibStrings::REGISTERED_APPLICATION_NAME),
                appKey, 
                LOAD_FROM_REPOSITORY));
        appKey = NotifyableKeyManipulator::removeObjectFromKey(appKey);
    }  while ((myApp == NULL) && (!appKey.empty()));

    return myApp;
}

Group *
NotifyableImpl::getMyGroup()
{
    TRACE(CL_LOG, "getMyGroup");

    throwIfRemoved();

    /*
     * Try to find the object.
     */
    string groupKey = getKey();
    Group *myGroup = NULL;
    do {
        groupKey = NotifyableKeyManipulator::removeObjectFromKey(groupKey);
        myGroup = dynamic_cast<Group *>(
            getOps()->getNotifyableFromKey(
                vector<string>(1, ClusterlibStrings::REGISTERED_GROUP_NAME),
                groupKey, 
                LOAD_FROM_REPOSITORY));
    } while ((myGroup == NULL) && (!groupKey.empty()));

    return myGroup;
}

Notifyable *
NotifyableImpl::getNotifyableFromKey(const string &key)
{
    TRACE(CL_LOG, "getNotifyableFromKey");

    return getOps()->getNotifyableFromKey(vector<string>(), key);
}

Notifyable::State
NotifyableImpl::getState() const
{
    TRACE(CL_LOG, "getState");

    Locker l1(getSyncLock());
    LOG_DEBUG(CL_LOG, 
              "getState: State for (%s) is %s", 
              getKey().c_str(), 
              NotifyableImpl::getStateString(m_state).c_str());
    return m_state;
}

Mutex *
NotifyableImpl::getSyncLock() const
{
    TRACE(CL_LOG, "getSyncLock");

    LOG_DEBUG(CL_LOG, "getSyncLock: For notifyable %s", getKey().c_str());

    return &m_syncLock;
}

Mutex *
NotifyableImpl::getSyncDistLock()
{
    TRACE(CL_LOG, "getSyncDistLock");

    LOG_DEBUG(CL_LOG, "getSyncDistLock: For notifyable %s", getKey().c_str());

    return &m_syncDistLock;
}

string
NotifyableImpl::getStateString(Notifyable::State state)
{
    TRACE(CL_LOG, "getStateString()");

    switch(state) {
        case Notifyable::READY:
            return "ready";
        case Notifyable::REMOVED:
            return "removed";
        default:
            return "unknown state";
    }
}

void
NotifyableImpl::acquireLock(bool acquireChildren)
{
    TRACE(CL_LOG, "acquireLock");
    
    if (!acquireLockWaitMsecs(-1LL, acquireChildren)) {
        throw InconsistentInternalStateException(
            "acquireLock: Impossible that acquireLockWaitMsecs failed!");
    }
}

bool
NotifyableImpl::acquireLockWaitMsecs(int64_t msecTimeout, bool acquireChildren)
{
    TRACE(CL_LOG, "acquireLockWaitMsecs");

    throwIfRemoved();

    if (msecTimeout < -1) {
        stringstream ss;
        ss << "acquireLockWaitMsecs: Cannot have msecTimeout < -1 (" 
           << msecTimeout << ")";
        throw InvalidArgumentsException(ss.str());
    }

    /* Adjust the curUsecTimeout for msecTimeout */
    int64_t curUsecTimeout = 0;
    int64_t maxUsecs = 0;
    if (msecTimeout != -1) {
        maxUsecs = TimerService::getCurrentTimeUsecs() + (msecTimeout * 1000);
    }
    else {
        curUsecTimeout = -1;
    }

    LOG_DEBUG(CL_LOG, 
              "acquireLockWaitMsec:: acquiring lock on %s", 
              getKey().c_str()); 

    /*
     * Algorithm:
     * -Acquire locks for this notifyable
     * -If acquireChildren is set, acquire all child locks of this notifyable
     */
    bool gotLock = false;
    NotifyableList ntList, tmpNtList;
    ntList.push_back(this);
    uint32_t ntListIndex = 0;
    do {
        if (curUsecTimeout != -1) {
            /* Don't let curUsecTimeout go negative if not already -1. */
            curUsecTimeout = max(
                maxUsecs - TimerService::getCurrentTimeUsecs(), 
                static_cast<int64_t>(0));
        }
        LOG_DEBUG(CL_LOG, 
                  "acquireLock: acquiring lock on %s with curUsecTimeout=%"
                  PRId64,
                  ntList[ntListIndex]->getKey().c_str(),
                  curUsecTimeout);
        gotLock = getOps()->getDistributedLocks()->acquireWaitUsecs(
            curUsecTimeout,
            ntList[ntListIndex],
            ClusterlibStrings::NOTIFYABLELOCK);
        if (!gotLock) {
            break;
        }

        if ((ntListIndex != 0) && (acquireChildren)) {
            tmpNtList = ntList[ntListIndex]->getMyChildren();
            /* Invalidates all iterators (therefore use of index) */
            ntList.insert(ntList.end(), tmpNtList.begin(), tmpNtList.end()); 
        }

        ++ntListIndex;
    } while (ntListIndex != ntList.size());

    /* Release the acquired locks if the last lock attempt failed */
    if (!gotLock && (ntListIndex > 0)) {
        for (int i = ntListIndex - 1; i >= 0; --i) {
            ntList[ntListIndex]->releaseLock(false);
        }
    }

    /* Release notifyables from getChildren() */
    NotifyableList::iterator ntIt;
    for (ntIt = ntList.begin(); ntIt != ntList.end(); ntIt++) {
        if (*ntIt != this) {
            (*ntIt)->releaseRef();
        }
    }

    if (gotLock) {
        return true;
    }
    else {
        return false;
    }
}

void 
NotifyableImpl::releaseLock(bool releaseChildren)
{
    TRACE(CL_LOG, "releaseLock");

    LOG_DEBUG(CL_LOG, 
              "releaseLock: releasing lock on %s", 
              getKey().c_str()); 
    if (releaseChildren == true) {
        NotifyableList ntList, tmpNtList;
        ntList.push_back(this);
        uint32_t ntListIndex = 0;
        do {
            LOG_DEBUG(CL_LOG, 
                      "releaseLock: releasing lock on %s", 
                      ntList[ntListIndex]->getKey().c_str());
            try {
                getOps()->getDistributedLocks()->release(
                    ntList[ntListIndex],
                    ClusterlibStrings::NOTIFYABLELOCK);
            } catch (ObjectRemovedException &e) {
                LOG_DEBUG(CL_LOG, 
                          "releaseLock: Object %s no longer exists "
                          "(most likely I deleted it",
                          getKey().c_str());
            }
            try {
                tmpNtList = ntList[ntListIndex]->getMyChildren();
            } catch (ObjectRemovedException &e) {
                LOG_INFO(CL_LOG,
                         "releaseLock: Getting children of %s failed, likely "
                         "since they no longer exist",
                         ntList[ntListIndex]->getKey().c_str());
                return;
            }

            /*
             * Invalidates all iterators (therefore use of index)
             */
            ntList.insert(ntList.end(), tmpNtList.begin(), tmpNtList.end()); 
            ntListIndex++;
        } while (ntListIndex != ntList.size());

        /* Release notifyables from getChildren() */
        NotifyableList::iterator ntIt;
        for (ntIt = ntList.begin(); ntIt != ntList.end(); ntIt++) {
            if (*ntIt != this) {
                (*ntIt)->releaseRef();
            }
        }
    }
    else {
        getOps()->getDistributedLocks()->release(
            this,
            ClusterlibStrings::NOTIFYABLELOCK);
    }
}

bool
NotifyableImpl::hasLock()
{
    TRACE(CL_LOG, "hasLock");

    throwIfRemoved();

    return getOps()->getDistributedLocks()->
        hasLock(this, ClusterlibStrings::NOTIFYABLELOCK);
}

bool
NotifyableImpl::getLockInfo(std::string *id, 
                            int64_t *msecs)
{
    TRACE(CL_LOG, "getLockInfo");

    throwIfRemoved();

    return getOps()->getDistributedLocks()->
        getInfo(this, ClusterlibStrings::NOTIFYABLELOCK, id, msecs);
}

void
NotifyableImpl::acquireOwnership()
{
    TRACE(CL_LOG, "acquireOwnership");
    
     getOps()->getDistributedLocks()->acquireWaitUsecs(
         -1,
         this,
         ClusterlibStrings::OWNERSHIP_LOCK);
}

bool
NotifyableImpl::acquireOwnershipWaitMsecs(int64_t msecTimeout)
{
    TRACE(CL_LOG, "acquireOwnershipWaitMsecs");

    return getOps()->getDistributedLocks()->acquireWaitUsecs(
        msecTimeout * 1000,
        this,
        ClusterlibStrings::OWNERSHIP_LOCK);
}

void
NotifyableImpl::releaseOwnership()
{
    TRACE(CL_LOG, "releaseOwnership");

    getOps()->getDistributedLocks()->release(
        this,
        ClusterlibStrings::OWNERSHIP_LOCK);
}

bool
NotifyableImpl::hasOwnership()
{
    TRACE(CL_LOG, "hasOwnership");

    throwIfRemoved();

    return getOps()->getDistributedLocks()->
        hasLock(this, ClusterlibStrings::OWNERSHIP_LOCK);
}

bool
NotifyableImpl::getOwnershipInfo(std::string *id, 
                                 int64_t *msecs)
{
    TRACE(CL_LOG, "getOwnershipInfo");

    throwIfRemoved();

    return getOps()->getDistributedLocks()->
        getInfo(this, ClusterlibStrings::OWNERSHIP_LOCK, id, msecs);
}

/*
 * AC - Needs to be fixed to not have to "lock" to find the locks or
 * else it will be worthless 
 */
NameList
NotifyableImpl::getLockBids(bool children)
{
    TRACE(CL_LOG, "getLockBids");

    string lockKey = NotifyableKeyManipulator::createLockKey(
        getKey(),
        ClusterlibStrings::NOTIFYABLELOCK);

    NameList tmpBidList;
    NameList finalBidList;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(lockKey,
                                                            tmpBidList),
                 "getLockBids: Getting bids for group %s failed: %s",
                 lockKey.c_str(),
                 false,
                 true);
    finalBidList.insert(finalBidList.end(),
                        tmpBidList.begin(), 
                        tmpBidList.end());
    if (children) {
        NotifyableList notifyableList = getMyChildren();
        NotifyableList::iterator notifyableListIt;
        for (notifyableListIt = notifyableList.begin();
             notifyableListIt != notifyableList.end();
             ++notifyableListIt) {
            tmpBidList = (*notifyableListIt)->getLockBids(children);
            finalBidList.insert(finalBidList.end(),
                                tmpBidList.begin(), 
                                tmpBidList.end());
        }
    }
    return finalBidList;
}

void
NotifyableImpl::remove(bool removeChildren)
{
    TRACE(CL_LOG, "remove");

    throwIfRemoved();

    Notifyable *parent = getMyParent();
    if (parent == NULL) {
        throw InvalidMethodException(
            "remove: Can not remove a Notifyable that has no parent");
    }

    LOG_DEBUG(CL_LOG, 
              "remove: removing %s (removeChildren=%d)", 
              getKey().c_str(),
              removeChildren); 

    /*
     * Algorithm: 
     *
     * 1. To be completely safe, must acquire exclusive locks on the
     * parent of this Notifyable and this Notifyable.  Also, lock all
     * the way down to the leaf nodes if removeChildren == true.  This
     * guarantees that nobody will try to modify this Notifyable at
     * the same time.
     *
     * Any failures to lock will cause failure.
     *
     * 2. Mark this Notifyables removed from the leaf node (or current
     * node if removeChildren == false) all the way up to the current
     * Notifyable so that other clients can immediately notice
     * deletions in the correct order.
     *
     * Set state after the removal of Repository entries so that if an
     * exception is thrown, the state isn't changed.
     *
     * In order to be sure the the effect of this remove will have
     * been reached at the end of this call, call a sync() and then
     * wait for the sync to have completed.  This guarantees that our
     * remove event was already propagated through the clusterlib
     * 'external' event thread. 
     */

    getMyParent()->acquireLock();
    acquireLock(removeChildren);

    try {
        if (removeChildren == false) {
            NotifyableList ntList = getMyChildren();
            if (ntList.empty() == false) {
                LOG_ERROR(CL_LOG,
                          "remove: Tried to remove a single Notifyable with %"
                          PRIuPTR " children",
                          ntList.size());
                throw InvalidMethodException(
                    "remove: Tried to remove a Notifyable "
                    "with children");
            }
            getOps()->removeCachedNotifyable(this);
            removeRepositoryEntries();
            
            /* Must release locks before try to clean up from removed cache */
            parent->releaseLock();
            releaseLock(removeChildren);

            getOps()->synchronize();
            removeFromRemovedNotifyablesIfReleased(false);
        }
        else {
            NotifyableList ntList, tmpNtList;
            ntList.push_back(this);
            uint32_t ntListIndex = 0;
            do {
                LOG_DEBUG(CL_LOG, 
                          "remove: getting node on %s", 
                          ntList[ntListIndex]->getKey().c_str());
                tmpNtList = ntList[ntListIndex]->getMyChildren();

                /*
                 * Invalidates all iterators (therefore use of index)
                 */
                ntList.insert(ntList.end(), 
                              tmpNtList.begin(),
                              tmpNtList.end()); 
                ntListIndex++;
            } while (ntListIndex != ntList.size());

            NotifyableList::reverse_iterator revNtListIt;
            for (revNtListIt = ntList.rbegin(); 
                 revNtListIt != ntList.rend(); 
                 revNtListIt++) {
                NotifyableImpl *curNtp = 
                    dynamic_cast<NotifyableImpl *>(*revNtListIt);
                LOG_DEBUG(CL_LOG, 
                          "remove: removing %s", 
                          curNtp->getKey().c_str()); 
                getOps()->removeCachedNotifyable(curNtp);
                curNtp->removeRepositoryEntries(); 
            }

            /* Must release locks before try to clean up from removed cache */
            parent->releaseLock();
            releaseLock(removeChildren);

            /* Release notifyables from getChildren() */
            for (revNtListIt = ntList.rbegin(); 
                 revNtListIt != ntList.rend(); 
                 revNtListIt++) {
                if (*revNtListIt != this) {
                    (*revNtListIt)->releaseRef();
                }
            }
            getOps()->synchronize();
            removeFromRemovedNotifyablesIfReleased(false); 
        }    
    } 
    catch (Exception &e) {
        parent->releaseLock();
        releaseLock(removeChildren);

        throw Exception(
            string("remove: released lock becauase of exception: ") +
            e.what());
    }
}

CachedState &
NotifyableImpl::cachedCurrentState()
{
    return m_cachedCurrentState;
}

CachedState &
NotifyableImpl::cachedDesiredState()
{
    return m_cachedDesiredState;
}

void
NotifyableImpl::initialize()
{
    TRACE(CL_LOG, "initialize");

    m_cachedCurrentState.loadDataFromRepository(false);
    m_cachedDesiredState.loadDataFromRepository(false);

    initializeCachedRepresentation();
}

void
NotifyableImpl::removeRepositoryEntries()
{
    TRACE(CL_LOG, "removeRepositoryEntries");
    
    SAFE_CALL_ZK(getOps()->getRepository()->deleteNode(getKey(), true),
                 "Could not delete key %s: %s",
                 getKey().c_str(),
                 false,
                 true);
}

const string
NotifyableImpl::getDistributedLockOwner(const string &lockName)
{
    Locker l1(getSyncLock());
    map<string, NameRef>::iterator lockIt = m_distLockMap.find(lockName);
    if (lockIt == m_distLockMap.end()) {
        return string();
    }
    else {
        return lockIt->second.lockOwner;
    }
}

void
NotifyableImpl::setDistributedLockOwner(const string &lockName,
                                        const string &lockOwner)
{ 
    Locker l1(getSyncLock());
    map<string, NameRef>::iterator lockIt = m_distLockMap.find(lockName);
    if (lockIt == m_distLockMap.end()) {
        NameRef nameRef(0, lockOwner);
        m_distLockMap.insert(pair<string, NameRef>(lockName, nameRef));
    }
    else {
        if (lockIt->second.refCount != 0) {
            LOG_ERROR(CL_LOG, 
                      "setDistributedLockOwnerKey: refCount = %d for for old "
                      "lock owner %s while trying to set new lock owner %s",
                      lockIt->second.refCount,
                      lockIt->second.lockOwner.c_str(),
                      lockOwner.c_str());
            throw InconsistentInternalStateException(
                string("setDistributedLockOwnerKey: Impossible that "
                       "refCount is non-zero for key ") + getKey());
        }

        lockIt->second.lockOwner = lockOwner;
    }
}

int32_t
NotifyableImpl::incrDistributedLockOwnerCount(const string &lockName)
{
    Locker l1(getSyncLock());
    map<string, NameRef>::iterator lockIt = m_distLockMap.find(lockName);
    if (lockIt == m_distLockMap.end()) {
        LOG_ERROR(CL_LOG, 
                  "incrDistributedLockOwnerCount: Couldn't find lockName %s",
                  lockName.c_str());
        throw InconsistentInternalStateException(
            "incrDistributedLockOwnerCount: Failed");
    }

    if (lockIt->second.refCount >= 0) {
        lockIt->second.refCount++;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "incrDistributedLockOwnerCount: Impossible "
                  "refCount %d",
                  lockIt->second.refCount);
        throw InconsistentInternalStateException(
            "incrDistributedLockLeyCount: Failed");
    }

    return lockIt->second.refCount;
}
    
int32_t
NotifyableImpl::decrDistributedLockOwnerCount(const string &lockName)
{
    Locker l1(getSyncLock());
    map<string, NameRef>::iterator lockIt = m_distLockMap.find(lockName);
    if (lockIt == m_distLockMap.end()) {
        LOG_ERROR(CL_LOG, 
                  "decrDistributedLockOwnerCount: Couldn't find lockName %s",
                  lockName.c_str());
        throw InconsistentInternalStateException(
            "decrDistributedLockOwnerCount: Failed");
    }

    if (lockIt->second.refCount > 0) {
        lockIt->second.refCount--;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "decrDistributedLockOwnerCount: Impossible "
                  "refCount %d",
                  lockIt->second.refCount);
        throw InconsistentInternalStateException(
            "decrDistributedLockLeyCount: Failed");
    }

    return lockIt->second.refCount;
}

int32_t
NotifyableImpl::getDistributedLockOwnerCount(const string &lockName)
{
    Locker l1(getSyncLock());
    map<string, NameRef>::iterator lockIt = m_distLockMap.find(lockName);
    if (lockIt == m_distLockMap.end()) {
        LOG_ERROR(CL_LOG, 
                  "getDistributedLockOwnerCount: Couldn't find lockName %s",
                  lockName.c_str());
        throw InconsistentInternalStateException(
            "getDistributedLockOwnerCount: Failed");
    }

    return lockIt->second.refCount;
}

void
NotifyableImpl::incrRefCount()
{
    TRACE(CL_LOG, "incrRefCount");

    Locker l1(getRefCountLock());
    if ((m_refCount < 0) || 
        (m_refCount == numeric_limits<int32_t>::max())) {
        throw InconsistentInternalStateException(
            std::string("incrRefCount: Impossible that reference ") +
                "count for Notifyable " + getKey().c_str() + 
            " is <= 0 or maxed out!");
    }
    
    m_refCount++;

    LOG_DEBUG(CL_LOG, 
              "incrRefCount: Notifyable (%s) now has %d references",
              getKey().c_str(),
              m_refCount);
}

void
NotifyableImpl::decrRefCount()
{
    TRACE(CL_LOG, "decrRefCount");
    
    Locker l1(getRefCountLock());
    if (m_refCount <= 0) {
        throw InconsistentInternalStateException(
            std::string("decrRefCount: Impossible that reference count") +
            " for Notifyable " + getKey().c_str() + " is <= 0!");
    }

    m_refCount--;

    LOG_DEBUG(CL_LOG, 
              "decrRefCount: Notifyable (%s) now has %d references",
              getKey().c_str(),
              m_refCount);
}

void
NotifyableImpl::removeFromRemovedNotifyablesIfReleased(bool decrRefCount)
{
    TRACE(CL_LOG, "removeFromRemovedNotifyablesIfReleased");

    getSyncLock()->acquire();

    if (decrRefCount == true) {
        NotifyableImpl::decrRefCount();
    }

    /* 
     * If the object was removed and the reference count == 0, then no
     * user has a valid pointer to it and no one will be able to get
     * another pointer to it.  It is safe to remove this object from
     * the removed object cache.
     */
    if ((getRefCount() == 0) && (getState() == REMOVED)) {
        Locker l2(getOps()->getRemovedNotifyablesLock());
        set<Notifyable *> *notifyableSet = 
            getOps()->getRemovedNotifyables();
        set<Notifyable *>::iterator it = notifyableSet->find(this);
        if (it == notifyableSet->end()) {
            throw InconsistentInternalStateException(
                string("release: Couldn't find notifyable ") + 
                getKey().c_str() + " in removed notifyables list to release!");
        }
        LOG_DEBUG(CL_LOG, 
                  "removeFromRemovedNotifyablesIfReleased: Cleaned up (%s)",
                  getKey().c_str());
        notifyableSet->erase(it);
        getSyncLock()->release();
        delete this;
    }
    else {
        LOG_DEBUG(CL_LOG, 
                  "removeFromRemovedNotifyablesIfReleased: Did not "
                  "clean up (%s) with %d references",
                  getKey().c_str(),
                  getRefCount());
        getSyncLock()->release();
    }
}

Mutex *
NotifyableImpl::getRefCountLock()
{
    TRACE(CL_LOG, "getRefCountLock");

    return &m_refCountLock;
}


string
NotifyableImpl::createStateJSONArrayKey(const string &notifyableKey,
                                        CachedStateImpl::StateType stateType)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    if (stateType == CachedStateImpl::CURRENT_STATE) {
        res.append(ClusterlibStrings::CURRENT_STATE_JSON_VALUE);
    }
    else if (stateType == CachedStateImpl::DESIRED_STATE) {
        res.append(ClusterlibStrings::DESIRED_STATE_JSON_VALUE);
    }
    else {
        ostringstream oss;
        oss << "createStateJSONArrayKey: Invalid StateType " << stateType;
        throw InvalidArgumentsException(oss.str());
    }

    return res;
}

};	/* End of 'namespace clusterlib' */
