/*
 * rootmpl.cc --
 *
 * Implementation of the Root class; it represents a set of
 * applications in a clusterlib instance.
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

NameList
RootImpl::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    return getOps()->getApplicationNames();
}

Application *
RootImpl::getApplication(const string &appName, bool create)
{
    TRACE(CL_LOG, "getApplication");

    return  getOps()->getApplication(appName, create);
}

Notifyable *
RootImpl::getNotifyableFromKey(const string &key)
{
    TRACE(CL_LOG, "getNotifyableFromKey");

    return getOps()->getNotifyableFromKey(key);
}

/*
 * Initialize the cached representation of this group.
 */
void
RootImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");
}

};	/* End of 'namespace clusterlib' */
