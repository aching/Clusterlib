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

    Properties *prop = getDelegate()->getProperties(this, create);
    if ((prop == NULL) && (create == true)) {
        throw ClusterException(
            string("getProperties: Couldn't create properties ") + getKey());
    }

    return prop;
}

Notifyable *
NotifyableImpl::getMyParent() const
{
    TRACE(CL_LOG, "getMyParent");

    if (mp_parent == NULL) {
        throw ClusterException(string("NULL parent for ") +
                               getKey());
    }

    return dynamic_cast<Notifyable *>(mp_parent);
}

Application *
NotifyableImpl::getMyApplication()
{
    TRACE(CL_LOG, "getMyApplication");

    /*
     * Try to find the object.
     *
     * An application is its own application, according to the
     * semantics implemented below...
     */
    string appKey = getKey();
    Application *myApp = NULL;
    do {
        myApp = getDelegate()->getApplicationFromKey(appKey, false);
        appKey = NotifyableKeyManipulator::removeObjectFromKey(appKey);
    }  while ((myApp == NULL) && (!appKey.empty()));

    return myApp;
}

Group *
NotifyableImpl::getMyGroup()
{
    TRACE(CL_LOG, "getMyGroup");

    /*
     * Try to find the object.
     */
    string groupKey = getKey();
    Group *myGroup = NULL;
    do {
        groupKey = NotifyableKeyManipulator::removeObjectFromKey(groupKey);
        myGroup = getDelegate()->getGroupFromKey(groupKey, false);
    } while ((myGroup == NULL) && (!groupKey.empty()));

    return myGroup;
}

Notifyable::State
NotifyableImpl::getState()
{
    Locker l1(getStateLock());
    return m_state;
}

void
NotifyableImpl::setState(Notifyable::State state)
{
    Locker l1(getStateLock());
    m_state = state;
}

void
NotifyableImpl::acquireLock()
{
    getDelegate()->getDistrbutedLocks()->acquire(this);
}

void 
NotifyableImpl::releaseLock()
{
    getDelegate()->getDistrbutedLocks()->release(this);
}

void
NotifyableImpl::remove(bool removeChildren)
{
    TRACE(CL_LOG, "remove");

    if (getMyParent() == NULL) {
        throw ClusterException("remove: Can not remove a Notifyable that "
                               "has no parent");
    }

    /*
     * Algorithm: 
     *
     * 1. To be completely safe, must acquire exclusive locks on
     * parent and at current level if removeChildren == false).
     * Otherwise lock at parent all the way down to the leaf nodes.
     * This guarantees that nobody will try to modify this Notifyable
     * at the same time.
     *
     * Any failures to lock will cause failure.
     *
     * 2. Mark this Notifyables removed from the leaf node (or current
     * node if removeChildren == false) all the way up so that other
     * nodes can immediately notice it in the correct order.
     */
    
    /*
     * Mark the Notifyable removed
     */
    setState(Notifyable::REMOVED);
    
    /*
     * Call the Notifyable-specific clean up function.
     */
    removeRepositoryEntries();
}

};	/* End of 'namespace clusterlib' */

