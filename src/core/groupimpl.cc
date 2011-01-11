/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace boost;

namespace clusterlib {

NameList
GroupImpl::getNodeNames() 
{
    TRACE(CL_LOG, "getNodeNames");

    throwIfRemoved();

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createNodeChildrenKey(getKey()),
        CachedObjectChangeHandlers::NODES_CHANGE);
}

bool
GroupImpl::getNodeWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<Node> *pNodeSP) 
{
    TRACE(CL_LOG, "getNodeWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        CLString::REGISTERED_NODE_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pNodeSP = dynamic_pointer_cast<Node>(notifyableSP);
    return completed;
}

shared_ptr<Node>
GroupImpl::getNode(const string &name,
                   AccessType accessType)
{
    shared_ptr<Node> nodeSP;
    getNodeWaitMsecs(name, accessType, -1, &nodeSP);
    return nodeSP;
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

bool
GroupImpl::getGroupWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<Group> *pGroupSP) 
{
    TRACE(CL_LOG, "getGroupWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        CLString::REGISTERED_GROUP_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pGroupSP = dynamic_pointer_cast<Group>(notifyableSP);
    return completed;
}

shared_ptr<Group>
GroupImpl::getGroup(const string &name,
                         AccessType accessType)
{
    shared_ptr<Group> groupSP;
    getGroupWaitMsecs(name, accessType, -1, &groupSP);
    return groupSP;
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
 
bool
GroupImpl::getDataDistributionWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<DataDistribution> *pDataDistributionSP) 
{
    TRACE(CL_LOG, "getDataDistributionWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        CLString::REGISTERED_DATADISTRIBUTION_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pDataDistributionSP = 
        dynamic_pointer_cast<DataDistribution>(notifyableSP);
    return completed;
}

shared_ptr<DataDistribution>
GroupImpl::getDataDistribution(const string &name,
                         AccessType accessType)
{
    shared_ptr<DataDistribution> dataDistributionSP;
    getDataDistributionWaitMsecs(name, accessType, -1, &dataDistributionSP);
    return dataDistributionSP;
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
        shared_from_this(),
        CLString::REGISTERED_NODE_NAME,
        getNodeNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());
    tmpList = getOps()->getNotifyableList(
        shared_from_this(),
        CLString::REGISTERED_GROUP_NAME,
        getGroupNames(),
        LOAD_FROM_REPOSITORY);
    finalList.insert(finalList.end(), tmpList.begin(), tmpList.end());
    tmpList = getOps()->getNotifyableList(
        shared_from_this(),
        CLString::REGISTERED_DATADISTRIBUTION_NAME,
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
