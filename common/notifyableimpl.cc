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

    Locker l(getChainLock());

    /*
     * If the properties list is already cached, return it.
     */
    if (mp_myProperties != NULL) {
        return mp_myProperties;
    }

    /*
     * Cache it.
     */
    mp_myProperties = getDelegate()->getProperties(this, create);

    return mp_myProperties;
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

    Locker l(getChainLock());
    string appKey = getKey();

    /*
     * If it's cached already, just return it.
     */
    if (mp_myApplication != NULL) {
        return mp_myApplication;
    }

    /*
     * Try to find the object.
     *
     * An application is its own application, according to the
     * semantics implemented below...
     */
    do {
        mp_myApplication =
            getDelegate()->getApplicationFromKey(appKey, false);
        appKey = NotifyableKeyManipulator::removeObjectFromKey(appKey);
    }  while ((mp_myApplication == NULL) && (!appKey.empty()));

    return mp_myApplication;
}

Group *
NotifyableImpl::getMyGroup()
{
    TRACE(CL_LOG, "getMyGroup");

    Locker l(getChainLock());
    string groupKey = getKey();

    /*
     * If it's already cached, just return it.
     */
    if (mp_myGroup != NULL) {
        return mp_myGroup;
    }

    /*
     * Try to find the object.
     */
    do {
        groupKey = NotifyableKeyManipulator::removeObjectFromKey(groupKey);
        mp_myGroup = getDelegate()->getGroupFromKey(groupKey, false);
    } while ((mp_myGroup == NULL) && (!groupKey.empty()));

    return mp_myGroup;
}

};	/* End of 'namespace clusterlib' */

