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

using namespace std;
using namespace boost;

namespace clusterlib {

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
const string Notifyable::NOTIFYABLE_STATE_MAINTAINING_VALUE = 
    "_notifyableStateMaintaining";
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

bool
NotifyableImpl::getPropertyListWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<PropertyList> *pPropertyListSP) 
{
    TRACE(CL_LOG, "getPropertyListWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        CLString::REGISTERED_PROPERTYLIST_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pPropertyListSP = dynamic_pointer_cast<PropertyList>(notifyableSP);
    return completed;
}

shared_ptr<PropertyList>
NotifyableImpl::getPropertyList(const string &name,
                                AccessType accessType)
{
    shared_ptr<PropertyList> propertyListSP;
    getPropertyListWaitMsecs(name, accessType, -1, &propertyListSP);
    return propertyListSP;
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

bool
NotifyableImpl::getQueueWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<Queue> *pQueueSP) 
{
    TRACE(CL_LOG, "getQueueWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        CLString::REGISTERED_QUEUE_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pQueueSP = dynamic_pointer_cast<Queue>(notifyableSP);
    return completed;
}

shared_ptr<Queue>
NotifyableImpl::getQueue(const string &name,
                                AccessType accessType)
{
    shared_ptr<Queue> queueSP;
    getQueueWaitMsecs(name, accessType, -1, &queueSP);
    return queueSP;
}

bool
NotifyableImpl::operator==(const Notifyable &other)
{
    return (other.getKey() == getKey()) ? true : false;
}

const string &
NotifyableImpl::getName() const
{
    return m_name; 
}

const string &
NotifyableImpl::getKey() const
{
    return m_key;
}

shared_ptr<Notifyable>
NotifyableImpl::getMyParent() const
{
    TRACE(CL_LOG, "getMyParent");

    throwIfRemoved();

    if (mp_parent == NULL) {
        throw InvalidArgumentsException(string("NULL parent for ") +
                                        getKey());
    }

    return dynamic_pointer_cast<Notifyable>(mp_parent);
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
        shared_from_this(),
        CLString::REGISTERED_PROPERTYLIST_NAME,
        getPropertyListNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());
    tmpList = getOps()->getNotifyableList(
        shared_from_this(),
        CLString::REGISTERED_QUEUE_NAME,
        getQueueNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());
    
    tmpList = getChildrenNotifyables();
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());

    return finalList;
}

bool
NotifyableImpl::getMyApplicationWaitMsecs(
    int64_t msecTimeout,
    shared_ptr<Application> *pApplicationSP)
{
    TRACE(CL_LOG, "getMyApplicationWaitMsecs");

    throwIfRemoved();

    /*
     * Try to find the object.
     *
     * An application is its own application, according to the
     * semantics implemented below.
     */
    bool completed = false;
    string applicationKey = getKey();
    shared_ptr<NotifyableImpl> notifyableSP;
    do {
        completed = getOps()->getNotifyableFromKeyWaitMsecs(
            vector<string>(1, 
                           CLString::REGISTERED_APPLICATION_NAME),
            applicationKey, 
            LOAD_FROM_REPOSITORY,
            msecTimeout,
            &notifyableSP);
        *pApplicationSP = dynamic_pointer_cast<Application>(notifyableSP);
        applicationKey = 
            NotifyableKeyManipulator::removeObjectFromKey(applicationKey);
    }  while ((*pApplicationSP == NULL) && (!applicationKey.empty()));

    return completed;
}

shared_ptr<Application>
NotifyableImpl::getMyApplication()
{
    TRACE(CL_LOG, "getMyApplication");

    shared_ptr<Application> applicationSP;
    getMyApplicationWaitMsecs(-1, &applicationSP);
    return applicationSP;
}

bool
NotifyableImpl::getMyGroupWaitMsecs(
    int64_t msecTimeout,
    shared_ptr<Group> *pGroupSP)
{
    TRACE(CL_LOG, "getMyGroupWaitMsecs");

    if (pGroupSP == NULL) {
        throw InvalidArgumentsException(
            "getMyGroupWaitMsecs: pGroupSP == NULL");
    }

    throwIfRemoved();

    /*
     * Try to find the object.
     *
     * An group is its own group, according to the
     * semantics implemented below.
     */
    bool completed = false;
    string groupKey = getKey();
    shared_ptr<NotifyableImpl> notifyableSP;
    do {
        groupKey = NotifyableKeyManipulator::removeObjectFromKey(groupKey);
        completed = getOps()->getNotifyableFromKeyWaitMsecs(
            vector<string>(1, 
                           CLString::REGISTERED_GROUP_NAME),
            groupKey, 
            LOAD_FROM_REPOSITORY,
            msecTimeout,
            &notifyableSP);
        *pGroupSP = dynamic_pointer_cast<Group>(notifyableSP);
    }  while ((*pGroupSP == NULL) && (!groupKey.empty()));

    return completed;
}

shared_ptr<Group>
NotifyableImpl::getMyGroup()
{
    TRACE(CL_LOG, "getMyGroup");

    shared_ptr<Group> groupSP;
    getMyGroupWaitMsecs(-1, &groupSP);
    return groupSP;
}

bool
NotifyableImpl::getNotifyableFromKeyWaitMsecs(
    const string &key,
    int64_t msecTimeout,
    shared_ptr<Notifyable> *pNotifyableSP)
{
    TRACE(CL_LOG, "getNotifyableFromKeyWaitMsecs");

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed =  getOps()->getNotifyableFromKeyWaitMsecs(
        vector<string>(), 
        key,
        LOAD_FROM_REPOSITORY,
        msecTimeout,
        &notifyableSP);

    *pNotifyableSP = dynamic_pointer_cast<Notifyable>(notifyableSP);
    return completed;
}

shared_ptr<Notifyable> 
NotifyableImpl::getNotifyableFromKey(const string &key) 
{
    TRACE(CL_LOG, "getNotifyableFromKeyWaitMsecs");
    
    shared_ptr<Notifyable> notifyableSP;

    getNotifyableFromKeyWaitMsecs(key,
                                  -1,
                                  &notifyableSP);
    return notifyableSP;
}

Notifyable::State
NotifyableImpl::getState() const
{
    TRACE(CL_LOG, "getState");

    Locker l(getSyncLock());

    LOG_DEBUG(CL_LOG, 
              "getState: State for (%s) is %s", 
              getKey().c_str(), 
              NotifyableImpl::getStateString(m_state).c_str());
    return m_state;
}

const Mutex &
NotifyableImpl::getSyncLock() const
{
    TRACE(CL_LOG, "getSyncLock");

    LOG_DEBUG(CL_LOG, "getSyncLock: For notifyable %s", getKey().c_str());

    return m_syncLock;
}

const Mutex &
NotifyableImpl::getSyncDistLock() const
{
    TRACE(CL_LOG, "getSyncDistLock");

    LOG_DEBUG(CL_LOG, "getSyncDistLock: For notifyable %s", getKey().c_str());

    return m_syncDistLock;
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
NotifyableImpl::acquireLock(const string &lockName,
                            DistributedLockType distributedLockType,
                            bool acquireChildren)
{
    TRACE(CL_LOG, "acquireLock");
    
    if (!acquireLockWaitMsecs(
            lockName, distributedLockType, -1LL, acquireChildren)) {
        throw InconsistentInternalStateException(
            "acquireLock: Impossible that acquireLockWaitMsecs failed!");
    }
}

bool
NotifyableImpl::acquireLockWaitMsecs(const string &lockName,
                                     DistributedLockType distributedLockType,
                                     int64_t msecTimeout, 
                                     bool acquireChildren)
{
    TRACE(CL_LOG, "acquireLockWaitMsecs");

    throwIfRemoved();

    if (msecTimeout < -1) {
        ostringstream oss;
        oss << "acquireLockWaitMsecs: Cannot have msecTimeout < -1 (" 
	    << msecTimeout << ")";
        throw InvalidArgumentsException(oss.str());
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
    ntList.push_back(dynamic_pointer_cast<Notifyable>(shared_from_this()));
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
            lockName,
            distributedLockType);
        if (!gotLock) {
            break;
        }

        if (true == acquireChildren) {
            tmpNtList = ntList[ntListIndex]->getMyChildren();
            /* Invalidates all iterators (therefore use of index) */
            ntList.insert(ntList.end(), tmpNtList.begin(), tmpNtList.end()); 
        }

        ++ntListIndex;
    } while (ntListIndex != ntList.size());

    /* Release the acquired locks if the last lock attempt failed */
    if (!gotLock && (ntListIndex > 0)) {
        for (int i = ntListIndex - 1; i >= 0; --i) {
            ntList[ntListIndex]->releaseLock(lockName, acquireChildren);
        }
    }

    return gotLock;
}

void 
NotifyableImpl::releaseLock(const string &lockName, bool releaseChildren)
{
    TRACE(CL_LOG, "releaseLock");

    LOG_DEBUG(CL_LOG, 
              "releaseLock: releasing lock on %s", 
              getKey().c_str()); 
    if (releaseChildren == true) {
        NotifyableList ntList, tmpNtList;
        ntList.push_back(shared_from_this());
        uint32_t ntListIndex = 0;
        do {
            LOG_DEBUG(CL_LOG, 
                      "releaseLock: releasing lock on %s", 
                      ntList[ntListIndex]->getKey().c_str());
            try {
                getOps()->getDistributedLocks()->release(
                    ntList[ntListIndex],
                    lockName);
            } catch (ObjectRemovedException &e) {
                LOG_DEBUG(CL_LOG, 
                          "releaseLock: Object %s no longer exists "
                          "(most likely I deleted it) - %s",
                          getKey().c_str(),
                          e.what());
            }
            try {
                tmpNtList = ntList[ntListIndex]->getMyChildren();
            } catch (ObjectRemovedException &e) {
                LOG_INFO(CL_LOG,
                         "releaseLock: Getting children of %s failed, likely "
                         "since they no longer exist - %s",
                         ntList[ntListIndex]->getKey().c_str(),
                         e.what());
                return;
            }

            /*
             * Invalidates all iterators (therefore use of index)
             */
            ntList.insert(ntList.end(), tmpNtList.begin(), tmpNtList.end()); 
            ntListIndex++;
        } while (ntListIndex != ntList.size());
    }
    else {
        getOps()->getDistributedLocks()->release(
            shared_from_this(),
            lockName);
    }
}

bool
NotifyableImpl::hasLock(const string &lockName,
                        DistributedLockType *pDistributedLockType)
{
    TRACE(CL_LOG, "hasLock");

    throwIfRemoved();

    return getOps()->getDistributedLocks()->hasLock(
        shared_from_this(), lockName, pDistributedLockType);
}

bool
NotifyableImpl::getLockInfo(const string &lockName,
                            string *pId, 
                            DistributedLockType *pDistributedLockType,
                            int64_t *pMsecs)
{
    TRACE(CL_LOG, "getLockInfo");

    throwIfRemoved();

    return getOps()->getDistributedLocks()->getInfo(
        shared_from_this(), 
        lockName,
        pId, 
        pDistributedLockType,
        pMsecs);
}

NameList
NotifyableImpl::getLockBids(const string &lockName,
                            bool children)
{
    TRACE(CL_LOG, "getLockBids");

    vector<string> finalBidList;
    vector<string> notifyableVec;
    size_t notifyableVecIndex = 0;
    notifyableVec.push_back(getKey());
    while (notifyableVecIndex != notifyableVec.size()) {
        vector<string> lockKeyVec;
        if (!lockName.empty()) {
            lockKeyVec.push_back(NotifyableKeyManipulator::createLockKey(
                notifyableVec[notifyableVecIndex], lockName));
        }
        else {
            SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                NotifyableKeyManipulator::createLocksKey(
                    notifyableVec[notifyableVecIndex]),
                    lockKeyVec),
                         "getLockBids: Getting bids for "
                         "Notifyable %s failed: %s",
                         notifyableVec[notifyableVecIndex].c_str(),
                         false,
                true);
        }

        vector<string> tmpBidList;
        vector<string>::const_iterator lockKeyVecIt;
        for (lockKeyVecIt = lockKeyVec.begin();
             lockKeyVecIt != lockKeyVec.end();
             ++lockKeyVecIt) {
            SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                *lockKeyVecIt,
                tmpBidList),
                         "getLockBids: Getting bids for lock key "
                         "%s failed: %s",
                         lockKeyVecIt->c_str(),
                         false,
                         true);
            finalBidList.insert(finalBidList.end(),
                                tmpBidList.begin(), 
                                tmpBidList.end());
        }

        if (children) {
            vector<string> childNotifyableTypeVec;
            SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                notifyableVec[notifyableVecIndex],
                childNotifyableTypeVec),
                         "getLockBids: Getting bids for notifyable %s "
                         "failed: %s",
                         notifyableVec[notifyableVecIndex].c_str(),
                         false,
                         true);
        
            vector<string> tmpChildNotifyableVec;
            vector<string>::const_iterator childNotifyableTypeVecIt;
            vector<string>::const_iterator tmpChildNotifyableVecIt;
            for (childNotifyableTypeVecIt = childNotifyableTypeVec.begin();
                 childNotifyableTypeVecIt != childNotifyableTypeVec.end();
                 ++childNotifyableTypeVecIt) {
                SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                    *childNotifyableTypeVecIt,
                    tmpChildNotifyableVec),
                             "getLockBids: Getting bids for notifyable "
                             "type %s failed: %s",
                             childNotifyableTypeVecIt->c_str(),
                             false,
                             true);
                for (tmpChildNotifyableVecIt = tmpChildNotifyableVec.begin();
                     tmpChildNotifyableVecIt != tmpChildNotifyableVec.end();
                 ++tmpChildNotifyableVecIt) {
                    if (getOps()->isValidKey(vector<string>(),
                                             *tmpChildNotifyableVecIt)) {
                        notifyableVec.push_back(*tmpChildNotifyableVecIt);
                    }
                }
            }
        }
        ++notifyableVecIndex;
    }
    return finalBidList;
}

void
NotifyableImpl::remove(bool removeChildren)
{
    TRACE(CL_LOG, "remove");

    throwIfRemoved();

    shared_ptr<Notifyable> parent = getMyParent();
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

    parent->acquireLock(CLString::CHILD_LOCK, DIST_LOCK_EXCL);
    acquireLock(CLString::CHILD_LOCK, 
                DIST_LOCK_EXCL, 
                removeChildren);

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
            getOps()->removeCachedNotifyable(shared_from_this());
            removeRepositoryEntries();
        }
        else {
            NotifyableList ntList, tmpNtList;
            ntList.push_back(shared_from_this());
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
                ++ntListIndex;
            } while (ntListIndex != ntList.size());

            NotifyableList::reverse_iterator revNtListIt;
            for (revNtListIt = ntList.rbegin(); 
                 revNtListIt != ntList.rend(); 
                 ++revNtListIt) {
                shared_ptr<NotifyableImpl> curNtp = 
                    dynamic_pointer_cast<NotifyableImpl>(*revNtListIt);
                LOG_DEBUG(CL_LOG, 
                          "remove: removing %s", 
                          curNtp->getKey().c_str()); 
                getOps()->removeCachedNotifyable(
                    shared_ptr<NotifyableImpl>(curNtp));
                curNtp->removeRepositoryEntries(); 
            }
        }    

        /* 
         * Must release locks of parents (not this NotifyableImpl
         * since it was removed).  Call synchronize() to make sure
         * that all the removes have been processed when this method
         * returns.
         */
        parent->releaseLock(CLString::CHILD_LOCK);
        getOps()->synchronize();
    } 
    catch (const ObjectRemovedException &e) {
        parent->releaseLock(CLString::CHILD_LOCK);
        releaseLock(CLString::CHILD_LOCK, removeChildren);

        LOG_ERROR(CL_LOG, 
                  "remove: released lock becauase of exception: %s",
                  e.what());
        throw e;
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

NotifyableImpl::NotifyableImpl(FactoryOps *fp,
                               const string &key,
                               const string &name,
                               const shared_ptr<NotifyableImpl> &parent)
    : mp_f(fp),
      m_key(key),
      m_name(name),
      mp_parent(parent),
      m_state(Notifyable::READY),
      m_safeNotifyableMap(NULL),
      m_cachedCurrentState(this, 
                           CachedStateImpl::CURRENT_STATE),
      m_cachedDesiredState(this,
                           CachedStateImpl::DESIRED_STATE) 
{
    // Note that m_cachedCurrentState and m_cachedDesiredState cannot
    // get the shared_from_this() since it isn't ready until after the
    // contructor.
}

FactoryOps *
NotifyableImpl::getOps()
{
    return mp_f;
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

bool
NotifyableImpl::getDistributedLockOwnerInfo(
    const string &lockName,
    std::string *pHostnamePidTid,
    std::string *pLockKey,
    std::string *pLockNodeCreatedPath,
    DistributedLockType *pDistributedLockType,
    int32_t *pRefCount) const
{
    Locker l(getSyncLock());

    map<string, map<int32_t, NameRef> >::const_iterator nameThreadLockMapIt = 
        m_nameThreadLockMap.find(lockName);
    if (nameThreadLockMapIt == m_nameThreadLockMap.end()) {
        return false;
    }

    map<int32_t, NameRef>::const_iterator threadNameRefIt = 
        nameThreadLockMapIt->second.find(
        ProcessThreadService::getTid());
    if (threadNameRefIt == nameThreadLockMapIt->second.end()) {
        return false;
    }

    if (pHostnamePidTid != NULL) {
        *pHostnamePidTid = threadNameRefIt->second.hostnamePidTid;
    }
    if (pLockKey != NULL) {
        *pLockKey = threadNameRefIt->second.lockKey;
    }
    if (pLockNodeCreatedPath != NULL) {
        *pLockNodeCreatedPath = threadNameRefIt->second.lockNodeCreatedPath;
    }
    if (pDistributedLockType != NULL) {
        *pDistributedLockType = threadNameRefIt->second.distributedLockType;
    }
    if (pRefCount != NULL) {
        *pRefCount = threadNameRefIt->second.refCount;
    }

    return true;
}

void
NotifyableImpl::setDistributedLockOwnerInfo(
    const string &lockName,
    const string &lockKey,
    const string &lockNodeCreatedPath,
    DistributedLockType distributedLockType)
{ 
    Locker l(getSyncLock());

    map<int32_t, NameRef>::iterator threadNameRefIt = 
        m_nameThreadLockMap[lockName].find(
        ProcessThreadService::getTid());
    if (threadNameRefIt == m_nameThreadLockMap[lockName].end()) {
        NameRef nameRef(
            0, lockKey, lockNodeCreatedPath, distributedLockType);
        m_nameThreadLockMap[lockName].insert(
            make_pair<int32_t, NameRef>(ProcessThreadService::getTid(), 
                                        nameRef));
    }
    else {
        if (threadNameRefIt->second.refCount != 0) {
            ostringstream oss;
            oss << "setDistributedLockOwnerInfo: Impossible that old lock "
                << "info still exists with "
                << "lockName=" << lockName
                << ", refCount=" << threadNameRefIt->second.refCount
                << ", hostnamePidTid=" 
                << threadNameRefIt->second.hostnamePidTid
                << ", lockKey=" << threadNameRefIt->second.lockKey
                << ", lockNodeCreatedPath=" 
                << threadNameRefIt->second.lockNodeCreatedPath
                << ", distributedLockType=" 
                << threadNameRefIt->second.distributedLockType << "("
                << distributedLockTypeToString(
                    threadNameRefIt->second.distributedLockType) << ")";
            LOG_ERROR(CL_LOG, "%s", oss.str().c_str());
            throw InconsistentInternalStateException(oss.str());
        }

        threadNameRefIt->second.hostnamePidTid = 
            ProcessThreadService::getHostnamePidTid();        
        threadNameRefIt->second.lockKey = lockKey;
        threadNameRefIt->second.lockNodeCreatedPath = lockNodeCreatedPath;
        threadNameRefIt->second.distributedLockType = distributedLockType;
    }
}

int32_t
NotifyableImpl::incrDistributedLockOwnerCount(const string &lockName)
{
    Locker l(getSyncLock());

    map<string, map<int32_t, NameRef> >::iterator nameThreadLockMapIt = 
        m_nameThreadLockMap.find(lockName);
    if (nameThreadLockMapIt == m_nameThreadLockMap.end()) {
        LOG_ERROR(CL_LOG, 
                  "incrDistributedLockOwnerCount: Couldn't find lockName %s",
                  lockName.c_str());
        throw InconsistentInternalStateException(
            "incrDistributedLockOwnerCount: Failed to get lockName map");
    }

    map<int32_t, NameRef>::iterator threadNameRefIt = 
        nameThreadLockMapIt->second.find(
        ProcessThreadService::getTid());
    if (threadNameRefIt == nameThreadLockMapIt->second.end()) {
        LOG_ERROR(CL_LOG, 
                  "incrDistributedLockOwnerCount: Couldn't find thread id %"
                  PRId32,
                  ProcessThreadService::getTid());
        throw InconsistentInternalStateException(
            "incrDistributedLockOwnerCount: Failed to get thread id map");

    }

    if (threadNameRefIt->second.refCount >= 0) {
        threadNameRefIt->second.refCount++;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "incrDistributedLockOwnerCount: Impossible "
                  "refCount %d",
                  threadNameRefIt->second.refCount);
        throw InconsistentInternalStateException(
            "incrDistributedLockLeyCount: Failed to increment"
            " (negative refCount)");
    }

    return threadNameRefIt->second.refCount;
}
    
int32_t
NotifyableImpl::decrDistributedLockOwnerCount(const string &lockName)
{
    Locker l(getSyncLock());

    map<string, map<int32_t, NameRef> >::iterator nameThreadLockMapIt = 
        m_nameThreadLockMap.find(lockName);
    if (nameThreadLockMapIt == m_nameThreadLockMap.end()) {
        LOG_ERROR(CL_LOG, 
                  "decrDistributedLockOwnerCount: Couldn't find lockName %s",
                  lockName.c_str());
        throw InconsistentInternalStateException(
            "decrDistributedLockOwnerCount: Failed to get lockName map");
    }

    map<int32_t, NameRef>::iterator threadNameRefIt = 
        nameThreadLockMapIt->second.find(
        ProcessThreadService::getTid());
    if (threadNameRefIt == nameThreadLockMapIt->second.end()) {
        LOG_ERROR(CL_LOG, 
                  "decrDistributedLockOwnerCount: Couldn't find thread id %"
                  PRId32,
                  ProcessThreadService::getTid());
        throw InconsistentInternalStateException(
            "decrDistributedLockOwnerCount: Failed to get thread id map");

    }

    int32_t refCount = threadNameRefIt->second.refCount;
    if (refCount == 1) {
        nameThreadLockMapIt->second.erase(threadNameRefIt);
    }
    else if (refCount > 1) {
        --(threadNameRefIt->second.refCount);
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "decrDistributedLockOwnerCount: Impossible "
                  "refCount %" PRId32,
                  refCount);
        throw InconsistentInternalStateException(
            "decrDistributedLockLeyCount: Failed with negative refCount");
    }

    return refCount - 1;
}

bool
NotifyableImpl::isReentrantLockAttempt(
    const string &lockName,
    const string &lockKey,
    DistributedLockType distributedLockType) const
{
    string ownerHostnamePidTid;
    string ownerLockKey;
    int32_t refCount = -1;
    DistributedLockType ownerDistributedLockType;
    bool lockExists = getDistributedLockOwnerInfo(lockName,
                                                  &ownerHostnamePidTid,
                                                  &ownerLockKey,
                                                  NULL,
                                                  &ownerDistributedLockType,
                                                  &refCount);
                                                 
    if (lockExists == false) {
        return false;
    }

    /*
     * If the basic information up to the PID and TID don't match,
     * this is no problem, just return false.  If the PID and TID
     * match, then it is re-entrant only if the DistributedLockType
     * matches.  If the DistributedLockType doesn't matches, the user
     * is trying to get a different DistributedLockType on a lock it
     * already has!
     */
    if (lockKey.compare(ownerLockKey) != 0) {
        ostringstream oss;
        oss << "isReentrantLockAttempt: Impossible that the lockKey="
            << lockKey << " and ownerLockKey=" << ownerLockKey;
        throw InconsistentInternalStateException(oss.str());
    }

    if (ProcessThreadService::getHostnamePidTid().compare(
            ownerHostnamePidTid) != 0) {
        return false;
    }
    else {
        if (distributedLockType != ownerDistributedLockType) {
            ostringstream oss;
            oss << "isReentrantLockAttempt: Lock is already held for "
                << ownerHostnamePidTid << " in lock key "
                << ownerLockKey << " under type " 
                << distributedLockTypeToString(ownerDistributedLockType)
                << " and can only be reentrant if the same type.  The "
                << "attempted reentrant lock is of type "
                << distributedLockTypeToString(distributedLockType);
            LOG_ERROR(CL_LOG, "%s", oss.str().c_str());
            throw InvalidArgumentsException(oss.str());
        }
        else {
            return true;
        }
    }
}

string
NotifyableImpl::createStateJSONArrayKey(const string &notifyableKey,
                                        CachedStateImpl::StateType stateType)
{
    string res;
    res.append(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    if (stateType == CachedStateImpl::CURRENT_STATE) {
        res.append(CLStringInternal::CURRENT_STATE_JSON_VALUE);
    }
    else if (stateType == CachedStateImpl::DESIRED_STATE) {
        res.append(CLStringInternal::DESIRED_STATE_JSON_VALUE);
    }
    else {
        ostringstream oss;
        oss << "createStateJSONArrayKey: Invalid StateType " << stateType;
        throw InvalidArgumentsException(oss.str());
    }

    return res;
}

}	/* End of 'namespace clusterlib' */
