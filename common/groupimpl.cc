/*
 * groupimpl.cc --
 *
 * Implementation of class GroupImpl; it represents a set of nodes within
 * a specific application of clusterlib
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

/*
 * Retrieve a node object. Load it from
 * the cluster if it is not yet in the
 * cache.
 */
Node *
GroupImpl::getNode(const string &nodeName, 
                   bool create)
{
    TRACE(CL_LOG, "getNode");

    Node *np;

    /*
     * If it is already cached, return the
     * cached node object.
     */
    {
        Locker l1(getNodeMapLock());

        NodeMap::const_iterator nodeIt = m_nodes.find(nodeName);
        if (nodeIt != m_nodes.end()) {
            return nodeIt->second;
        }
    }

    /*
     * If it is not yet cached, load the
     * node from the cluster, cache it,
     * and return the object.
     */
    np = getDelegate()->getNode(nodeName, this, true, create);
    if (np != NULL) {
        Locker l2(getNodeMapLock());

        m_nodes[nodeName] = np;
        return np;
    }

    return NULL;
}

/*
 * Retrieve a group object. Load it
 * from the cluster if it is not
 * yet in the cache.
 */
Group *
GroupImpl::getGroup(const string &groupName,
                    bool create)
{
    TRACE(CL_LOG, "getGroup");

    Group *group;

    /*
     * If it is already cached, return the
     * cached group object.
     */
    {
        Locker l1(getGroupMapLock());

        GroupMap::const_iterator groupIt = m_groups.find(groupName);
        if (groupIt != m_groups.end()) {
            return groupIt->second;
        }
    }

    /*
     * If it is not yet cached, load the
     * group from the cluster, cache it,
     * and return the object.
     */
    group = getDelegate()->getGroup(groupName, this, create);
    if (group != NULL) {
        Locker l2(getGroupMapLock());

        m_groups[groupName] = group;
        return group;
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
GroupImpl::getDataDistribution(const string &distName,
                               bool create)
{
    TRACE(CL_LOG, "getDataDistribution");

    DataDistribution *distp;

    /*
     * If it is already cached, return the
     * cached distribution object.
     */
    {
        Locker l1(getDataDistributionMapLock());

        DataDistributionMap::const_iterator distIt = m_dists.find(distName);
        if (distIt != m_dists.end()) {
            return distIt->second;
        }
    }

    /*
     * If it's not yet cached, load the distribution
     * from the cluster, cache it, and return it.
     */
    distp = getDelegate()->getDataDistribution(distName, this, create);
    if (distp != NULL) {
        Locker l2(getDataDistributionMapLock());
        
        m_dists[distName] = distp;
        return distp;
    }

    /*
     * Object not found.
     */
    throw ClusterException(string("Cannot find distribution object ") +
                           distName);
}

/*
 * Initialize the cached representation of this group.
 */
void
GroupImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Nothing to do here.
     */
}

/*
 * Recache the nodes in this group.
 */
void
GroupImpl::recacheNodes()
{
    TRACE(CL_LOG, "recacheNodes");

    Locker l(getNodeMapLock());
    IdList nnames = getDelegate()->getNodeNames(this);
    IdList::iterator nnIt;

    m_nodes.clear();
    for (nnIt = nnames.begin(); nnIt != nnames.end(); nnIt++) {
        (void) getNode(*nnIt, false);
    }
}

/*
 * Refresh the cache of groups in this application.
 */
void
GroupImpl::recacheGroups()
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
 * Refresh the cache of data distributions in this group.
 */
void
GroupImpl::recacheDataDistributions()
{
    TRACE(CL_LOG, "recacheDataDistributions");

    Locker l(getDataDistributionMapLock());
    IdList dnames = 
        getDelegate()->getDataDistributionNames(this);
    IdList::iterator dnIt;

    m_dists.clear();
    for (dnIt = dnames.begin(); dnIt != dnames.end(); dnIt++) {
        (void) getDataDistribution(*dnIt, false);
    }
}

/*
 * Return the node representing the group leader,
 * if any.
 */
Node *
GroupImpl::getLeader()
{
    TRACE(CL_LOG, "getLeader");

    Locker l1(getLeadershipLock());

    if (mp_leader == NULL) {
        if (m_leaderIsKnown == false) {
            updateLeader(getDelegate()->getLeader(this));
        }
    }
    return mp_leader;
}

/*
 * Update the leader.
 */
void
GroupImpl::updateLeader(Node *lp)
{
    Locker l1(getLeadershipLock());

    mp_leader = lp;
    m_leaderIsKnown =
        (lp == NULL)
        ? false
        : true;
}

/*
 * Methods to manage strings used in leadership protocol.
 */
void
GroupImpl::initializeStringsForLeadershipProtocol()
{
    Locker l1(getLeadershipLock());

    if (!m_leadershipStringsInitialized) {
        m_leadershipStringsInitialized = true;
        m_currentLeaderNodeName =
            getDelegate()->getCurrentLeaderNodeName(getKey());
        m_leadershipBidsNodeName = 
            getDelegate()->getLeadershipBidsNodeName(getKey());
        m_leadershipBidPrefix = 
            getDelegate()->getLeadershipBidPrefix(getKey());
    }
}
string
GroupImpl::getCurrentLeaderNodeName()
{
    initializeStringsForLeadershipProtocol();
    return m_currentLeaderNodeName;
}
string
GroupImpl::getLeadershipBidsNodeName()
{
    initializeStringsForLeadershipProtocol();
    return m_leadershipBidsNodeName;
}
string
GroupImpl::getLeadershipBidPrefix()
{
    initializeStringsForLeadershipProtocol();
    return m_leadershipBidPrefix;
}

};	/* End of 'namespace clusterlib' */
