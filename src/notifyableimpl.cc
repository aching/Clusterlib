/*
 * notifyableimpl.cc
 *
 * Implementation of the notification classes outlined methods.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"
#include <limits>

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;

namespace clusterlib
{

NameList
NotifyableImpl::getPropertyListNames() 
{
    TRACE(CL_LOG, "getPropertyListNames");

    throwIfRemoved();

    return getOps()->getPropertyListNames(this);
}

PropertyList *
NotifyableImpl::getPropertyList(const std::string &name, bool create)
{
    TRACE(CL_LOG, "getPropertyList");

    throwIfRemoved();

    return getOps()->getPropertyList(name, this, create);
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

Application *
NotifyableImpl::getMyApplication()
{
    TRACE(CL_LOG, "getMyApplication");

    throwIfRemoved();

    /*
     * Try to find the object.
     *
     * An application is its own application, according to the
     * semantics implemented below...
     */
    string appKey = getKey();
    Application *myApp = NULL;
    do {
        myApp = getOps()->getApplicationFromKey(appKey, false);
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
        myGroup = getOps()->getGroupFromKey(groupKey, false);
    } while ((myGroup == NULL) && (!groupKey.empty()));

    return myGroup;
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

    throwIfRemoved();

    if (getMyParent() == NULL) {
        throw InvalidMethodException(
            "acquireLock: Can not lock a Notifyable that has no parent "
            "(most likely because it is the Root!)");
    }

    getOps()->getDistributedLocks()->acquire(
        getMyParent(), 
        ClusterlibStrings::NOTIFYABLELOCK);

    LOG_DEBUG(CL_LOG, 
              "acquireLock: acquring parent lock on %s", 
              getKey().c_str()); 
    if (acquireChildren == true) {
        NotifyableList ntList, tmpNtList;
        ntList.push_back(this);
        uint32_t ntListIndex = 0;
        do {
            LOG_DEBUG(CL_LOG, 
                      "acquireLock: acquiring lock on %s", 
                      ntList[ntListIndex]->getKey().c_str());
            getOps()->getDistributedLocks()->acquire(
                ntList[ntListIndex],
                ClusterlibStrings::NOTIFYABLELOCK);
            tmpNtList = getOps()->getChildren(ntList[ntListIndex]);

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
        getOps()->getDistributedLocks()->acquire(
            this,
            ClusterlibStrings::NOTIFYABLELOCK);
    }
}

void 
NotifyableImpl::releaseLock(bool releaseChildren)
{
    TRACE(CL_LOG, "releaseLock");

    /*
     * Typically, use getMyParent() here, but it is possible we marked
     * it deleted (in which case getMyParent() will throw.  If I have
     * acquired the locks and deleted this node, than no other thread
     * could possibly have deleted the parent until after I have
     * released my locks.  Therefore, it is safe to directly use
     * mp_parent.
     */
    if (mp_parent == NULL) {
        throw InvalidMethodException(
            "releaseLock: Can not unlock a Notifyable that has no parent");
    }

    getOps()->getDistributedLocks()->release(
        mp_parent,
        ClusterlibStrings::NOTIFYABLELOCK);

    LOG_DEBUG(CL_LOG, 
              "releaseLock: releasing parent lock on %s", 
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
                tmpNtList = getOps()->getChildren(ntList[ntListIndex]);
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

void
NotifyableImpl::remove(bool removeChildren)
{
    TRACE(CL_LOG, "remove");

    throwIfRemoved();

    if (getMyParent() == NULL) {
        throw InvalidMethodException(
            "remove: Can not remove a Notifyable that has no parent");
    }

    /*
     * Algorithm: 
     *
     * 1. To be completely safe, must acquire exclusive locks on this
     * Notifyable if removeChildren == false.  Otherwise lock at this
     * Notifyable all the way down to the leaf nodes.  This guarantees
     * that nobody will try to modify this Notifyable at the same
     * time.
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

    acquireLock(removeChildren);

    try {
        if (removeChildren == false) {
            LOG_DEBUG(CL_LOG, 
                      "remove: removing %s", 
                      getKey().c_str()); 

            NotifyableList ntList = getOps()->getChildren(this);
            if (ntList.empty() == false) {
                LOG_ERROR(CL_LOG,
                          "remove: Tried to remove a single Notifyable with "
                          "%u children",
                          ntList.size());
                throw InvalidMethodException(
                    "remove: Tried to remove a Notifyable "
                    "with children");
            }
            getOps()->removeNotifyableFromCacheByKey(getKey());
            removeRepositoryEntries();
            
            /* Must release lock before try to clean up from removed cache */
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
                tmpNtList = getOps()->getChildren(ntList[ntListIndex]);

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
                getOps()->removeNotifyableFromCacheByKey(curNtp->getKey());
                curNtp->removeRepositoryEntries(); 
            }

            /* Must release lock before try to clean up from removed cache */
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
        releaseLock(removeChildren);
        throw Exception(
            string("remove: released lock becauase of exception: ") +
            e.what());
    }
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

};	/* End of 'namespace clusterlib' */

