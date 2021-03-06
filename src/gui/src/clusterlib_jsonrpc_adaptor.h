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

#ifndef _INCLUDED_CLUSTERLIB_JSONRPC_ADAPTOR_H_
#define _INCLUDED_CLUSTERLIB_JSONRPC_ADAPTOR_H_

DEFINE_LOGGER(CLM_LOG, "clusterlib.rpc.json.MethodAdaptor");

namespace clusterlib { 

namespace rpc { 

namespace json {

const ::json::JSONValue::JSONString idTypeProperty = "type";
const ::json::JSONValue::JSONString idProperty = "id";
const ::json::JSONValue::JSONString idNameProperty = "name";
const ::json::JSONValue::JSONString idOptions = "options";
const ::json::JSONValue::JSONString idBidArr = "lockBidArray";
const ::json::JSONValue::JSONString idOwner = "owner";
const ::json::JSONValue::JSONString idAcquiredOwnerTime = 
    "acquiredOwnerTime";
const ::json::JSONValue::JSONString idCurrentState = "currentState";
const ::json::JSONValue::JSONString idDesiredState = "desiredState";

const ::json::JSONValue::JSONString idAddAttribute = "addAttribute";
const ::json::JSONValue::JSONString idNotifyableState = "state";
const ::json::JSONValue::JSONString idNotifyableStatus = "status";

/* Notifyable specific attributes */ 
const ::json::JSONValue::JSONString idNodesHealthy = "nodesHealthy";
const ::json::JSONValue::JSONString idNodesUnhealthy = "nodesUnhealthy";
const ::json::JSONValue::JSONString idDataDistributionHashRangeName = 
    "hashRangeName";
const ::json::JSONValue::JSONString idDataDistributionCovered = "covered";
const ::json::JSONValue::JSONString idNodeStateSetTime = 
    "client state set time";
const ::json::JSONValue::JSONString idNodeState = "client state";
const ::json::JSONValue::JSONString idNodeStateDesc = 
    "client state description";
const ::json::JSONValue::JSONString idNodeHealth = "health";
const ::json::JSONValue::JSONString idNodeUseProcessSlots = 
    "useProcessSlots";
const ::json::JSONValue::JSONString idNodeMaxProcessSlots = 
    "maxProcessSlots";
const ::json::JSONValue::JSONString idProcessSlotHostnameArr = "hostnameArr";
const ::json::JSONValue::JSONString idProcessSlotPortArr = "portArr";
const ::json::JSONValue::JSONString idProcessSlotExecArgs = "execArgs";
const ::json::JSONValue::JSONString idProcessSlotRunningExecArgs = 
    "runningExecArgs";
const ::json::JSONValue::JSONString idProcessSlotPID = "PID";
const ::json::JSONValue::JSONString idProcessSlotDesiredProcessState = 
    "desiredProcessState";
const ::json::JSONValue::JSONString idProcessSlotCurrentProcessState = 
    "currentProcessState";
const ::json::JSONValue::JSONString idProcessSlotReservationName = 
    "reservation name";
const ::json::JSONValue::JSONString idPropertyListProperty = 
    "property";
const ::json::JSONValue::JSONString idQueueCount = "count";
const ::json::JSONValue::JSONString idQueueElementPrefix = 
    "queueElementId";
const ::json::JSONValue::JSONString idShardCount = "shardCount";
const ::json::JSONValue::JSONString idShardStartRange = "shardStart";
const ::json::JSONValue::JSONString idShardEndRange = "shardEnd";

/*
 * Used in the keys passed to the browser to determine if an attribute
 * can be modified
 */
const ::json::JSONValue::JSONString idPrefixEditable = "edit";
const ::json::JSONValue::JSONString idPrefixDeletable = "delete";
const ::json::JSONValue::JSONString idPrefixDelim = ";";

const uint32_t setAttributeKeyIdx = 0;

/* Data that can be part of the the getNotifyableAttributesFromKey object */
const std::string idApplicationSummary = "applicationSummary";
const std::string idGroupSummary = "groupSummary";
const std::string idDataDistributionSummary = "dataDistributionSummary";
const std::string idNodeSummary = "nodeSummary";
const std::string idProcessSlotSummary = "processSlotSummary";
const std::string idPropertyListSummary = "propertyListSummary";
const std::string idQueueSummary = "queueSummary";
const std::string idShardSummary = "shardSummary";

/* Type that is returned from the getNotifyableAttributesFromKey function */
const std::string idTypeRoot = "Root";
const std::string idTypeApplication = "Application";
const std::string idTypeGroup = "Group";
const std::string idTypeNode = "Node";
const std::string idTypeProcessSlot = "ProcessSlot";
const std::string idTypeDataDistribution = "DataDistribution";
const std::string idTypePropertyList = "PropertyList";
const std::string idTypeQueue = "Queue";

/* Status report */
const std::string statusInactive = "Inactive";
const std::string statusBad = "Bad";
const std::string statusWarning = "Warning";
const std::string statusReady = "Ready";
const std::string statusWorking = "Working";

const std::string stateNotApplicable = "N/A";

const std::string optionRemove = "Remove";
const std::string optionAddApplication = "Add Application";
const std::string optionAddGroup = "Add Group";
const std::string optionAddDataDistribution = "Add Data Distribution";
const std::string optionAddNode = "Add Node";
const std::string optionAddProcessSlot = "Add ProcessSlot";
const std::string optionAddPropertyList = "Add PropertyList";
const std::string optionAddQueue = "Add Queue";
const std::string optionStartProcess = "Start Process";
const std::string optionStopProcess = "Stop Process";

/** Separates the options (backspace should not be used) */
const std::string optionDelim = "\b";

const std::string addProperty = "Add property";
const std::string addQueueElement = "Add queue element";

class MethodAdaptor : public virtual ::json::rpc::JSONRPCMethod
{
  public:
    MethodAdaptor(clusterlib::Client *client);

    virtual const std::string &getName() const;
    
    virtual void checkParams(const ::json::JSONValue::JSONArray &paramArr);

    ::json::JSONValue invoke(const std::string &name, 
                             const ::json::JSONValue::JSONArray &param, 
                             ::json::rpc::StatePersistence *persistence);

  private:
    ::json::JSONValue::JSONString addNotifyableFromKey(
        const ::json::JSONValue::JSONString &key,
        const ::json::JSONValue::JSONString &op,
        const ::json::JSONValue::JSONString &name);

    ::json::JSONValue::JSONString removeNotifyableFromKey(
        const ::json::JSONValue::JSONString &key,
        const ::json::JSONValue::JSONBoolean &removeChildren);

    ::json::JSONValue::JSONArray getApplications();

    ::json::JSONValue::JSONObject getNotifyableAttributesFromKey(
        const ::json::JSONValue::JSONString &name);

    ::json::JSONValue::JSONString setNotifyableAttributesFromKey(
        const ::json::JSONValue::JSONArray &obj);

    ::json::JSONValue::JSONString removeNotifyableAttributesFromKey(
        const ::json::JSONValue::JSONString &key,
        const ::json::JSONValue::JSONString &op,
        const ::json::JSONValue::JSONString &attribute);

    ::json::JSONValue::JSONObject getNotifyableChildrenFromKey(
        const ::json::JSONValue::JSONString &name);

    ::json::JSONValue::JSONObject getApplication(
        const ::json::JSONValue::JSONObject &name);

    ::json::JSONValue::JSONObject getGroup(
        const ::json::JSONValue::JSONObject &name);

    ::json::JSONValue::JSONObject getDataDistribution(
        const ::json::JSONValue::JSONObject &name);

    ::json::JSONValue::JSONObject getNode(
        const ::json::JSONValue::JSONObject &name);

    ::json::JSONValue::JSONObject getProcessSlot(
        const ::json::JSONValue::JSONObject &name);

    ::json::JSONValue::JSONObject getPropertyList(
        const ::json::JSONValue::JSONObject &name);

    ::json::JSONValue::JSONObject getNotifyableId(
        const boost::shared_ptr<clusterlib::Notifyable> &notifyableSP);

    ::json::JSONValue::JSONObject getPropertyList(
        const boost::shared_ptr<clusterlib::PropertyList> &propertyList);

    boost::shared_ptr<clusterlib::Notifyable> getNotifyable(
        const ::json::JSONValue::JSONObject &id, 
        const std::string &expectType);
    
    ::json::JSONValue::JSONArray getApplicationStatus(
        const ::json::JSONValue::JSONArray &ids);

    ::json::JSONValue::JSONArray getNodeStatus(
        const ::json::JSONValue::JSONArray &ids);

    ::json::JSONValue::JSONArray getProcessSlotStatus(
        const ::json::JSONValue::JSONArray &ids);

    ::json::JSONValue::JSONArray getGroupStatus(
        const ::json::JSONValue::JSONArray &ids);

    ::json::JSONValue::JSONArray getDataDistributionStatus(
        const ::json::JSONValue::JSONArray &ids);

    ::json::JSONValue::JSONArray getPropertyListStatus(
        const ::json::JSONValue::JSONArray &ids);

    ::json::JSONValue::JSONArray getQueueStatus(
        const ::json::JSONValue::JSONArray &ids);

    ::json::JSONValue::JSONArray getShardStatus(
        std::vector<clusterlib::Shard> &shardVec);
    
    ::json::JSONValue::JSONObject getOneNotifyableStatus(
        const boost::shared_ptr<clusterlib::Notifyable> &notifyableSP);

    ::json::JSONValue::JSONObject getOneApplicationStatus(
        const boost::shared_ptr<clusterlib::Application> &applicationSP);

    ::json::JSONValue::JSONObject getOneNodeStatus(
        const boost::shared_ptr<clusterlib::Node> &nodeSP);

    ::json::JSONValue::JSONObject getOneProcessSlotStatus(
        const boost::shared_ptr<clusterlib::ProcessSlot> &processSlotSP);

    ::json::JSONValue::JSONObject getOneGroupStatus(
        const boost::shared_ptr<clusterlib::Group> &groupSP);

    ::json::JSONValue::JSONObject getOneDataDistributionStatus(
        const boost::shared_ptr<clusterlib::DataDistribution> &distributionSP);

    ::json::JSONValue::JSONObject getOnePropertyListStatus(
        const boost::shared_ptr<clusterlib::PropertyList> &propertyListSP);

    ::json::JSONValue::JSONObject getOneQueueStatus(
        const boost::shared_ptr<clusterlib::Queue> &queueSP);

    ::json::JSONValue::JSONObject getOneShardStatus(
        clusterlib::Shard &shard);

    ::json::JSONValue::JSONArray getChildrenLockBids(
        ::json::JSONValue::JSONString notifyableKey);

  private:
    /**
     * Client context for clusterlib.
     */
    clusterlib::Client *m_client;

    /**
     * Root for clusterlib.
     */
    boost::shared_ptr<clusterlib::Root> m_rootSP;

    std::string m_name;
};

}

}

}

#endif
