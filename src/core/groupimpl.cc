/*
 * groupimpl.cc --
 *
 * Implementation of class GroupImpl; it represents a set of nodes within
 * a specific application of clusterlib
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
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

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createNodeChildrenKey(getKey()),
        CachedObjectChangeHandlers::NODES_CHANGE);
}

Node *
GroupImpl::getNode(const string &nodeName, 
                   AccessType accessType)
{
    TRACE(CL_LOG, "getNode");

    throwIfRemoved();

    return dynamic_cast<Node *>(
        getOps()->getNotifyable(this,
                                ClusterlibStrings::REGISTERED_NODE_NAME,
                                nodeName,
                                accessType));
}

NameList
GroupImpl::getGroupNames()
{
    TRACE(CL_LOG, "getGroupNames");

    throwIfRemoved();

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createGroupChildrenKey(getKey()),
        CachedObjectChangeHandlers::GROUPS_CHANGE);
}

Group *
GroupImpl::getGroup(const string &groupName,
                    AccessType accessType)
{
    TRACE(CL_LOG, "getGroup");

    throwIfRemoved();

    return dynamic_cast<Group *>(
        getOps()->getNotifyable(this,
                                ClusterlibStrings::REGISTERED_GROUP_NAME, 
                                groupName,
                                accessType));
}

NameList
GroupImpl::getDataDistributionNames()
{
    TRACE(CL_LOG, "getDataDistributionNames");

    throwIfRemoved();

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createDataDistributionChildrenKey(getKey()),
        CachedObjectChangeHandlers::DATADISTRIBUTIONS_CHANGE);
}

DataDistribution *
GroupImpl::getDataDistribution(const string &distName,
                               AccessType accessType)
{
    TRACE(CL_LOG, "getDataDistribution");

    throwIfRemoved();

    return dynamic_cast<DataDistribution *>(
        getOps()->getNotifyable(
            this,
            ClusterlibStrings::REGISTERED_DATADISTRIBUTION_NAME, 
            distName,
            accessType));
}

NotifyableList
GroupImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");
    
    throwIfRemoved();
    
    /*
     * Add the notifyables from this object and then the subclass
     * specific objects.
     */
    NotifyableList tmpList, finalList;
    tmpList = getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_NODE_NAME,
        getNodeNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());
    tmpList = getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_GROUP_NAME,
        getGroupNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());
    tmpList = getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_DATADISTRIBUTION_NAME,
        getDataDistributionNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());

    return finalList;
}

void
GroupImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Nothing to do here.
     */
}

}	/* End of 'namespace clusterlib' */
