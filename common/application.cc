/*
 * application.cc --
 *
 * Implementation of the Application class.
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
 * Retrieve a group object. Load it
 * from the cluster if it is not
 * yet in the cache.
 */
Group *
Application::getGroup(const string &groupName,
		      bool create)
{
    Group *grp;

    /*
     * If it is already cached, return the
     * cached group object.
     */
    {
        Locker l1(getGroupMapLock());

        grp = m_groups[groupName];
        if (grp != NULL) {
            return grp;
        }
    }

    /*
     * If it is not yet cached, load the
     * group from the cluster, cache it,
     * and return the object.
     */
    grp = getDelegate()->getGroup(groupName, this, create);
    if (grp != NULL) {
        Locker l2(getGroupMapLock());

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

/*
 * Retrieve a data distribution object. Load
 * it from the cluster if it is not yet in
 * the cache.
 */
DataDistribution *
Application::getDistribution(const string &distName,
			     bool create)
{
    DataDistribution *dist;

    /*
     * If it is already cached, return the
     * cached group object.
     */
    {
        Locker l1(getDistributionMapLock());

        dist = m_distributions[distName];
        if (dist != NULL) {
            return dist;
        }
    }

    /*
     * If it's not yet cached, load the distribution
     * from the cluster, cache it, and return it.
     */
    dist = getDelegate()->getDistribution(distName, this, create);
    if (dist != NULL) {
        Locker l2(getDistributionMapLock());
        
        m_distributions[distName] = dist;
        return dist;
    }

    /*
     * Object not found.
     */
    throw ClusterException(string ("") +
                           "Cannot find distribution object " +
                           distName);
}

/*
 * Update the cached representation of a clusterlib application.
 */
void
Application::updateCachedRepresentation()
{
    TRACE( CL_LOG, "updateCachedRepresentation" );

    if (cachingGroups()) {
        recacheGroups();
    }
    if (cachingDists()) {
        recacheDists();
    }
}

/*
 * Refresh the cache of groups in this application.
 */
void
Application::recacheGroups()
{
    TRACE( CL_LOG, "recacheGroups" );
}

/*
 * Refresh the cache of distributions in this
 * application.
 */
void
Application::recacheDists()
{
    TRACE( CL_LOG, "recacheDists" );
}

};	/* End of 'namespace clusterlib' */
