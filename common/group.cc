/*
 * group.cc --
 *
 * Implementation of the Group class.
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
 * Retrieve a node object. Load it from
 * the cluster if it is not yet in the
 * cache.
 */
Node *
Group::getNode(const string &nodeName, 
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

        np = m_nodes[nodeName];
        if (np != NULL) {
            return np;
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

    throw ClusterException("Cannot find node object " +
                           nodeName);
}

/*
 * Update the cached representation of this group.
 */
void
Group::updateCachedRepresentation()
{
    TRACE(CL_LOG, "updateCachedRepresentation");

    if (cachingNodes()) {
        recacheNodes();
    }
}

/*
 * Recache the nodes in this group.
 */
void
Group::recacheNodes()
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
 * Return the node representing the group leader,
 * if any.
 */
Node *
Group::getLeader()
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
Group::updateLeader(Node *lp)
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
Group::initializeStringsForLeadershipProtocol()
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
Group::getCurrentLeaderNodeName()
{
    initializeStringsForLeadershipProtocol();
    return m_currentLeaderNodeName;
}
string
Group::getLeadershipBidsNodeName()
{
    initializeStringsForLeadershipProtocol();
    return m_leadershipBidsNodeName;
}
string
Group::getLeadershipBidPrefix()
{
    initializeStringsForLeadershipProtocol();
    return m_leadershipBidPrefix;
}

};	/* End of 'namespace clusterlib' */
