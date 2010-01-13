#ifndef _INCLUDED_CLUSTERLIB_JSONRPC_ADAPTOR_H_
#define _INCLUDED_CLUSTERLIB_JSONRPC_ADAPTOR_H_
#include "jsonrpc_httpd_adaptor.h"
#include <memory>
#include <clusterlib.h>
#include <log4cxx/logger.h>

namespace clusterlib { namespace rpc { namespace json {

const ::json::JSONValue::JSONString idTypeProperty = "type";
const ::json::JSONValue::JSONString idProperty = "id";
const ::json::JSONValue::JSONString idNameProperty = "name";
const ::json::JSONValue::JSONString idOptions = "options";
const ::json::JSONValue::JSONString idAddAttribute = "addAttribute";
const ::json::JSONValue::JSONString idNotifyableState = "state";
const ::json::JSONValue::JSONString idNotifyableStatus = "status";

/* Notifyable specific attributes */ 
const ::json::JSONValue::JSONString idNodesHealthy = "nodesHealthy";
const ::json::JSONValue::JSONString idNodesUnhealthy = "nodesUnhealthy";
const ::json::JSONValue::JSONString idDataDistributionCovered = "covered";
const ::json::JSONValue::JSONString idNodeState = "clientstate";
const ::json::JSONValue::JSONString idNodeStateSetTime = 
    "client state set time";
const ::json::JSONValue::JSONString idNodeConnected = "connected";
const ::json::JSONValue::JSONString idNodeConnectedId = "connectedId";
const ::json::JSONValue::JSONString idNodeConnectedTime = 
    "connected time";
const ::json::JSONValue::JSONString idNodeHealth = "health";
const ::json::JSONValue::JSONString idNodeUseProcessSlots = 
    "useProcessSlots";
const ::json::JSONValue::JSONString idProcessSlotPortVec = "portVec";
const ::json::JSONValue::JSONString idProcessSlotExecArgs = "execArgs";
const ::json::JSONValue::JSONString idProcessSlotRunningExecArgs = 
    "runningExecArgs";
const ::json::JSONValue::JSONString idProcessSlotPID = "PID";
const ::json::JSONValue::JSONString idProcessSlotDesiredProcessState = 
    "desiredProcessState";
const ::json::JSONValue::JSONString idProcessSlotCurrentProcessState = 
    "currentProcessState";
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

class MethodAdaptor : public virtual ::json::rpc::JSONRPCMethod {
  public:
    MethodAdaptor(clusterlib::Client *client);

    virtual std::string getName();
    
    virtual bool checkParams(const ::json::JSONValue::JSONArray &paramArr);

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
        clusterlib::Notifyable *notifyable);

    ::json::JSONValue::JSONObject getPropertyList(
        clusterlib::PropertyList *propertyList);

    clusterlib::Notifyable *getNotifyable(
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
        clusterlib::Notifyable *notifyable);

    ::json::JSONValue::JSONObject getOneApplicationStatus(
        clusterlib::Application *application);

    ::json::JSONValue::JSONObject getOneNodeStatus(
        clusterlib::Node *node);

    ::json::JSONValue::JSONObject getOneProcessSlotStatus(
        clusterlib::ProcessSlot *processSlot);

    ::json::JSONValue::JSONObject getOneGroupStatus(
        clusterlib::Group *group);

    ::json::JSONValue::JSONObject getOneDataDistributionStatus(
        clusterlib::DataDistribution *distribution);

    ::json::JSONValue::JSONObject getOnePropertyListStatus(
        clusterlib::PropertyList *propertyList);

    ::json::JSONValue::JSONObject getOneQueueStatus(
        clusterlib::Queue *queue);

    ::json::JSONValue::JSONObject getOneShardStatus(
        clusterlib::Shard &shard);

  private:
    static log4cxx::LoggerPtr m_logger;

    clusterlib::Client *m_client;

    clusterlib::Root *m_root;
};
}}}
#endif
