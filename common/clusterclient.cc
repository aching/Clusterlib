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
Client::consumeClusterEvents()
{
    TRACE( CL_LOG, "consumeClusterEvents" );

    /* TO BE WRITTEN */

#ifdef	VERY_VERBOSE_DEBUG
    cerr << "Hello from consumeClusterEvents" 
         << " this: "
         << this
         << ", thread: "
         << pthread_self()
         << endl;
#endif
}

/*
 * Register a timer handler.
 */
TimerId
Client::registerTimer(TimerEventHandler *tp,
                      uint64_t afterTime,
                      ClientData data)
    throw(ClusterException)
{
    return mp_f->registerTimer(tp, afterTime, data);
}

/*
 * Cancel a timer event. Returns true if the event was successfully
 * cancelled, false otherwise (if false is returned, its possible that
 * the timer event may still be delivered, or it has already been
 * delivered).
 */
bool
Client::cancelTimer(TimerId id)
    throw(ClusterException)
{
    return mp_f->cancelTimer(id);
}

};	/* End of 'namespace clusterlib' */

 
