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
    TRACE(CL_LOG, "getGroup");

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
    throw ClusterException(string("Cannot find group object ") +
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
    TRACE(CL_LOG, "getDistribution");

    DataDistribution *distp;

    /*
     * If it is already cached, return the
     * cached group object.
     */
    {
        Locker l1(getDistributionMapLock());

        distp = m_distributions[distName];
        if (distp != NULL) {
            return distp;
        }
    }

    /*
     * If it's not yet cached, load the distribution
     * from the cluster, cache it, and return it.
     */
    distp = getDelegate()->getDistribution(distName, this, create);
    if (distp != NULL) {
        Locker l2(getDistributionMapLock());
        
        m_distributions[distName] = distp;
        return distp;
    }

    /*
     * Object not found.
     */
    throw ClusterException(string("Cannot find distribution object ") +
                           distName);
}

/*
 * Update the cached representation of a clusterlib application.
 */
void
Application::updateCachedRepresentation()
{
    TRACE(CL_LOG, "updateCachedRepresentation");

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
    TRACE(CL_LOG, "recacheGroups");

    Locker l(getGroupMapLock());
    IdList gnames = getDelegate()->getGroupNames(this);
    IdList::iterator gnIt;

    m_groups.clear();
    for (gnIt = gnames.begin(); gnIt != gnames.end(); gnIt++) {
        (void) getGroup(*gnIt, false);
    }
}

/*
 * Refresh the cache of distributions in this
 * application.
 */
void
Application::recacheDists()
{
    TRACE(CL_LOG, "recacheDists");

    Locker l(getDistributionMapLock());
    IdList dnames = getDelegate()->getDistributionNames(this);
    IdList::iterator dnIt;

    m_distributions.clear();
    for (dnIt = dnames.begin(); dnIt != dnames.end(); dnIt++) {
        (void) getDistribution(*dnIt, false);
    }
}

};	/* End of 'namespace clusterlib' */
