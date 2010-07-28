/*
 * rootmpl.cc --
 *
 * Implementation of the Root class; it represents a set of
 * applications in a clusterlib instance.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;

namespace clusterlib
{

NameList
RootImpl::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createApplicationChildrenKey(getKey()),
        CachedObjectChangeHandlers::APPLICATIONS_CHANGE);
}

Application *
RootImpl::getApplication(const string &appName, AccessType accessType)
{
    TRACE(CL_LOG, "getApplication");

    return dynamic_cast<Application *>(
        getOps()->getNotifyable(this,
                                ClusterlibStrings::REGISTERED_APPLICATION_NAME,
                                appName,
                                accessType));
}

NotifyableList
RootImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");
    
    throwIfRemoved();
    
    return getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_APPLICATION_NAME,
        getApplicationNames(),
        LOAD_FROM_REPOSITORY);
}

void
RootImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");
}

};	/* End of 'namespace clusterlib' */
