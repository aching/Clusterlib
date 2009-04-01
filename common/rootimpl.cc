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

    return getDelegate()->getApplicationNames();
}

Application *
RootImpl::getApplication(const string &appName, bool create)
{
    TRACE(CL_LOG, "getApplication");

    Application *app = getDelegate()->getApplication(appName, create);
    if ((app == NULL) && (create == true)) {
        throw ClusterException(
            string("getApplication: Couldn't create application ") + appName);
    }

    return app;
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
