/*
 * clusterclient.cc --
 *
 * Implementation of the ClusterClient class.
 *
 * =============================================================================
 * $Header:$
 * $Revision:$
 * $Date:$
 * =============================================================================
 */

#include "clusterlib.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{

/*
 * Retrieve a named application. Load it from the
 * cluster as needed.
 */
Application *
ClusterClient::getApplication(const string &appName)
    throw(ClusterException)
{
    Application *app = m_apps[appName];

    /*
     * If the application object is cached already,
     * just return it.
     */
    if (app != NULL) {
        return app;
    }

    /*
     * Otherwise load it from the cluster, cache it,
     * and return it.
     */
    app = mp_f->getApplication(appName);
    if (app != NULL) {
        m_apps[appName] = app;
        return app;
    }

    /*
     * Could not find the application object.
     */
    throw ClusterException(string("") + 
                           "Cannot find application " +
                           appName);
}

};	/* End of 'namespace clusterlib' */

 
