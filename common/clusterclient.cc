/*
 * clusterclient.cc --
 *
 * Implementation of the Client class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
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
Client::getApplication(const string &appName,
                       bool create)
    throw(ClusterException)
{
    TRACE( CL_LOG, "getApplication" );

    Application *app = mp_f->getApplication(appName, create);

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
                           "Cannot " +
                           (create ? "find or create " : "find ") +
                           "application " +
                           appName);
}

/*
 * The following method runs in the event handling thread,
 * invoking handlers for events as they come in.
 */
void
Client::consumeEvents()
{
    TRACE( CL_LOG, "consumeEvents" );

    /* TO BE WRITTEN */
}

};	/* End of 'namespace clusterlib' */

 
