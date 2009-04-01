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

NameList
GroupImpl::getNodeNames() 
{
    return getDelegate()->getNodeNames(this);
}

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

    return getDelegate()->getNode(nodeName, this, true, create);
}

NameList
GroupImpl::getGroupNames()
{
    return getDelegate()->getGroupNames(this);
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

    Group *group = getDelegate()->getGroup(groupName, this, create);
    if ((group == NULL) && (create == true)) {
        throw ClusterException(
            string("getGrup: Couldn't create group ") + groupName);
    }

    return group;
}

NameList
GroupImpl::getDataDistributionNames()
{
    return getDelegate()->getDataDistributionNames(this);
}

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

    DataDistribution *dist = 
        getDelegate()->getDataDistribution(distName, this, create);
    if ((dist == NULL) && (create == true)) {
        throw ClusterException(
            string("getDataDistribution: Couldn't create data distribution ") +
            distName);
    }

    return dist;
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
