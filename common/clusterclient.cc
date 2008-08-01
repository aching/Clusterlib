/*
 * clusterclient.cc --
 *
 * Implementation of the ClusterClient class.
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

/*
 * Retrieve a named application. Load it from the
 * cluster as needed.
 */
Application *
ClusterClient::getApplication(const string &appName)
    throw(ClusterException)
{
    Application *app = mp_f->getApplication(appName);

    /*
     * If the application object is found,
     * just return it.
     */
    if (app != NULL) {
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

 
