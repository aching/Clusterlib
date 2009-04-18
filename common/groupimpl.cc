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
    TRACE(CL_LOG, "getNodeNames");

    throwIfRemoved();

    return getOps()->getNodeNames(this);
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

    throwIfRemoved();

    return getOps()->getNode(nodeName, this, true, create);
}

NameList
GroupImpl::getGroupNames()
{
    TRACE(CL_LOG, "getGroupNames");

    throwIfRemoved();

    return getOps()->getGroupNames(this);
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

    throwIfRemoved();

    return getOps()->getGroup(groupName, this, create);
}

NameList
GroupImpl::getDataDistributionNames()
{
    TRACE(CL_LOG, "getDataDistributionNames");

    throwIfRemoved();

    return getOps()->getDataDistributionNames(this);
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

    throwIfRemoved();

    return getOps()->getDataDistribution(distName, this, create);
}

void
GroupImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Nothing to do here.
     */
}

void
GroupImpl::removeRepositoryEntries()
{
    getOps()->removeGroup(this);
}

/*
 * Return the node representing the group leader,
 * if any.
 */
Node *
GroupImpl::getLeader()
{
    TRACE(CL_LOG, "getLeader");

    throwIfRemoved();

    Locker l1(getLeadershipLock());

    if (mp_leader == NULL) {
        if (m_leaderIsKnown == false) {
            updateLeader(getOps()->getLeader(this));
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
    TRACE(CL_LOG, "updateLeader");

    throwIfRemoved();

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
    throwIfRemoved();

    Locker l1(getLeadershipLock());

    if (!m_leadershipStringsInitialized) {
        m_leadershipStringsInitialized = true;
        m_currentLeaderNodeName =
            getOps()->getCurrentLeaderNodeName(getKey());
        m_leadershipBidsNodeName = 
            getOps()->getLeadershipBidsNodeName(getKey());
        m_leadershipBidPrefix = 
            getOps()->getLeadershipBidPrefix(getKey());
    }
}
string
GroupImpl::getCurrentLeaderNodeName()
{
    throwIfRemoved();

    initializeStringsForLeadershipProtocol();
    return m_currentLeaderNodeName;
}
string
GroupImpl::getLeadershipBidsNodeName()
{
    throwIfRemoved();

    initializeStringsForLeadershipProtocol();
    return m_leadershipBidsNodeName;
}
string
GroupImpl::getLeadershipBidPrefix()
{
    throwIfRemoved();

    initializeStringsForLeadershipProtocol();
    return m_leadershipBidPrefix;
}

};	/* End of 'namespace clusterlib' */
