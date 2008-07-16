/*
 * application.cc --
 *
 * Implementation of the Application class.
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
 * Retrieve a group object. Load it
 * from the cluster if it is not
 * yet in the cache.
 */
Group *
Application::getGroup(const string &groupName)
    throw(ClusterException)
{
    /*
     * If it is already cached, return the
     * cached group object.
     */
    Group *grp = m_groups[groupName];
    if (grp != NULL) {
        return grp;
    }

    /*
     * If it is not yet cached, load the
     * group from the cluster, cache it,
     * and return the object.
     */
    grp = getDelegate()->getGroup(groupName, this);
    if (grp != NULL) {
        m_groups[groupName] = grp;
        return grp;
    }

    /*
     * Object not found.
     */
    throw ClusterException(string("") +
                           "Cannot find group object " +
                           groupName);
};

};	/* End of 'namespace clusterlib' */

 
