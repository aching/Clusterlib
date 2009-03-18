/*
 * notifyable.cc
 *
 * Implementation of the notification classes outlined methods.
 *
 * =============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#include "clusterlib.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{
Properties *
Notifyable::getProperties(bool create)
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
Notifyable::getMyParent() const
{
    TRACE(CL_LOG, "getMyParent");

    if (mp_parent == NULL) {
        throw ClusterException(string("NULL parent for ") +
                               getKey());
    }

    return mp_parent;
}

Application *
Notifyable::getMyApplication()
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
        appKey = getDelegate()->removeObjectFromKey(appKey);
    }  while ((mp_myApplication == NULL) && (!appKey.empty()));

    return mp_myApplication;
}

Group *
Notifyable::getMyGroup()
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
        groupKey = getDelegate()->removeObjectFromKey(groupKey);
        mp_myGroup = getDelegate()->getGroupFromKey(groupKey, false);
    } while ((mp_myGroup == NULL) && (!groupKey.empty()));

    return mp_myGroup;
}

};	/* End of 'namespace clusterlib' */

