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

void
GroupImpl::becomeLeader()
{
    TRACE(CL_LOG, "becomeLeader");

    throwIfRemoved();

    getOps()->getDistributedLocks()->acquire(this,
                                             ClusterlibStrings::LEADERLOCK);
}

void
GroupImpl::abdicateLeader()
{
    TRACE(CL_LOG, "abdicateLeader");

    throwIfRemoved();

    getOps()->getDistributedLocks()->release(this,
                                             ClusterlibStrings::LEADERLOCK);
}

bool
GroupImpl::isLeader()
{
    TRACE(CL_LOG, "isLeader");

    throwIfRemoved();

    return getOps()->getDistributedLocks()->hasLock(
        this,
        ClusterlibStrings::LEADERLOCK);
}

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

    return getOps()->getNode(nodeName, this, create);
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

};	/* End of 'namespace clusterlib' */
