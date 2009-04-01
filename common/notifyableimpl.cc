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

};	/* End of 'namespace clusterlib' */

