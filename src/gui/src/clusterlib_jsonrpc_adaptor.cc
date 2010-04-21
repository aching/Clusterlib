#include <clusterlib.h>
#include "clusterlib_jsonrpc_adaptor.h"
#include <iomanip>

using namespace json;
using namespace json::rpc;
using namespace log4cxx;
using namespace std;

namespace clusterlib { namespace rpc { namespace json {

LoggerPtr MethodAdaptor::m_logger(
    Logger::getLogger("clusterlib.rpc.json.MethodAdaptor"));

MethodAdaptor::MethodAdaptor(clusterlib::Client *f) 
    : m_client(f),
      m_name("clusterlib::rpc::json::MethodAdaptor")
{
    m_root = m_client->getRoot();
}

const std::string &
MethodAdaptor::getName() const
{
    return m_name;
}

void
MethodAdaptor::checkParams(const JSONValue::JSONArray &paramArr)
{
    JSONCodec::encode(paramArr);
}

/** Wait up to 5 seconds to try and operation */
static const int32_t maxLockWaitMsecs = 5000;

JSONValue 
MethodAdaptor::invoke(const std::string &name, 
                      const JSONValue::JSONArray &param, 
                      StatePersistence *persistence) {        
    if (name == "getNotifyableAttributesFromKey") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one string parameter.");
        }
        return getNotifyableAttributesFromKey(
            param[0].get<JSONValue::JSONString>());
    } else if (name == "setNotifyableAttributesFromKey") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires at one array parameter.");
        }
        return setNotifyableAttributesFromKey(
            param[0].get<JSONValue::JSONArray>());
    } else if (name == "removeNotifyableAttributesFromKey") {
        if (param.size() != 3 ||
            param[0].type() != typeid(JSONValue::JSONString) ||
            param[1].type() != typeid(JSONValue::JSONString) ||
            param[2].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires three string parameter.");
        }
        return removeNotifyableAttributesFromKey(
            param[0].get<JSONValue::JSONString>(),
            param[1].get<JSONValue::JSONString>(),
            param[2].get<JSONValue::JSONString>());
    } else if (name == "getNotifyableChildrenFromKey") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one string parameter.");
        }
        return getNotifyableChildrenFromKey(
            param[0].get<JSONValue::JSONString>());
    } else if (name == "addNotifyableFromKey") {
        if (param.size() != 3 ||
            param[0].type() != typeid(JSONValue::JSONString) ||
            param[1].type() != typeid(JSONValue::JSONString) ||
            param[2].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires three string parameters.");
        }
        return addNotifyableFromKey(
            param[0].get<JSONValue::JSONString>(),
            param[1].get<JSONValue::JSONString>(),
            param[2].get<JSONValue::JSONString>());
    } else if (name == "removeNotifyableFromKey") {
        if (param.size() != 2 ||
            param[0].type() != typeid(JSONValue::JSONString) ||
            param[1].type() != typeid(JSONValue::JSONBoolean)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one string parameter and "
                "one boolean parameter.");
        }
        return removeNotifyableFromKey(
            param[0].get<JSONValue::JSONString>(),
            param[1].get<JSONValue::JSONBoolean>());
    } else if (name == "getApplication") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getApplication(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getGroup") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getGroup(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getDataDistribution") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getDataDistribution(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getNode") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getNode(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getPropertyList") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getPropertyList(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getApplications") {
        if (param.size() != 0) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires no parameter.");
        }
        return getApplications();
    } else if (name == "getApplicationStatus") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getApplicationStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getGroupStatus") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getGroupStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getNodeStatus") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getNodeStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getPropertyListStatus") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getPropertyListStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getDataDistributionStatus") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getDataDistributionStatus(
            param[0].get<JSONValue::JSONArray>());
    } else if (name == "getChildrenLockBids") {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one string parameter.");
        }
        return getChildrenLockBids(
            param[0].get<JSONValue::JSONString>());
    } else {
        throw JSONRPCInvocationException(
            "Unknown method '" + name + "' invoked.");
    }
}

JSONValue::JSONObject 
MethodAdaptor::getNotifyableId(
    Notifyable *notifyable) {
    string type;
    
    JSONValue::JSONObject id;
    
    if (dynamic_cast<Root*>(notifyable) != NULL) {
        type = idTypeRoot;
        id[idOptions] = 
            optionAddApplication + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_cast<Application*>(notifyable) != NULL) {
        type = idTypeApplication;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddGroup + optionDelim +
            optionAddDataDistribution + optionDelim +
            optionAddNode + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_cast<Group*>(notifyable) != NULL) {
        type = idTypeGroup;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddGroup + optionDelim +
            optionAddDataDistribution + optionDelim +
            optionAddNode + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_cast<Node*>(notifyable) != NULL) {
        type = idTypeNode;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_cast<ProcessSlot*>(notifyable) != NULL) {
        type = idTypeProcessSlot;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList + optionDelim +
            optionStartProcess + optionDelim +
            optionStopProcess;
    } else if (dynamic_cast<DataDistribution*>(notifyable) != NULL) {
        type = idTypeDataDistribution;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_cast<PropertyList *>(notifyable) != NULL) {
        type = idTypePropertyList;
        id[idOptions] =
            optionRemove;
    } else if (dynamic_cast<Queue *>(notifyable) != NULL) {
        type = idTypeQueue;
        id[idOptions] =
            optionRemove;
    } else {
        type = "unknown";
        throw JSONRPCInvocationException(
            "getNotifyableId: No such notifyable");
    }

    /* Find the locks and their bids of this notifyable */
    NameList bidList = notifyable->getLockBids(false);
    JSONValue::JSONArray bidArr;
    NameList::const_iterator bidListIt;
    for (bidListIt = bidList.begin(); 
         bidListIt != bidList.end();
         ++bidListIt) {
        bidArr.push_back(*bidListIt);
    }
    
    id[idTypeProperty] = type;
    id[idProperty] = notifyable->getKey();
    id[idNameProperty] = notifyable->getName();
    id[idBidArr] = bidArr;

    return id;
}
    
JSONValue::JSONObject 
MethodAdaptor::getPropertyList(PropertyList *propertyList) {
    JSONValue::JSONObject result;
    
    // Get all keys
    if (propertyList != NULL) {
        vector<string> keys = propertyList->getPropertyListKeys();
        for (vector<string>::const_iterator iter = keys.begin(); 
             iter != keys.end(); 
             ++iter) {
            result[*iter] = propertyList->getProperty(*iter);
        }
    }
    
    return result;
}
    
JSONValue::JSONArray MethodAdaptor::getApplications() {
    JSONValue::JSONArray appIds;
    NameList appNames = m_root->getApplicationNames();
    
    for (NameList::const_iterator iter = appNames.begin(); 
         iter != appNames.end(); 
         ++iter) {
        try {
            Application *child = m_root->getApplication(*iter);
            appIds.push_back(getNotifyableId(child));
        } catch (const ::clusterlib::Exception &ex) {
            // Ignore any application that cannot be retrieved
            LOG4CXX_WARN(m_logger, 
                         "Application " << *iter << " missing (" <<
                         ex.what() << ")");
        }
    }
    
    return appIds;
}

Notifyable *MethodAdaptor::getNotifyable(
    const JSONValue::JSONObject &id, const string &expectType) {
    JSONValue::JSONObject::const_iterator typeIter = 
        id.find(idTypeProperty);
    JSONValue::JSONObject::const_iterator idIter = 
        id.find(idProperty);
    if (typeIter == id.end()) {
        throw JSONRPCInvocationException(
            "Object type required in object ID.");
    }
    if (idIter == id.end()) {
        throw JSONRPCInvocationException(
            "Object ID required in object ID.");
    }
    if (typeIter->second.get<JSONValue::JSONString>() != expectType) {
        throw JSONRPCInvocationException(
            "Object type " + 
            typeIter->second.get<JSONValue::JSONString>() + 
            " found, expect " + expectType + ".");
    }
    
    Notifyable *notifyable = m_root->getNotifyableFromKey(
        idIter->second.get<JSONValue::JSONString>());
    if (expectType == idTypeRoot) {
        notifyable = dynamic_cast<Root*>(notifyable);
    } else if (expectType == idTypeApplication) {
        notifyable = dynamic_cast<Application*>(notifyable);
    } else if (expectType == idTypeGroup) {
        notifyable = dynamic_cast<Group*>(notifyable);
    } else if (expectType == idTypeNode) {
        notifyable = dynamic_cast<Node*>(notifyable);
    } else if (expectType == idTypeProcessSlot) {
        notifyable = dynamic_cast<ProcessSlot*>(notifyable);
    } else if (expectType == idTypeDataDistribution) {
        notifyable = dynamic_cast<DataDistribution*>(notifyable);
    }
    
    if (notifyable == NULL) {
        throw JSONRPCInvocationException(
            "The " + expectType + " requested is not found.");
    }
    
    return notifyable;
}

JSONValue::JSONObject MethodAdaptor::getNotifyableAttributesFromKey(
    const JSONValue::JSONString &key) {
    try {
        Notifyable *notifyable = NULL;
        
        if (key == "/") {
            notifyable = m_root;
        }
        else {
            notifyable = m_root->getNotifyableFromKey(key);
        }
        
        if (notifyable == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }
        
        /* Get the basic attributes again. */
        JSONValue::JSONObject attributes = getNotifyableId(notifyable);
        
        /* Get the children */
        JSONValue::JSONObject childObj = getNotifyableChildrenFromKey(key);
        
        NameList names;
        if (dynamic_cast<Root *>(notifyable)) {
            attributes["# of Applications"] = 
                (m_root->getApplicationNames()).size();
            attributes[idApplicationSummary] = 
                getApplicationStatus(childObj["applications"].
                                     get<JSONValue::JSONArray>());
            attributes[idGroupSummary] = 
                getGroupStatus(childObj["groups"].
                               get<JSONValue::JSONArray>()); 
            attributes[idDataDistributionSummary] = 
                getDataDistributionStatus(childObj["dataDistributions"].
                                          get<JSONValue::JSONArray>()); 
            attributes[idNodeSummary] = 
                getNodeStatus(childObj["nodes"].
                              get<JSONValue::JSONArray>()); 
            attributes[idPropertyListSummary] = 
                getPropertyListStatus(childObj["propertyLists"].
                                      get<JSONValue::JSONArray>()); 
            attributes[idQueueSummary] = 
                getQueueStatus(childObj["queues"].
                               get<JSONValue::JSONArray>()); 
        }
        else if (dynamic_cast<Group *>(notifyable)) {
            Group *group = 
                dynamic_cast<Group *>(notifyable);
            names = group->getNodeNames();
            int32_t healthyNodes = 0, unhealthyNodes = 0;
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                if (group->getNode(*iter)->isHealthy()) {
                    healthyNodes++;
                }
                else {
                    unhealthyNodes++;
                }
            }
            attributes[idNodesHealthy] = healthyNodes;
            attributes[idNodesUnhealthy] = unhealthyNodes;
            
            attributes[idGroupSummary] = 
                getGroupStatus(childObj["groups"].
                               get<JSONValue::JSONArray>()); 
            attributes[idDataDistributionSummary] = 
                getDataDistributionStatus(childObj["dataDistributions"].
                                          get<JSONValue::JSONArray>()); 
            attributes[idNodeSummary] = 
                getNodeStatus(childObj["nodes"].
                              get<JSONValue::JSONArray>()); 
            attributes[idPropertyListSummary] = 
                getPropertyListStatus(childObj["propertyLists"].
                                      get<JSONValue::JSONArray>()); 
            attributes[idQueueSummary] = 
                getQueueStatus(childObj["queues"].
                               get<JSONValue::JSONArray>()); 
        }
        else if (dynamic_cast<DataDistribution *>(notifyable)) {
            DataDistribution *dataDistribution = 
                dynamic_cast<DataDistribution *>(notifyable);
            attributes[idDataDistributionCovered] = 
                dataDistribution->isCovered();
            attributes[idShardCount] = dataDistribution->getShardCount();
            vector<Shard> shardVec = dataDistribution->getAllShards();
            attributes[idShardSummary] =
                getShardStatus(shardVec);
            attributes[idPropertyListSummary] = 
                getPropertyListStatus(childObj["propertyLists"].
                                      get<JSONValue::JSONArray>()); 
            attributes[idQueueSummary] = 
                getQueueStatus(childObj["queues"].
                               get<JSONValue::JSONArray>()); 
        }
        else if (dynamic_cast<Node *>(notifyable)) {
            Node *node = dynamic_cast<Node *>(notifyable);
            int64_t clientStateTime = -1;
            string clientState, clientStateDesc;
            stringstream timeSs;
            node->getClientState(&clientStateTime,
                                 &clientState, 
                                 &clientStateDesc);
            timeSs << clientStateTime << " (" 
                   << TimerService::getMsecsTimeString(clientStateTime) 
                   << ")";
            attributes[idNodeStateSetTime] = timeSs.str();
            attributes[idNodeState] = clientState;
            attributes[idNodeStateDesc] = clientStateDesc;
            string connectedId;
            int64_t connectionTime = -1;
            bool connected = node->isConnected(&connectedId, &connectionTime);
            attributes[idNodeConnected] = connected ? "true" : "false";
            if (connected) {
                timeSs.str("");
                timeSs << connectionTime << " (" 
                       << TimerService::getMsecsTimeString(connectionTime) 
                       << ")";
                attributes[idNodeConnectedId] = connectedId;
                attributes[idNodeConnectedTime] = timeSs.str();
            }
            attributes[idNodeHealth] = 
                node->isHealthy() ? "healthy" : "unhealthy";
            attributes[idNodeUseProcessSlots] = 
                node->getUseProcessSlots() ? "true" : "false";
            
            attributes[idProcessSlotSummary] = 
                getProcessSlotStatus(childObj["processSlots"].
                                     get<JSONValue::JSONArray>()); 
            attributes[idPropertyListSummary] = 
                getPropertyListStatus(childObj["propertyLists"].
                                      get<JSONValue::JSONArray>()); 
            attributes[idQueueSummary] = 
                getQueueStatus(childObj["queues"].
                               get<JSONValue::JSONArray>()); 
            
        }
        else if (dynamic_cast<ProcessSlot *>(notifyable)) {
            ProcessSlot *processSlot = 
                dynamic_cast<ProcessSlot *>(notifyable);
            
            attributes[idPrefixEditable + idPrefixDelim +
                       idProcessSlotPortVec] = 
                JSONCodec::encode(processSlot->getJsonPortVec());
            attributes[idPrefixEditable + idPrefixDelim +
                       idProcessSlotExecArgs] = 
                processSlot->getJsonExecArgs();
            attributes[idProcessSlotRunningExecArgs] = 
                processSlot->getJsonRunningExecArgs();
            attributes[idProcessSlotPID] = 
                processSlot->getJsonPID();
            attributes[idProcessSlotDesiredProcessState] =
                processSlot->getJsonDesiredProcessState(); 
            attributes[idProcessSlotCurrentProcessState] = 
                processSlot->getJsonCurrentProcessState();
            attributes[idProcessSlotReservationName] = 
                processSlot->getReservationName();

            attributes[idPropertyListSummary] = 
                getPropertyListStatus(childObj["propertyLists"].
                                      get<JSONValue::JSONArray>()); 
            attributes[idQueueSummary] = 
                getQueueStatus(childObj["queues"].
                               get<JSONValue::JSONArray>()); 
        }
        else if (dynamic_cast<PropertyList *>(notifyable)) {
            PropertyList *propertyList = 
                dynamic_cast<PropertyList *>(notifyable);
            attributes[idAddAttribute] = addProperty;
            vector<string> keyVec = propertyList->getPropertyListKeys();
            for (vector<string>::iterator it = keyVec.begin();
                 it != keyVec.end();
                 it++) {
                /* 
                 * Properties are editable to anything and
                 * deletable. 
                 */
                attributes[idPrefixEditable + idPrefixDelim + 
                           idPrefixDeletable + idPrefixDelim +
                           idPropertyListProperty + " " + *it] 
                    = propertyList->getProperty(*it);
            }
        }
        else if (dynamic_cast<Queue *>(notifyable)) {
            attributes[idAddAttribute] = addQueueElement;
            Queue *queue = 
                dynamic_cast<Queue *>(notifyable);
            map<int64_t, string> idElementMap = 
                queue->getAllElements();
            stringstream ss;
            map<int64_t, string>::const_iterator idElementMapIt;
            for (idElementMapIt = idElementMap.begin();
                 idElementMapIt != idElementMap.end();
                 idElementMapIt++) {
                ss.str("");
                ss << idQueueElementPrefix << " " << setw(3) 
                   << idElementMapIt->first;
                    attributes[idPrefixDeletable + idPrefixDelim +
                               ss.str()] = idElementMapIt->second;
            }
            attributes[idQueueCount] = queue->size();
        }
        
        return attributes;
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                         "Getting notifyable failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString MethodAdaptor::setNotifyableAttributesFromKey(
    const JSONValue::JSONArray &arr) {
    ostringstream oss;
    try {
        if (arr.size() < 1) {
            throw JSONRPCInvocationException(
                "Arr is less than size of 1 ");
        }
        
        string key = arr[setAttributeKeyIdx].get<JSONValue::JSONString>();
        Notifyable *notifyable = 
            m_root->getNotifyableFromKey(key);
        if (notifyable == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + 
                key);
        }
        
        if (dynamic_cast<Root *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for root key " +
                key);
        }
        else if (dynamic_cast<Application *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for group key " + 
                key);
        }
        else if (dynamic_cast<Group *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for root key " + 
                key);
        }
        else if (dynamic_cast<DataDistribution *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for data distribution key " + 
                key);
        }
        else if (dynamic_cast<Node *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for node key " + 
                key);
        }
        else if (dynamic_cast<ProcessSlot *>(notifyable)) {
            if (arr.size() != 4) {
                throw JSONRPCInvocationException(
                    "Need 4 elements in array");
            }

            ProcessSlot *processSlot = 
                dynamic_cast<ProcessSlot *>(notifyable);                
            string op = 
                arr[setAttributeKeyIdx + 2].get<JSONValue::JSONString>();
                
            if (!op.compare(0,
                            idProcessSlotPortVec.size(), 
                            idProcessSlotPortVec)) {
                processSlot->setJsonPortVec(arr[setAttributeKeyIdx + 3].
                                            get<JSONValue::JSONArray>());
            }
            else if (!op.compare(
                         0,
                         idProcessSlotExecArgs.size(), 
                         idProcessSlotExecArgs)) {
                processSlot->setJsonExecArgs(arr[setAttributeKeyIdx + 3].
                                             get<JSONValue::JSONObject>());
            }
            else if (!op.compare(
                         0,
                         optionStartProcess.size(),
                         optionStartProcess)) {
                processSlot->start();
            }
            else if (!op.compare(
                         0,
                         optionStopProcess.size(),
                         optionStopProcess)) {
                processSlot->stop();
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for process slot key " + 
                    key);
            }
        }
        else if (dynamic_cast<PropertyList *>(notifyable)) {
            if (arr.size() != 4) {
                throw JSONRPCInvocationException(
                    "Need 4 elements in array");
            }

            PropertyList *propertyList = 
                dynamic_cast<PropertyList *>(notifyable);                
            string op = 
                arr[setAttributeKeyIdx + 1].get<JSONValue::JSONString>();
            string property = 
                arr[setAttributeKeyIdx + 2].get<JSONValue::JSONString>();
            string value = 
                JSONCodec::encode(arr[setAttributeKeyIdx + 3]);
                
            if (!op.compare(0, 
                            idPropertyListProperty.size(), 
                            idPropertyListProperty) ||
                !op.compare(addProperty)) {
                bool gotLock = propertyList->acquireLockWaitMsecs(
                    maxLockWaitMsecs);
                if (!gotLock) {
                    oss.str("");
                    oss << "Failed to set property " << property 
                        << " with value " << value << " within "
                        << maxLockWaitMsecs << " msecs";
                    throw JSONRPCInvocationException(oss.str());
                }
                propertyList->setProperty(property, value);
                propertyList->publishProperties();
                propertyList->releaseLock();
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for property list key " + 
                    key);
            }
        }
        else if (dynamic_cast<Queue *>(notifyable)) {
            if (arr.size() != 4) {
                throw JSONRPCInvocationException(
                    "Needs 4 elements in array");
            }

            string op = 
                arr[setAttributeKeyIdx + 1].get<JSONValue::JSONString>();
            Queue *queue = dynamic_cast<Queue *>(notifyable);
            if (!op.compare("Add queue element")) {
                queue->put(arr[setAttributeKeyIdx + 3].
                           get<JSONValue::JSONString>());
            }
        }
        return string("0");
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                     "Getting notifyable failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString MethodAdaptor::removeNotifyableAttributesFromKey(
    const JSONValue::JSONString &key,
    const JSONValue::JSONString &op,
    const JSONValue::JSONString &attribute) {
    ostringstream oss;
    try {
        Notifyable *notifyable = m_root->getNotifyableFromKey(key);
        if (notifyable == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }

        if (dynamic_cast<Root *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for root key " + key);
        }
        else if (dynamic_cast<Application *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for group key " + key);
        }
        else if (dynamic_cast<Group *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for root key " + key);
        }
        else if (dynamic_cast<DataDistribution *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for data distribution key " + key);
        }  
        else if (dynamic_cast<Node *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for node key " + key);
        }
        else if (dynamic_cast<ProcessSlot *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for process slot key " + key);
        }
        else if (dynamic_cast<PropertyList *>(notifyable)) {
            PropertyList *propertyList = 
                dynamic_cast<PropertyList *>(notifyable);
            if (!op.compare(idPropertyListProperty)) {
                bool gotLock = propertyList->acquireLockWaitMsecs(
                    maxLockWaitMsecs);
                if (!gotLock) {
                    oss.str("");
                    oss << "Failed to delete property " << attribute
                        << " within " << maxLockWaitMsecs << " msecs";
                    throw JSONRPCInvocationException(oss.str());
                }
                propertyList->deleteProperty(attribute);
                propertyList->publishProperties();
                propertyList->releaseLock();
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for property list key " + key);
            }
        }
        else if (dynamic_cast<Queue *>(notifyable)) {
            Queue *queue = dynamic_cast<Queue *>(notifyable);
            if (!op.compare(idQueueElementPrefix)) {
                stringstream ss;
                ss << attribute;
                int64_t id;
                ss >> id;
                queue->removeElement(id);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for queue key " + key);
            }
        }
        return string("0");
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                     "Getting notifyable failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject MethodAdaptor::getNotifyableChildrenFromKey(
    const JSONValue::JSONString &name) {
    try {
        Notifyable *notifyable = NULL;
        JSONValue::JSONObject children;
        JSONValue::JSONArray root, applications, groups, 
            dataDistributions, nodes, processSlots, propertyLists, queues;

        /* Special case of getting the root */
        if (name == "/") {
            root.push_back(getNotifyableId(m_root));
            children["root"] = root;
            return children;
        }
        else {
            notifyable = m_root->getNotifyableFromKey(name);
        }

        if (notifyable == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key." + name);
        }

        NameList names;
        if (dynamic_cast<Root *>(notifyable)) {
            names = m_root->getApplicationNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                Application *child = 
                    m_root->getApplication(*iter);
                if (child) {
                    applications.push_back(getNotifyableId(child));
                }
            }
        }
        else if (dynamic_cast<Application *>(notifyable)) {
            Application *application = 
                (dynamic_cast<Application *>(notifyable));
            names = application->getGroupNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                Group *child = application->getGroup(*iter);
                if (child) {
                    groups.push_back(getNotifyableId(child));
                }
            }
            names = application->getDataDistributionNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                DataDistribution *child = 
                    application->getDataDistribution(*iter);
                if (child) {
                    dataDistributions.push_back(getNotifyableId(child));
                }
            }
            names = application->getNodeNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                Node *child = application->getNode(*iter);
                if (child) {
                    nodes.push_back(getNotifyableId(child));
                }
            }
        }
        else if (dynamic_cast<Group *>(notifyable)) {
            Group *group = (dynamic_cast<Group *>(notifyable));
            names = group->getGroupNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                Group *child = group->getGroup(*iter);
                if (child) {
                    groups.push_back(getNotifyableId(child));
                }
            }
            names = group->getDataDistributionNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                DataDistribution *child = 
                    group->getDataDistribution(*iter);
                if (child) {
                    dataDistributions.push_back(getNotifyableId(child));
                }
            }
            names = group->getNodeNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                Node *child = group->getNode(*iter);
                if (child) {
                    nodes.push_back(getNotifyableId(child));
                }
            }
        }
        else if (dynamic_cast<Node *>(notifyable)) {
            Node *node = (dynamic_cast<Node *>(notifyable));
            names = node->getProcessSlotNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                ProcessSlot *child = node->getProcessSlot(*iter);
                if (child) {
                    processSlots.push_back(getNotifyableId(child));
                }
            }
        }

        /* If not a PropertyList, object can search for PropertyLists */
        if (!dynamic_cast<PropertyList *>(notifyable)) {
            names = notifyable->getPropertyListNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                PropertyList *child = notifyable->getPropertyList(*iter);
                if (child) {
                    propertyLists.push_back(getNotifyableId(child));
                }
            }
        }

        /* 
         * If not a PropertyList or a Queue, object can search for
         * Queues
         */
        if ((!dynamic_cast<PropertyList *>(notifyable) &&
             !dynamic_cast<Queue *>(notifyable))) {
            names = notifyable->getQueueNames();
            for (NameList::const_iterator iter = names.begin(); 
                 iter != names.end(); 
                 ++iter) {
                Queue *child = notifyable->getQueue(*iter);
                if (child) {
                    queues.push_back(getNotifyableId(child));
                }
            }
        }

        children["applications"] = applications;
        children["groups"] = groups;
        children["dataDistributions"] = dataDistributions;
        children["nodes"] = nodes;
        children["processSlots"] = processSlots;
        children["propertyLists"] = propertyLists;
        children["queues"] = queues;

        return children;
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                     "Getting notifyable failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString MethodAdaptor::addNotifyableFromKey(
    const JSONValue::JSONString &key,
    const JSONValue::JSONString &op,
    const JSONValue::JSONString &name) {
    try {
        Notifyable *notifyable = m_root->getNotifyableFromKey(key);
        if (notifyable == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }

        if (dynamic_cast<Root *>(notifyable)) {
            if (!op.compare(optionAddApplication)) {
                notifyable = m_root->getApplication(name, true);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyable = m_root->getPropertyList(name, true);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for root key " + key);
            }
        }
        else if (dynamic_cast<Application *>(notifyable)) {
            Application *application = 
                dynamic_cast<Application *>(notifyable);
            if (!op.compare(optionAddGroup)) {
                notifyable = application->getGroup(name, true);
            }
            else if (!op.compare(optionAddDataDistribution)) {
                notifyable = application->getDataDistribution(name, true);
            }
            else if (!op.compare(optionAddNode)) {
                notifyable = application->getNode(name, true);
            }
            else if (!op.compare(optionAddQueue)) {
                notifyable = application->getQueue(name, true);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyable =  application->getPropertyList(name, true);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for group key " + key);
            }
        }
        else if (dynamic_cast<Group *>(notifyable)) {
            Group *group = dynamic_cast<Group *>(notifyable);
            if (!op.compare(optionAddGroup)) {
                notifyable = group->getGroup(name, true);
            }
            else if (!op.compare(optionAddDataDistribution)) {
                notifyable = group->getDataDistribution(name, true);
            }
            else if (!op.compare(optionAddNode)) {
                notifyable = group->getNode(name, true);
            }
            else if (!op.compare(optionAddQueue)) {
                notifyable = group->getQueue(name, true);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyable = group->getPropertyList(name, true);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for root key " + key);
            }
        }
        else if (dynamic_cast<DataDistribution *>(notifyable)) {
            DataDistribution *dataDistributions = 
                dynamic_cast<DataDistribution *>(notifyable);
            if (!op.compare(optionAddQueue)) {
                notifyable = dataDistributions->getQueue(name, true);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyable = dataDistributions->getPropertyList(name, true);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for data distribution key " + key);
            }
        }
        else if (dynamic_cast<Node *>(notifyable)) {
            Node *node = dynamic_cast<Node *>(notifyable);
            if (!op.compare(optionAddQueue)) {
                notifyable = node->getQueue(name, true);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyable = node->getPropertyList(name, true);
            }                
            else {
                throw JSONRPCInvocationException(
                    "No such op for node key " + key);
            }
        }
        else if (dynamic_cast<ProcessSlot *>(notifyable)) {
            ProcessSlot *processSlot = 
                dynamic_cast<ProcessSlot *>(notifyable);
            if (!op.compare(optionAddQueue)) {
                notifyable = processSlot->getQueue(name, true);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyable = processSlot->getPropertyList(name, true);
            }                
            else {
                throw JSONRPCInvocationException(
                    "No such op for node key " + key);
            }
        }
        else if (dynamic_cast<PropertyList *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for property list key " + key);
        }
        else if (dynamic_cast<Queue *>(notifyable)) {
            throw JSONRPCInvocationException(
                "No such op for queue key " + key);
        }
        else if (notifyable == NULL) {
            throw JSONRPCInvocationException(
                "Creating the notifyable failed or removed right "
                "away for key " + key);
        }
        else {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }
        return notifyable->getKey();
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                     "addNotifyableFromKey: Getting notifyable failed (" 
                     << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString MethodAdaptor::removeNotifyableFromKey(
    const JSONValue::JSONString &key,
    const JSONValue::JSONBoolean &removeChildren) {
    try {
        Notifyable *notifyable = m_root->getNotifyableFromKey(key);
        if (notifyable == NULL) {
            throw JSONRPCInvocationException(
                "removeNotifyableFromKey: Cannot get Notifyable from key "
                + key);
        }
        notifyable->remove(removeChildren);
        return string("0");
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger,
                     "removeNotifyableFromKey: " << ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}
    
JSONValue::JSONObject MethodAdaptor::getApplication(
    const JSONValue::JSONObject &name) {
    try {
        Application *application = 
            dynamic_cast<Application*>(getNotifyable(name, 
                                                     idTypeApplication));
        NameList groupNames = application->getGroupNames();
        NameList distributionNames = 
            application->getDataDistributionNames();
        NameList nodeNames = application->getNodeNames();
        NameList propertyListNames = application->getPropertyListNames();

        JSONValue::JSONArray groups, distributions, nodes, propertyLists;

        // Transform groups into group IDs
        for (NameList::const_iterator iter = groupNames.begin(); 
             iter != groupNames.end(); 
             ++iter) {
            Group *child = application->getGroup(*iter);
            groups.push_back(getNotifyableId(child));
        }

        // Transform distributions into distribution IDs
        for (NameList::const_iterator iter = distributionNames.begin();
             iter != distributionNames.end(); ++iter) {
            DataDistribution *child = 
                application->getDataDistribution(*iter);
            distributions.push_back(getNotifyableId(child));
        }

        // Transform nodes into node IDs
        for (NameList::const_iterator iter = nodeNames.begin(); 
             iter != nodeNames.end(); 
             ++iter) {
            Node *child = application->getNode(*iter);
            nodes.push_back(getNotifyableId(child));
        }

        // Transform propertyLists into propertyList IDs
        for (NameList::const_iterator iter = propertyListNames.begin(); 
             iter != propertyListNames.end(); 
             ++iter) {
            PropertyList *child = application->getPropertyList(*iter);
            propertyLists.push_back(getNotifyableId(child));
        }

        JSONValue::JSONObject result;
        result["groups"] = groups;
        result["dataDistributions"] = distributions;
        result["nodes"] = nodes;
        result["properties"] = getPropertyList(
            application->getPropertyList());
        result["propertyLists"] = propertyLists;
        result["status"] = getOneApplicationStatus(application);

        return result;
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                     "Getting application failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject MethodAdaptor::getGroup(
    const JSONValue::JSONObject &name) {
    try {
        Group *group = 
            dynamic_cast<Group*>(getNotifyable(name, idTypeGroup));
        NameList groupNames = group->getGroupNames();
        NameList distributionNames = group->getDataDistributionNames();
        NameList nodeNames = group->getNodeNames();
        NameList propertyListNames = group->getPropertyListNames();
            
        JSONValue::JSONArray groups, distributions, nodes, propertyLists;

        // Transform groups into group IDs
        for (NameList::const_iterator iter = groupNames.begin(); 
             iter != groupNames.end(); 
             ++iter) {
            Group *child = group->getGroup(*iter);
            groups.push_back(getNotifyableId(child));
        }

        // Transform distributions into distribution IDs
        for (NameList::const_iterator iter = distributionNames.begin();
             iter != distributionNames.end(); 
             ++iter) {
            DataDistribution *child = group->getDataDistribution(*iter);
            distributions.push_back(getNotifyableId(child));
        }

        // Transform nodes into node IDs
        for (NameList::const_iterator iter = nodeNames.begin(); 
             iter != nodeNames.end(); 
             ++iter) {
            Node *child = group->getNode(*iter);
            nodes.push_back(getNotifyableId(child));
        }

        // Transform propertyLists into propertyList IDs
        for (NameList::const_iterator iter = propertyListNames.begin(); 
             iter != propertyListNames.end(); 
             ++iter) {
            PropertyList *child = group->getPropertyList(*iter);
            propertyLists.push_back(getNotifyableId(child));
        }

        JSONValue::JSONObject result;
        result["parent"] = getNotifyableId(group->getMyParent());
        result["groups"] = groups;
        result["dataDistributions"] = distributions;
        result["nodes"] = nodes;
        result["properties"] = getPropertyList(group->getPropertyList());
        result["propertyListObjects"] = propertyLists;
        //result["isLeader"] = group->isLeader();
        result["status"] = getOneGroupStatus(group);

        return result;
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, "Getting group failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject MethodAdaptor::getDataDistribution(
    const JSONValue::JSONObject &name) {
    try {
        DataDistribution *distribution = 
            dynamic_cast<DataDistribution*>(
                getNotifyable(name, idTypeDataDistribution));

        JSONValue::JSONArray shards;
            
        vector<Shard> shardVec = distribution->getAllShards();
            
        // Transform shards
        for (uint32_t i = 0; i < shardVec.size(); ++i) {
            JSONValue::JSONObject shard;
            ostringstream oss;
            oss << shardVec[i].getStartRange();
            shard["low"] = oss.str();
            oss.str("");
            oss << shardVec[i].getEndRange();
            shard["high"] = oss.str();
            oss.str("");
            oss << shardVec[i].getPriority();
            shard["priority"] = oss.str();
            if (shardVec[i].getNotifyable() != NULL) {
                shard["id"] = getNotifyableId(
                    m_root->getNotifyableFromKey(
                        shardVec[i].getNotifyable()->getKey()));
            } else {
                shard["id"] = JSONValue::Null;
            }
            shards.push_back(shard);
        }

        JSONValue::JSONObject result;
        bool covered = distribution->isCovered();
        result["parent"] = getNotifyableId(distribution->getMyParent());
        result["isCovered"] = covered;
        result["shards"] = shards;
        result["properties"] = 
            getPropertyList(distribution->getPropertyList());
        result["status"] = getOneDataDistributionStatus(distribution);

        return result;
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                     "Getting data distribution failed (" << 
                     ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}
    
JSONValue::JSONObject MethodAdaptor::getNode(
    const JSONValue::JSONObject &name) {
    try {
        Node *node = dynamic_cast<Node*>(getNotifyable(name, idTypeNode));
        int64_t clientStateTime = -1;
        string clientState, clientStateDesc;
        node->getClientState(&clientStateTime,
                             &clientState, 
                             &clientStateDesc);
        string connectedId;
        int64_t connectionTime = -1;
        bool connected = node->isConnected(&connectedId, &connectionTime);
        JSONValue::JSONObject result;
        result["isHealthy"] = node->isHealthy();
        result["isConnected"] = connected;
        result["lastStateTime"] = clientStateTime;
        result["lastMasterStateTime"] = node->getMasterSetStateTime();
        result["lastConnectionTime"] = connectionTime;
        result["masterState"] = node->getMasterSetState();
        result["state"] = clientState;
        result["parent"] = getNotifyableId(node->getMyParent());
        result["status"] = getOneNodeStatus(node);
        result["properties"] = getPropertyList(node->getPropertyList());

        return result;
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, "Getting node failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject MethodAdaptor::getPropertyList(
    const JSONValue::JSONObject &name) {
    try {
        PropertyList *propertyList = 
            dynamic_cast<PropertyList*>(getNotifyable(name, 
                                                      idTypePropertyList));
            
        JSONValue::JSONObject result;
        result["parent"] = getNotifyableId(propertyList->getMyParent());
        result["status"] = getOnePropertyListStatus(propertyList);
        result["properties"] = getPropertyList(propertyList);

        return result;
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(m_logger, 
                     "Getting propertyList failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject MethodAdaptor::getOneNotifyableStatus(
    Notifyable *notifyable) {
    try {
        if (dynamic_cast<Application *>(notifyable)) {
            return getOneApplicationStatus(
                dynamic_cast<Application *>(notifyable));
        }
        else if (dynamic_cast<Group *>(notifyable)) {
            return getOneGroupStatus(
                dynamic_cast<Group *>(notifyable));
        }
        else if (dynamic_cast<DataDistribution *>(notifyable)) {
            return getOneDataDistributionStatus(
                dynamic_cast<DataDistribution *>(notifyable));
        }
        else if (dynamic_cast<Node *>(notifyable)) {
            return getOneNodeStatus(
                dynamic_cast<Node *>(notifyable));
        }
        else if (dynamic_cast<ProcessSlot *>(notifyable)) {
            return getOneProcessSlotStatus(
                dynamic_cast<ProcessSlot *>(notifyable));
        }
        else if (dynamic_cast<PropertyList *>(notifyable)) {
            return getOnePropertyListStatus(
                dynamic_cast<PropertyList *>(notifyable));
        }
        else if (dynamic_cast<Queue *>(notifyable)) {
            return getOneQueueStatus(
                dynamic_cast<Queue *>(notifyable));
        }
        else {
            throw JSONRPCInvocationException("Invalid notifyable type");
        }
    } catch (const ::clusterlib::Exception &ex) {
        LOG4CXX_WARN(
            m_logger,
            "getOneNotifyableStatus: Failed (" << ex.what() << ")");
        throw JSONRPCInvocationException(ex.what());
    }        
}

JSONValue::JSONObject MethodAdaptor::getOneApplicationStatus(
    Application *application) {
    // In clusterlib, application is a special group
    return getOneGroupStatus(application);
}

JSONValue::JSONObject MethodAdaptor::getOneNodeStatus(Node *node) {
    // Should change this to other condition
    bool connected = node->isConnected();
    bool healthy = node->isHealthy();

    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = node->getKey();
    jsonObj[idNameProperty] = node->getName();
    jsonObj[idNotifyableStatus] = statusReady;

    if (connected && healthy) {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Connected and healthy";
    } 
    else if (connected || healthy) {
        jsonObj[idNotifyableStatus] = statusWarning;
        jsonObj[idNotifyableState] = "Either not connected or not healthy";
    }
    else {
        jsonObj[idNotifyableStatus] = statusBad;
        jsonObj[idNotifyableState] = "Not connected and not healthy";
    }

    return jsonObj;
}
    
JSONValue::JSONObject MethodAdaptor::getOneProcessSlotStatus(
    ProcessSlot *processSlot) {
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = processSlot->getKey();
    jsonObj[idNameProperty] = processSlot->getName();
    jsonObj[idNotifyableStatus] = statusReady;

    ProcessSlot::ProcessState currentState;
    processSlot->getCurrentProcessState(&currentState, NULL);
        
    if (currentState == ProcessSlot::UNUSED) {
        jsonObj[idNotifyableStatus] = statusInactive;
        jsonObj[idNotifyableState] = "Not being used";
    }
    else if (currentState == ProcessSlot::STARTED) {
        jsonObj[idNotifyableStatus] = statusWarning;
        jsonObj[idNotifyableState] = "Started a process";
    }
    else if (currentState == ProcessSlot::RUNNING) {
        jsonObj[idNotifyableStatus] = statusWorking;
        jsonObj[idNotifyableState] = "Running a process";
    } 
    else if (currentState == ProcessSlot::FINISHED) {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Finished a process";
    }
    else {
        jsonObj[idNotifyableStatus] = statusBad;
        jsonObj[idNotifyableState] = "Unknown problem";
    }

    return jsonObj;
}

JSONValue::JSONObject MethodAdaptor::getOneGroupStatus(
    Group *group) {
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = group->getKey();
    jsonObj[idNameProperty] = group->getName();
    jsonObj[idNotifyableStatus] = statusReady;
    jsonObj[idNotifyableState] = "No problems";
    string notifyableStatus;

    // Check for all groups
    NameList names = group->getGroupNames();
    for (NameList::const_iterator iter = names.begin(); 
         iter != names.end(); 
         ++iter) {
        Group *childGroup = group->getGroup(*iter);
        if (childGroup == NULL) {
            continue;
        }
        notifyableStatus = 
            getOneGroupStatus(childGroup)[idNotifyableStatus].get<JSONValue::JSONString>();
        if (notifyableStatus == statusBad) {
            // If we have a bad status node, the group definitely is bad
            jsonObj[idNotifyableStatus] = statusBad;
            jsonObj[idNotifyableState] = 
                "At least one group is bad";
        } else if (notifyableStatus == statusWarning && 
                   jsonObj[idNotifyableStatus].
                   get<JSONValue::JSONString>() == statusReady) {
            // If we have a warn status node, the group is warn
            // unless there is a bad
            jsonObj[idNotifyableStatus] = statusWarning;
            jsonObj[idNotifyableState] = 
                "At least one group is in warning";
        }
    }

    // Check for all data distribution
    names = group->getDataDistributionNames();
    for (NameList::const_iterator iter = names.begin(); 
         iter != names.end(); 
         ++iter) {
        DataDistribution *distribution = group->getDataDistribution(*iter);
        if (distribution == NULL) {
            continue;
        }
        notifyableStatus = 
            getOneDataDistributionStatus(distribution)[idNotifyableStatus].get<JSONValue::JSONString>();
        if (notifyableStatus == statusBad) {
            // If we have a bad status node, the group definitely is bad
            jsonObj[idNotifyableStatus] = statusBad;
            jsonObj[idNotifyableState] = 
                "At least one data distribution is bad";
        } else if (notifyableStatus == statusWarning && 
                   jsonObj[idNotifyableStatus].
                   get<JSONValue::JSONString>() == statusReady) {
            // If we have a warn status node, the group is warn
            // unless there is a bad
            jsonObj[idNotifyableStatus] = statusWarning;
            jsonObj[idNotifyableState] = 
                "At least one data distribution is in warning";
        }
    }

    // Check for all nodes
    names = group->getNodeNames();
    for (NameList::const_iterator iter = names.begin(); 
         iter != names.end(); 
         ++iter) {
        Node *node = group->getNode(*iter);
        if (node == NULL) {
            continue;
        }
        notifyableStatus = 
            getOneNodeStatus(node)[idNotifyableStatus].get<JSONValue::JSONString>();
        if (notifyableStatus == statusBad) {
            // If we have a bad status node, the group definitely is bad
            jsonObj[idNotifyableStatus] = statusBad;
            jsonObj[idNotifyableState] = 
                "At least one node is bad";
        } else if (notifyableStatus == statusWarning && 
                   jsonObj[idNotifyableStatus].
                   get<JSONValue::JSONString>()  == statusReady) {
            // If we have a warn status node, the group is warn
            // unless there is a bad
            jsonObj[idNotifyableStatus] = statusWarning;
            jsonObj[idNotifyableState] = 
                "At least one node is in warning";
        }
    }

    return jsonObj;
}

JSONValue::JSONObject MethodAdaptor::getOneDataDistributionStatus(
    DataDistribution *distribution) {
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = distribution->getKey();
    jsonObj[idNameProperty] = distribution->getName();
    if (distribution->getShardCount() <= 0) {
        jsonObj[idNotifyableStatus] = statusInactive;
    }
    else if (distribution->isCovered()) {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Covered and ready";
    }
    else {
        jsonObj[idNotifyableStatus] = statusBad;
        jsonObj[idNotifyableState] = "Not covered";
    }
        
    return jsonObj;
}

JSONValue::JSONObject MethodAdaptor::getOnePropertyListStatus(
    PropertyList *propertyList) {
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = propertyList->getKey();
    jsonObj[idNameProperty] = propertyList->getName();
    if (propertyList->getPropertyListKeys().size() == 0) {
        jsonObj[idNotifyableStatus] = statusInactive;
        jsonObj[idNotifyableState] = "No keys used";
    }
    else {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Keys are being used";
    }
    return jsonObj;
}

JSONValue::JSONObject MethodAdaptor::getOneQueueStatus(
    Queue *queue) {
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = queue->getKey();
    jsonObj[idNameProperty] = queue->getName();
    if (queue->size() == 0) {
        jsonObj[idNotifyableStatus] = statusInactive;
        jsonObj[idNotifyableState] = "Empty queue";
    }
    else {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Queue is being used";
    }
    return jsonObj;
}

JSONValue::JSONObject MethodAdaptor::getOneShardStatus(
    Shard &shard) {
    JSONValue::JSONObject jsonObj;
    stringstream ss;
    ss << "start=" << shard.getStartRange() << ", end="
       << shard.getEndRange();
    Notifyable *notifyable = shard.getNotifyable();
    if (notifyable == NULL) {
        jsonObj[idProperty] = "N/A";
        jsonObj[idNameProperty] = "N/A";
        jsonObj[idNotifyableStatus] = "N/A";
        jsonObj[idNotifyableState] = "N/A";
    }
    else {
        jsonObj[idProperty] = notifyable->getKey();
        jsonObj[idNameProperty] = notifyable->getName();
        JSONValue::JSONObject jsonNotifyableObject = 
            getOneNotifyableStatus(notifyable);
        jsonObj[idNotifyableStatus] = 
            jsonNotifyableObject[idNotifyableStatus];
        jsonObj[idNotifyableState] = ss.str();
    }
    return jsonObj;
}

JSONValue::JSONArray MethodAdaptor::getApplicationStatus(
    const JSONValue::JSONArray &ids) {
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin(); 
         iter != ids.end(); 
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneApplicationStatus(
                    dynamic_cast<Application*>(
                        getNotifyable(id, idTypeApplication))));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid ID (" << ex.what() << ")");
        }
    }

    return status;
}

JSONValue::JSONArray MethodAdaptor::getNodeStatus(
    const JSONValue::JSONArray &ids) {
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin(); 
         iter != ids.end(); 
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneNodeStatus(dynamic_cast<Node*>(
                                     getNotifyable(id, idTypeNode))));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid ID (" << ex.what() << ")");
        }
    }

    return status;
}

JSONValue::JSONArray MethodAdaptor::getProcessSlotStatus(
    const JSONValue::JSONArray &ids) {
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin(); 
         iter != ids.end(); 
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneProcessSlotStatus(
                    dynamic_cast<ProcessSlot*>(
                        getNotifyable(id, idTypeProcessSlot))));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid ID (" << ex.what() << ")");
        }
    }

    return status;
}

JSONValue::JSONArray MethodAdaptor::getGroupStatus(
    const JSONValue::JSONArray &ids) {
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin(); 
         iter != ids.end(); 
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneGroupStatus(dynamic_cast<Group*>(
                                      getNotifyable(id, idTypeGroup))));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid ID (" << ex.what() << ")");
        }
    }

    return status;
}

JSONValue::JSONArray MethodAdaptor::getDataDistributionStatus(
    const JSONValue::JSONArray &ids) {
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin(); 
         iter != ids.end(); 
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneDataDistributionStatus(
                    dynamic_cast<DataDistribution*>(
                        getNotifyable(id, idTypeDataDistribution))));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid ID (" << ex.what() << ")");
        }
    }

    return status;
}

JSONValue::JSONArray MethodAdaptor::getPropertyListStatus(
    const JSONValue::JSONArray &ids) {
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin(); 
         iter != ids.end(); 
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOnePropertyListStatus(
                    dynamic_cast<PropertyList*>(
                        getNotifyable(id, idTypePropertyList))));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid ID (" << ex.what() << ")");
        }
    }
        
    return status;
}

JSONValue::JSONArray MethodAdaptor::getQueueStatus(
    const JSONValue::JSONArray &ids) {
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin(); 
         iter != ids.end(); 
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneQueueStatus(
                    dynamic_cast<Queue*>(
                        getNotifyable(id, idTypeQueue))));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid ID (" << ex.what() << ")");
        }
    }
        
    return status;
}

JSONValue::JSONArray MethodAdaptor::getShardStatus(
    vector<Shard> &shardVec) {
    JSONValue::JSONArray status;        
    for (vector<Shard>::iterator it = shardVec.begin();
         it != shardVec.end();
         it++) {
        try {
            status.push_back(getOneShardStatus(*it));
        } catch (const ::clusterlib::Exception &ex) {
            LOG4CXX_WARN(m_logger, "Invalid shard (" << ex.what() << ")");
        }
    }
    return status;
}

JSONValue::JSONArray MethodAdaptor::getChildrenLockBids(
    JSONValue::JSONString notifyableKey)
{
    Notifyable *notifyable = m_root->getNotifyableFromKey(notifyableKey); 
    NameList bidList = notifyable->getLockBids(true);
    JSONValue::JSONArray bidArr;
    NameList::const_iterator bidListIt;
    for (bidListIt = bidList.begin(); 
         bidListIt != bidList.end();
         ++bidListIt) {
        bidArr.push_back(*bidListIt);
    }
    
    return bidArr;
}

}}}
