/*
 * notifyableimpl.cc
 *
 * Implementation of the notification classes outlined methods.
 *
 * =============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;

namespace clusterlib
{

Properties *
NotifyableImpl::getProperties(bool create)
{
    TRACE(CL_LOG, "getProperties");

    throwIfRemoved();

    return getOps()->getProperties(this, create);
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
    Locker l1(getStateLock());
    return m_state;
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

    getOps()->getDistrbutedLocks()->acquire(getMyParent());
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
            getOps()->getDistrbutedLocks()->acquire(ntList[ntListIndex]);
            tmpNtList = getOps()->getChildren(ntList[ntListIndex]);

            /*
             * Invalidates all iterators (therefore use of index)
             */
            ntList.insert(ntList.end(), tmpNtList.begin(), tmpNtList.end()); 
            ntListIndex++;
        } while (ntListIndex != ntList.size());
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
     * released my locks.
     */
    if (mp_parent == NULL) {
        throw InvalidMethodException(
            "releaseLock: Can not unlock a Notifyable that has no parent");
    }

    getOps()->getDistrbutedLocks()->release(mp_parent);
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
                getOps()->getDistrbutedLocks()->release(ntList[ntListIndex]);
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
    }
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
        }    
    } 
    catch (Exception &e) {
        releaseLock(removeChildren);
        throw Exception(
            string("remove: released lock becauase of exception: ") +
            e.what());
    }

    releaseLock(removeChildren);
}

const string &
NotifyableImpl::getDistributedLockKey()
{
    Locker l1(getStateLock());
    return m_distLockKey; 
}

void
NotifyableImpl::setDistributedLockKey(const string &distLockKey)
{ 
    Locker l1(getStateLock());
    m_distLockKey = distLockKey; 
}

int32_t
NotifyableImpl::incrDistributedLockKeyCount()
{
    Locker l1(getStateLock());
    if (m_distLockKeyCount >= 0) {
        m_distLockKeyCount++;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "incrDistributedLockLeyCount: Impossible "
                  "m_distLockKeyCount %d",
                  m_distLockKeyCount);
        throw InconsistentInternalStateException(
            "incrDistributedLockLeyCount: Failed");
    }

    return m_distLockKeyCount;
}
    
int32_t
NotifyableImpl::decrDistributedLockLeyCount()
{
    Locker l1(getStateLock());
    if (m_distLockKeyCount < 1) {
        LOG_ERROR(CL_LOG, 
                  "decrDistributedLockLeyCount: Impossible "
                  "m_distLockKeyCount %d",
                  m_distLockKeyCount);
        throw InconsistentInternalStateException(
            "decrDistributedLockLeyCount: Failed");
    }
    else {
        m_distLockKeyCount--;
        if (m_distLockKeyCount == 0) {
            m_distLockKey.clear();
        }
    }

    return m_distLockKeyCount;
}

int32_t
NotifyableImpl::getDistributedLockKeyCount()
{
    Locker l1(getStateLock());
    return m_distLockKeyCount;
}

};	/* End of 'namespace clusterlib' */

