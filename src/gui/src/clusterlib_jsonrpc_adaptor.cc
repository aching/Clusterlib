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

#include "gui.h"
#include "clusterlib_jsonrpc_adaptor.h"

using namespace json;
using namespace json::rpc;
using namespace std;
using namespace boost;

namespace clusterlib { 

namespace rpc { 

namespace json {

MethodAdaptor::MethodAdaptor(clusterlib::Client *f)
    : m_client(f),
      m_name("clusterlib::rpc::json::MethodAdaptor")
{
    m_rootSP = m_client->getRoot();
}

const string &
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
MethodAdaptor::invoke(const string &name,
                      const JSONValue::JSONArray &param,
                      StatePersistence *persistence)
{
    if (name == "getNotifyableAttributesFromKey") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one string parameter.");
        }
        return getNotifyableAttributesFromKey(
            param[0].get<JSONValue::JSONString>());
    } else if (name == "setNotifyableAttributesFromKey") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires at one array parameter.");
        }
        return setNotifyableAttributesFromKey(
            param[0].get<JSONValue::JSONArray>());
    } else if (name == "removeNotifyableAttributesFromKey") {
        if (param.size() != 3 ||
            param[0].type() != typeid(JSONValue::JSONString) ||
            param[1].type() != typeid(JSONValue::JSONString) ||
            param[2].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires three string parameter.");
        }
        return removeNotifyableAttributesFromKey(
            param[0].get<JSONValue::JSONString>(),
            param[1].get<JSONValue::JSONString>(),
            param[2].get<JSONValue::JSONString>());
    } else if (name == "getNotifyableChildrenFromKey") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one string parameter.");
        }
        return getNotifyableChildrenFromKey(
            param[0].get<JSONValue::JSONString>());
    } else if (name == "addNotifyableFromKey") {
        if (param.size() != 3 ||
            param[0].type() != typeid(JSONValue::JSONString) ||
            param[1].type() != typeid(JSONValue::JSONString) ||
            param[2].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCParamsException(
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
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one string parameter and "
                "one boolean parameter.");
        }
        return removeNotifyableFromKey(
            param[0].get<JSONValue::JSONString>(),
            param[1].get<JSONValue::JSONBoolean>());
    } else if (name == "getApplication") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getApplication(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getGroup") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getGroup(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getDataDistribution") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getDataDistribution(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getNode") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getNode(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getPropertyList") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one object parameter.");
        }
        return getPropertyList(param[0].get<JSONValue::JSONObject>());
    } else if (name == "getApplications") {
        if (param.size() != 0) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires no parameter.");
        }
        return getApplications();
    } else if (name == "getApplicationStatus") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getApplicationStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getGroupStatus") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getGroupStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getNodeStatus") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getNodeStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getPropertyListStatus") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getPropertyListStatus(param[0].get<JSONValue::JSONArray>());
    } else if (name == "getDataDistributionStatus") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONArray)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one array parameter.");
        }
        return getDataDistributionStatus(
            param[0].get<JSONValue::JSONArray>());
    } else if (name == "getChildrenLockBids") {
        if (param.size() != 1 ||
            param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCParamsException(
                "Method '" + name + "' requires one string parameter.");
        }
        return getChildrenLockBids(
            param[0].get<JSONValue::JSONString>());
    } else {
        throw JSONRPCParamsException(
            "Unknown method '" + name + "' invoked.");
    }
}

JSONValue::JSONObject
MethodAdaptor::getNotifyableId(
    const shared_ptr<Notifyable> &notifyableSP) {
    string type;

    JSONValue::JSONObject id;

    if (dynamic_pointer_cast<Root>(notifyableSP) != NULL)
{
        type = idTypeRoot;
        id[idOptions] =
            optionAddApplication + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_pointer_cast<Application>(notifyableSP) != NULL) {
        type = idTypeApplication;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddGroup + optionDelim +
            optionAddDataDistribution + optionDelim +
            optionAddNode + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_pointer_cast<Group>(notifyableSP) != NULL) {
        type = idTypeGroup;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddGroup + optionDelim +
            optionAddDataDistribution + optionDelim +
            optionAddNode + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_pointer_cast<Node>(notifyableSP) != NULL) {
        type = idTypeNode;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_pointer_cast<ProcessSlot>(notifyableSP) != NULL) {
        type = idTypeProcessSlot;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList + optionDelim +
            optionStartProcess + optionDelim +
            optionStopProcess;
    } else if (dynamic_pointer_cast<DataDistribution>(notifyableSP) != NULL) {
        type = idTypeDataDistribution;
        id[idOptions] =
            optionRemove + optionDelim +
            optionAddQueue + optionDelim +
            optionAddPropertyList;
    } else if (dynamic_pointer_cast<PropertyList >(notifyableSP) != NULL) {
        type = idTypePropertyList;
        id[idOptions] =
            optionRemove;
    } else if (dynamic_pointer_cast<Queue >(notifyableSP) != NULL) {
        type = idTypeQueue;
        id[idOptions] =
            optionRemove;
    } else {
        type = "unknown";
        throw JSONRPCInvocationException(
            "getNotifyableId: No such notifyable");
    }

    /* Find the locks and their bids of this notifyable */
    NameList bidList = notifyableSP->getLockBids(
        CLString::NOTIFYABLE_LOCK, false);
    JSONValue::JSONArray bidArr;
    NameList::const_iterator bidListIt;
    for (bidListIt = bidList.begin();
         bidListIt != bidList.end();
         ++bidListIt) {
        bidArr.push_back(*bidListIt);
    }

    id[idTypeProperty] = type;
    id[idProperty] = notifyableSP->getKey();
    id[idNameProperty] = notifyableSP->getName();
    string ownerId;
    int64_t acquiredOwnerTime = -1;
    bool hasOwner =
        notifyableSP->getLockInfo(CLString::OWNERSHIP_LOCK, 
                                  &ownerId, 
                                  NULL, 
                                  &acquiredOwnerTime);
    if (hasOwner) {
        id[idOwner] = ownerId;
        ostringstream oss;
        oss << TimerService::getMsecsTimeString(acquiredOwnerTime)
            << " (" << acquiredOwnerTime << ")";
        id[idAcquiredOwnerTime] = oss.str();
    }
    id[idBidArr] = bidArr;
    id[idCurrentState] = notifyableSP->cachedCurrentState().getHistoryArray();
    id[idDesiredState] = notifyableSP->cachedDesiredState().getHistoryArray();

    return id;
}

JSONValue::JSONObject
MethodAdaptor::getPropertyList(
    const shared_ptr<PropertyList> &propertyListSP)
{
    JSONValue::JSONObject result;

    // Get all keys and values
    bool exists = false;
    JSONValue jsonValue;
    if (propertyListSP != NULL) {
        vector<JSONValue::JSONString> keys =
            propertyListSP->cachedKeyValues().getKeys();
        vector<JSONValue::JSONString>::const_iterator keysIt;
        for (keysIt= keys.begin(); keysIt != keys.end(); ++keysIt) {
            exists = propertyListSP->cachedKeyValues().get(*keysIt, jsonValue);
            if (exists) {
                result[*keysIt] = jsonValue;
            }
        }
    }

    return result;
}

JSONValue::JSONArray
MethodAdaptor::getApplications() {
    JSONValue::JSONArray appIds;
    NameList appNames = m_rootSP->getApplicationNames();

    for (NameList::const_iterator iter = appNames.begin();
         iter != appNames.end();
         ++iter)
{
        try {
            shared_ptr<Application> childSP = m_rootSP->getApplication(
                *iter, LOAD_FROM_REPOSITORY);
            appIds.push_back(getNotifyableId(childSP));
        }
        catch (const ::clusterlib::Exception &ex) {
            // Ignore any application that cannot be retrieved
            LOG_WARN(CLM_LOG,
                     "Application %s missing (%s)",
                     iter->c_str(),
                     ex.what());
        }
    }

    return appIds;
}

shared_ptr<Notifyable>
MethodAdaptor::getNotifyable(
    const JSONValue::JSONObject &id, const string &expectType)
{
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

    shared_ptr<Notifyable> notifyableSP = m_rootSP->getNotifyableFromKey(
        idIter->second.get<JSONValue::JSONString>());
    if (expectType == idTypeRoot) {
        notifyableSP = dynamic_pointer_cast<Root>(notifyableSP);
    } else if (expectType == idTypeApplication) {
        notifyableSP = dynamic_pointer_cast<Application>(notifyableSP);
    } else if (expectType == idTypeGroup) {
        notifyableSP = dynamic_pointer_cast<Group>(notifyableSP);
    } else if (expectType == idTypeNode) {
        notifyableSP = dynamic_pointer_cast<Node>(notifyableSP);
    } else if (expectType == idTypeProcessSlot) {
        notifyableSP = dynamic_pointer_cast<ProcessSlot>(notifyableSP);
    } else if (expectType == idTypeDataDistribution) {
        notifyableSP = dynamic_pointer_cast<DataDistribution>(notifyableSP);
    }

    if (notifyableSP == NULL) {
        throw JSONRPCInvocationException(
            "The " + expectType + " requested is not found.");
    }

    return notifyableSP;
}

JSONValue::JSONObject
MethodAdaptor::getNotifyableAttributesFromKey(
    const JSONValue::JSONString &key)
{
    try {
        shared_ptr<Notifyable> notifyableSP;

        if (key == "/") {
            notifyableSP = m_rootSP;
        }
        else {
            notifyableSP = m_rootSP->getNotifyableFromKey(key);
        }

        if (notifyableSP == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }

        /* Get the basic attributes again. */
        JSONValue::JSONObject attributes = getNotifyableId(notifyableSP);

        /* Get the children */
        JSONValue::JSONObject childObj = getNotifyableChildrenFromKey(key);

        NameList names;
        if (dynamic_pointer_cast<Root >(notifyableSP)) {
            attributes["# of Applications"] =
                static_cast<uint32_t>(m_rootSP->getApplicationNames().size());
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
        else if (dynamic_pointer_cast<Group >(notifyableSP)) {
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
        else if (dynamic_pointer_cast<DataDistribution >(notifyableSP)) {
            shared_ptr<DataDistribution> dataDistributionSP =
                dynamic_pointer_cast<DataDistribution >(notifyableSP);
            attributes[idDataDistributionHashRangeName] =
                dataDistributionSP->cachedShards().getHashRangeName();
            if (dataDistributionSP->cachedShards().getHashRangeName() !=
                "UnknownHashRange") {
                attributes[idDataDistributionCovered] =
                    dataDistributionSP->cachedShards().isCovered();
            }
            attributes[idShardCount] =
                dataDistributionSP->cachedShards().getCount();
            vector<Shard> shardVec =
                dataDistributionSP->cachedShards().getAllShards();
            attributes[idShardSummary] =
                getShardStatus(shardVec);
            attributes[idPropertyListSummary] =
                getPropertyListStatus(childObj["propertyLists"].
                                      get<JSONValue::JSONArray>());
            attributes[idQueueSummary] =
                getQueueStatus(childObj["queues"].
                               get<JSONValue::JSONArray>());
        }
        else if (dynamic_pointer_cast<Node >(notifyableSP)) {
            shared_ptr<Node> nodeSP =
                dynamic_pointer_cast<Node >(notifyableSP);
            bool useProcessSlots = nodeSP->cachedProcessSlotInfo().getEnable();
            attributes[idNodeUseProcessSlots] = useProcessSlots;
            if (useProcessSlots) {
                attributes[idNodeMaxProcessSlots] =
                    nodeSP->cachedProcessSlotInfo().getMaxProcessSlots();
            }
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
        else if (dynamic_pointer_cast<ProcessSlot >(notifyableSP)) {
            shared_ptr<ProcessSlot> processSlotSP =
                dynamic_pointer_cast<ProcessSlot >(notifyableSP);

            attributes[idPrefixEditable + idPrefixDelim +
                       idProcessSlotHostnameArr] =
                JSONCodec::encode(
                    processSlotSP->cachedProcessInfo().getHostnameArr());
            attributes[idPrefixEditable + idPrefixDelim +
                       idProcessSlotPortArr] =
                JSONCodec::encode(
                    processSlotSP->cachedProcessInfo().getPortArr());
            attributes[idPropertyListSummary] =
                getPropertyListStatus(childObj["propertyLists"].
                                      get<JSONValue::JSONArray>());
            attributes[idQueueSummary] =
                getQueueStatus(childObj["queues"].
                               get<JSONValue::JSONArray>());
        }
        else if (dynamic_pointer_cast<PropertyList >(notifyableSP)) {
            shared_ptr<PropertyList> propertyListSP =
                dynamic_pointer_cast<PropertyList >(notifyableSP);
            attributes[idAddAttribute] = addProperty;
            vector<JSONValue::JSONString> keyVec =
                propertyListSP->cachedKeyValues().getKeys();
            bool exists;
            JSONValue jsonValue;
            for (vector<JSONValue::JSONString>::iterator it = keyVec.begin();
                 it != keyVec.end();
                 ++it) {

                exists = propertyListSP->cachedKeyValues().get(*it, jsonValue);
                if (exists) {
                    /*
                     * Properties are editable to anything and
                     * deletable.
                     */
                    attributes[idPrefixEditable + idPrefixDelim +
                               idPrefixDeletable + idPrefixDelim +
                               idPropertyListProperty + " " + *it]
                        = jsonValue;
                }
            }
        }
        else if (dynamic_pointer_cast<Queue >(notifyableSP)) {
            attributes[idAddAttribute] = addQueueElement;
            shared_ptr<Queue> queueSP =
                dynamic_pointer_cast<Queue >(notifyableSP);
            map<int64_t, string> idElementMap =
                queueSP->getAllElements();
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
            attributes[idQueueCount] = queueSP->size();
        }

        return attributes;
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "Getting notifyable failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString
MethodAdaptor::setNotifyableAttributesFromKey(
    const JSONValue::JSONArray &arr)
{
    ostringstream oss;
    try {
        if (arr.size() < 1) {
            throw JSONRPCInvocationException(
                "Arr is less than size of 1 ");
        }

        string key = arr[setAttributeKeyIdx].get<JSONValue::JSONString>();
        shared_ptr<Notifyable> notifyableSP =
            m_rootSP->getNotifyableFromKey(key);
        if (notifyableSP == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " +
                key);
        }

        if (dynamic_pointer_cast<Root >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for root key " +
                key);
        }
        else if (dynamic_pointer_cast<Application >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for group key " +
                key);
        }
        else if (dynamic_pointer_cast<Group >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for root key " +
                key);
        }
        else if (dynamic_pointer_cast<DataDistribution >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for data distribution key " +
                key);
        }
        else if (dynamic_pointer_cast<Node >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for node key " +
                key);
        }
        else if (dynamic_pointer_cast<ProcessSlot >(notifyableSP)) {
            if (arr.size() != 4) {
                throw JSONRPCInvocationException(
                    "Need 4 elements in array");
            }

            shared_ptr<ProcessSlot> processSlotSP =
                dynamic_pointer_cast<ProcessSlot >(notifyableSP);
            string op =
                arr[setAttributeKeyIdx + 2].get<JSONValue::JSONString>();

            if (!op.compare(
                    0,
                    optionStartProcess.size(),
                    optionStartProcess)) {

                processSlotSP->cachedDesiredState().set(
                    ProcessSlot::PROCESS_STATE_KEY,
                    ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE);
                try {
                    processSlotSP->cachedDesiredState().publish();
                }
                catch (PublishVersionException e) {
                    throw JSONRPCInvocationException(
                        string("publish failed with: ") + e.what());
                }
            }
            else if (!op.compare(
                         0,
                         optionStopProcess.size(),
                         optionStopProcess)) {
                processSlotSP->cachedDesiredState().set(
                    ProcessSlot::PROCESS_STATE_KEY,
                    ProcessSlot::PROCESS_STATE_EXIT_VALUE);
                processSlotSP->cachedDesiredState().set(
                    ProcessSlot::PROCESS_STATE_SET_MSECS_KEY,
                    TimerService::getMsecsTimeString());
                try {
                    processSlotSP->cachedDesiredState().publish();
                }
                catch (PublishVersionException e) {
                    throw JSONRPCInvocationException(
                        string("publish failed with: ") + e.what());
                }
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for process slot key " +
                    key);
            }
        }
        else if (dynamic_pointer_cast<PropertyList >(notifyableSP)) {
            if (arr.size() != 4) {
                throw JSONRPCInvocationException(
                    "Need 4 elements in array");
            }

            shared_ptr<PropertyList> propertyListSP =
                dynamic_pointer_cast<PropertyList >(notifyableSP);
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
                NotifyableLocker l(propertyListSP,
                                   CLString::NOTIFYABLE_LOCK,
                                   DIST_LOCK_EXCL,
                                   maxLockWaitMsecs);

                if (!l.hasLock()) {
                    oss.str("");
                    oss << "Failed to set property " << property
                        << " with value " << value << " within "
                        << maxLockWaitMsecs << " msecs";
                    throw JSONRPCInvocationException(oss.str());
                }
                propertyListSP->cachedKeyValues().set(property, value);
                propertyListSP->cachedKeyValues().publish();
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for property list key " +
                    key);
            }
        }
        else if (dynamic_pointer_cast<Queue >(notifyableSP)) {
            if (arr.size() != 4) {
                throw JSONRPCInvocationException(
                    "Needs 4 elements in array");
            }

            string op =
                arr[setAttributeKeyIdx + 1].get<JSONValue::JSONString>();
            shared_ptr<Queue> queueSP = 
                dynamic_pointer_cast<Queue >(notifyableSP);
            if (!op.compare("Add queue element")) {
                queueSP->put(arr[setAttributeKeyIdx + 3].
                           get<JSONValue::JSONString>());
            }
        }
        return string("0");
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "Getting notifyable failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString
MethodAdaptor::removeNotifyableAttributesFromKey(
    const JSONValue::JSONString &key,
    const JSONValue::JSONString &op,
    const JSONValue::JSONString &attribute)
{
    ostringstream oss;
    try {
        shared_ptr<Notifyable> notifyableSP = 
            m_rootSP->getNotifyableFromKey(key);
        if (notifyableSP == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }

        if (dynamic_pointer_cast<Root >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for root key " + key);
        }
        else if (dynamic_pointer_cast<Application >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for group key " + key);
        }
        else if (dynamic_pointer_cast<Group >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for root key " + key);
        }
        else if (dynamic_pointer_cast<DataDistribution >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for data distribution key " + key);
        }
        else if (dynamic_pointer_cast<Node >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for node key " + key);
        }
        else if (dynamic_pointer_cast<ProcessSlot >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for process slot key " + key);
        }
        else if (dynamic_pointer_cast<PropertyList >(notifyableSP)) {
            shared_ptr<PropertyList> propertyListSP =
                dynamic_pointer_cast<PropertyList >(notifyableSP);
            if (!op.compare(idPropertyListProperty)) {
                NotifyableLocker l(propertyListSP,
                                   CLString::NOTIFYABLE_LOCK,
                                   DIST_LOCK_EXCL,
                                   maxLockWaitMsecs);

                if (!l.hasLock()) {
                    oss.str("");
                    oss << "Failed to delete property " << attribute
                        << " within " << maxLockWaitMsecs << " msecs";
                    throw JSONRPCInvocationException(oss.str());
                }
                propertyListSP->cachedKeyValues().erase(attribute);
                propertyListSP->cachedKeyValues().publish();
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for property list key " + key);
            }
        }
        else if (dynamic_pointer_cast<Queue >(notifyableSP)) {
            shared_ptr<Queue> queueSP = 
                dynamic_pointer_cast<Queue >(notifyableSP);
            if (!op.compare(idQueueElementPrefix)) {
                stringstream ss;
                ss << attribute;
                int64_t id;
                ss >> id;
                queueSP->removeElement(id);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for queue key " + key);
            }
        }
        return string("0");
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "Getting notifyable failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getNotifyableChildrenFromKey(
    const JSONValue::JSONString &name)
{
    try {
        shared_ptr<Notifyable> notifyableSP;
        JSONValue::JSONObject children;
        JSONValue::JSONArray root, applications, groups,
            dataDistributions, nodes, processSlots, propertyLists, queues;

        /* Special case of getting the root */
        if (name == "/") {
            root.push_back(getNotifyableId(m_rootSP));
            children["root"] = root;
            return children;
        }
        else {
            notifyableSP = m_rootSP->getNotifyableFromKey(name);
        }

        if (notifyableSP == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key." + name);
        }

        NameList names;
        if (dynamic_pointer_cast<Root >(notifyableSP)) {
            names = m_rootSP->getApplicationNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<Application> childSP =
                    m_rootSP->getApplication(*iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    applications.push_back(getNotifyableId(childSP));
                }
            }
        }
        else if (dynamic_pointer_cast<Application >(notifyableSP)) {
            shared_ptr<Application> applicationSP =
                (dynamic_pointer_cast<Application >(notifyableSP));
            names = applicationSP->getGroupNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<Group> childSP = applicationSP->getGroup(
                    *iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    groups.push_back(getNotifyableId(childSP));
                }
            }
            names = applicationSP->getDataDistributionNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<DataDistribution> childSP =
                    applicationSP->getDataDistribution(
                        *iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    dataDistributions.push_back(getNotifyableId(childSP));
                }
            }
            names = applicationSP->getNodeNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<Node> childSP = applicationSP->getNode(
                    *iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    nodes.push_back(getNotifyableId(childSP));
                }
            }
        }
        else if (dynamic_pointer_cast<Group >(notifyableSP)) {
            shared_ptr<Group> groupSP = 
                dynamic_pointer_cast<Group >(notifyableSP);
            names = groupSP->getGroupNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<Group> childSP = groupSP->getGroup(
                    *iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    groups.push_back(getNotifyableId(childSP));
                }
            }
            names = groupSP->getDataDistributionNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<DataDistribution> childSP =
                    groupSP->getDataDistribution(*iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    dataDistributions.push_back(getNotifyableId(childSP));
                }
            }
            names = groupSP->getNodeNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<Node> childSP = groupSP->getNode(
                    *iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    nodes.push_back(getNotifyableId(childSP));
                }
            }
        }
        else if (dynamic_pointer_cast<Node >(notifyableSP)) {
            shared_ptr<Node> nodeSP =
                (dynamic_pointer_cast<Node >(notifyableSP));
            names = nodeSP->getProcessSlotNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<ProcessSlot> childSP = nodeSP->getProcessSlot(
                    *iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    processSlots.push_back(getNotifyableId(childSP));
                }
            }
        }

        /* If not a PropertyList, object can search for PropertyLists */
        if (!dynamic_pointer_cast<PropertyList >(notifyableSP)) {
            names = notifyableSP->getPropertyListNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<PropertyList> childSP = 
                    notifyableSP->getPropertyList(*iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    propertyLists.push_back(getNotifyableId(childSP));
                }
            }
        }

        /*
         * If not a PropertyList or a Queue, object can search for
         * Queues
         */
        if ((!dynamic_pointer_cast<PropertyList >(notifyableSP) &&
             !dynamic_pointer_cast<Queue >(notifyableSP))) {
            names = notifyableSP->getQueueNames();
            for (NameList::const_iterator iter = names.begin();
                 iter != names.end();
                 ++iter) {
                shared_ptr<Queue> childSP = 
                    notifyableSP->getQueue(*iter, LOAD_FROM_REPOSITORY);
                if (childSP) {
                    queues.push_back(getNotifyableId(childSP));
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
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "Getting notifyable failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString
MethodAdaptor::addNotifyableFromKey(
    const JSONValue::JSONString &key,
    const JSONValue::JSONString &op,
    const JSONValue::JSONString &name)
{
    try {
        shared_ptr<Notifyable> notifyableSP = 
            m_rootSP->getNotifyableFromKey(key);
        if (notifyableSP == NULL) {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }

        if (dynamic_pointer_cast<Root >(notifyableSP)) {
            if (!op.compare(optionAddApplication)) {
                notifyableSP = m_rootSP->getApplication(
                    name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyableSP = m_rootSP->getPropertyList(
                    name, CREATE_IF_NOT_FOUND);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for root key " + key);
            }
        }
        else if (dynamic_pointer_cast<Application >(notifyableSP)) {
            shared_ptr<Application> applicationSP =
                dynamic_pointer_cast<Application >(notifyableSP);
            if (!op.compare(optionAddGroup)) {
                notifyableSP = applicationSP->getGroup(
                    name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddDataDistribution)) {
                notifyableSP = applicationSP->getDataDistribution(
                    name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddNode)) {
                notifyableSP = applicationSP->getNode(
                    name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddQueue)) {
                notifyableSP = applicationSP->getQueue(
                    name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyableSP = applicationSP->getPropertyList(
                    name, CREATE_IF_NOT_FOUND);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for group key " + key);
            }
        }
        else if (dynamic_pointer_cast<Group >(notifyableSP)) {
            shared_ptr<Group> groupSP = 
                dynamic_pointer_cast<Group >(notifyableSP);
            if (!op.compare(optionAddGroup)) {
                notifyableSP = groupSP->getGroup(name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddDataDistribution)) {
                notifyableSP = groupSP->getDataDistribution(name,
                                                        CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddNode)) {
                notifyableSP = groupSP->getNode(name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddQueue)) {
                notifyableSP = groupSP->getQueue(name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyableSP = groupSP->getPropertyList(
                    name, CREATE_IF_NOT_FOUND);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for root key " + key);
            }
        }
        else if (dynamic_pointer_cast<DataDistribution >(notifyableSP)) {
            shared_ptr<DataDistribution> dataDistributionSP =
                dynamic_pointer_cast<DataDistribution >(notifyableSP);
            if (!op.compare(optionAddQueue)) {
                notifyableSP = dataDistributionSP->getQueue(
                    name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyableSP = dataDistributionSP->getPropertyList(
                    name, CREATE_IF_NOT_FOUND);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for data distribution key " + key);
            }
        }
        else if (dynamic_pointer_cast<Node >(notifyableSP)) {
            shared_ptr<Node> nodeSP = 
                dynamic_pointer_cast<Node >(notifyableSP);
            if (!op.compare(optionAddQueue)) {
                notifyableSP = nodeSP->getQueue(name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyableSP = nodeSP->getPropertyList(
                    name, CREATE_IF_NOT_FOUND);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for node key " + key);
            }
        }
        else if (dynamic_pointer_cast<ProcessSlot >(notifyableSP)) {
            shared_ptr<ProcessSlot> processSlotSP =
                dynamic_pointer_cast<ProcessSlot >(notifyableSP);
            if (!op.compare(optionAddQueue)) {
                notifyableSP = processSlotSP->getQueue(
                    name, CREATE_IF_NOT_FOUND);
            }
            else if (!op.compare(optionAddPropertyList)) {
                notifyableSP = processSlotSP->getPropertyList(
                    name, CREATE_IF_NOT_FOUND);
            }
            else {
                throw JSONRPCInvocationException(
                    "No such op for node key " + key);
            }
        }
        else if (dynamic_pointer_cast<PropertyList >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for property list key " + key);
        }
        else if (dynamic_pointer_cast<Queue >(notifyableSP)) {
            throw JSONRPCInvocationException(
                "No such op for queue key " + key);
        }
        else if (notifyableSP == NULL) {
            throw JSONRPCInvocationException(
                "Creating the notifyable failed or removed right "
                "away for key " + key);
        }
        else {
            throw JSONRPCInvocationException(
                "Cannot get Notifyable from key " + key);
        }
        return notifyableSP->getKey();
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "addNotifyableFromKey: Getting notifyable failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONString
MethodAdaptor::removeNotifyableFromKey(
    const JSONValue::JSONString &key,
    const JSONValue::JSONBoolean &removeChildren)
{
    try {
        shared_ptr<Notifyable> notifyableSP = 
            m_rootSP->getNotifyableFromKey(key);
        if (notifyableSP == NULL) {
            throw JSONRPCInvocationException(
                "removeNotifyableFromKey: Cannot get Notifyable from key "
                + key);
        }
        notifyableSP->remove(removeChildren);
        return string("0");
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "removeNotifyableFromKey: %s",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getApplication(
    const JSONValue::JSONObject &name)
{
    try {
        shared_ptr<Application> applicationSP =
            dynamic_pointer_cast<Application>(getNotifyable(name,
                                                     idTypeApplication));
        NameList groupNames = applicationSP->getGroupNames();
        NameList distributionNames =
            applicationSP->getDataDistributionNames();
        NameList nodeNames = applicationSP->getNodeNames();
        NameList propertyListNames = applicationSP->getPropertyListNames();

        JSONValue::JSONArray groups, dataDistributions, nodes, propertyLists;

        // Transform groups into groupSP IDs
        for (NameList::const_iterator iter = groupNames.begin();
             iter != groupNames.end();
             ++iter) {
            shared_ptr<Group> childSP = 
                applicationSP->getGroup(*iter, LOAD_FROM_REPOSITORY);
            groups.push_back(getNotifyableId(childSP));
        }

        // Transform distributions into distribution IDs
        for (NameList::const_iterator iter = distributionNames.begin();
             iter != distributionNames.end(); ++iter) {
            shared_ptr<DataDistribution> childSP =
                applicationSP->getDataDistribution(
                    *iter, LOAD_FROM_REPOSITORY);
            dataDistributions.push_back(getNotifyableId(childSP));
        }

        // Transform nodes into node IDs
        for (NameList::const_iterator iter = nodeNames.begin();
             iter != nodeNames.end();
             ++iter) {
            shared_ptr<Node> childSP = applicationSP->getNode(
                *iter, LOAD_FROM_REPOSITORY);
            nodes.push_back(getNotifyableId(childSP));
        }

        // Transform propertyLists into propertyList IDs
        for (NameList::const_iterator iter = propertyListNames.begin();
             iter != propertyListNames.end();
             ++iter) {
            shared_ptr<PropertyList> childSP = 
                applicationSP->getPropertyList(*iter, LOAD_FROM_REPOSITORY);
            propertyLists.push_back(getNotifyableId(childSP));
        }

        JSONValue::JSONObject result;
        result["groups"] = groups;
        result["dataDistributions"] = dataDistributions;
        result["nodes"] = nodes;
        result["properties"] = getPropertyList(
            applicationSP->getPropertyList(
                CLString::DEFAULT_PROPERTYLIST, LOAD_FROM_REPOSITORY));
        result["propertyLists"] = propertyLists;
        result["status"] = getOneApplicationStatus(applicationSP);

        return result;
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "Getting application failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getGroup(
    const JSONValue::JSONObject &name)
{
    try {
        shared_ptr<Group> groupSP =
            dynamic_pointer_cast<Group>(getNotifyable(name, idTypeGroup));
        NameList groupNames = groupSP->getGroupNames();
        NameList distributionNames = groupSP->getDataDistributionNames();
        NameList nodeNames = groupSP->getNodeNames();
        NameList propertyListNames = groupSP->getPropertyListNames();

        JSONValue::JSONArray groups, dataDistributions, nodes, propertyLists;

        // Transform groups into group IDs
        for (NameList::const_iterator iter = groupNames.begin();
             iter != groupNames.end();
             ++iter) {
            shared_ptr<Group> childSP = 
                groupSP->getGroup(*iter, LOAD_FROM_REPOSITORY);
            groups.push_back(getNotifyableId(childSP));
        }

        // Transform distributions into distribution IDs
        for (NameList::const_iterator iter = distributionNames.begin();
             iter != distributionNames.end();
             ++iter) {
            shared_ptr<DataDistribution> childSP = 
                groupSP->getDataDistribution(*iter, LOAD_FROM_REPOSITORY);
            dataDistributions.push_back(getNotifyableId(childSP));
        }

        // Transform nodes into node IDs
        for (NameList::const_iterator iter = nodeNames.begin();
             iter != nodeNames.end();
             ++iter) {
            shared_ptr<Node> childSP = 
                groupSP->getNode(*iter, LOAD_FROM_REPOSITORY);
            nodes.push_back(getNotifyableId(childSP));
        }

        // Transform propertyLists into propertyList IDs
        for (NameList::const_iterator iter = propertyListNames.begin();
             iter != propertyListNames.end();
             ++iter) {
            shared_ptr<PropertyList> childSP = 
                groupSP->getPropertyList(*iter, LOAD_FROM_REPOSITORY);
            propertyLists.push_back(getNotifyableId(childSP));
        }

        JSONValue::JSONObject result;
        result["parent"] = getNotifyableId(groupSP->getMyParent());
        result["groups"] = groups;
        result["dataDistributions"] = dataDistributions;
        result["nodes"] = nodes;
        result["properties"] = getPropertyList(groupSP->getPropertyList(
            CLString::DEFAULT_PROPERTYLIST, LOAD_FROM_REPOSITORY));
        result["propertyListObjects"] = propertyLists;
        result["status"] = getOneGroupStatus(groupSP);

        return result;
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG, "Getting group failed (%s)", ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getDataDistribution(
    const JSONValue::JSONObject &name)
{
    try {
        shared_ptr<DataDistribution> dataDistributionSP =
            dynamic_pointer_cast<DataDistribution>(
                getNotifyable(name, idTypeDataDistribution));

        JSONValue::JSONArray shards;

        vector<Shard> shardVec = 
            dataDistributionSP->cachedShards().getAllShards();

        // Transform shards
        for (uint32_t i = 0; i < shardVec.size(); ++i) {
            JSONValue::JSONObject shard;
            ostringstream oss;
            oss << JSONCodec::encode(
                shardVec[i].getStartRange().toJSONValue());
            shard["low"] = oss.str();
            oss.str("");
            oss << JSONCodec::encode(
                shardVec[i].getEndRange().toJSONValue());
            shard["high"] = oss.str();
            oss.str("");
            oss << shardVec[i].getPriority();
            shard["priority"] = oss.str();
            if (shardVec[i].getNotifyable() != NULL) {
                shard["id"] = getNotifyableId(
                    m_rootSP->getNotifyableFromKey(
                        shardVec[i].getNotifyable()->getKey()));
            } else {
                shard["id"] = JSONValue::Null;
            }
            shards.push_back(shard);
        }

        JSONValue::JSONObject result;
        bool covered = dataDistributionSP->cachedShards().isCovered();
        result["parent"] = getNotifyableId(dataDistributionSP->getMyParent());
        result["isCovered"] = covered;
        result["shards"] = shards;
        result["properties"] =
            getPropertyList(dataDistributionSP->getPropertyList(
                CLString::DEFAULT_PROPERTYLIST, LOAD_FROM_REPOSITORY));
        result["status"] = getOneDataDistributionStatus(dataDistributionSP);

        return result;
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "Getting data distribution failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getNode(
    const JSONValue::JSONObject &name)
{
    try {
        shared_ptr<Node> nodeSP = 
            dynamic_pointer_cast<Node>(getNotifyable(name, idTypeNode));
        string ownerId;
        int64_t ownerAcquiredTime = -1;
        nodeSP->getLockInfo(CLString::NOTIFYABLE_LOCK,
                            &ownerId,
                            NULL, 
                            &ownerAcquiredTime);
        JSONValue::JSONObject result;
        result["owner"] = ownerId;
        result["ownerAcquiredTime"] = ownerAcquiredTime;
        result["parent"] = getNotifyableId(nodeSP->getMyParent());
        result["status"] = getOneNodeStatus(nodeSP);
        result["properties"] = getPropertyList(nodeSP->getPropertyList(
            CLString::DEFAULT_PROPERTYLIST, CREATE_IF_NOT_FOUND));

        return result;
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG, "Getting node failed (%s)", ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getPropertyList(
    const JSONValue::JSONObject &name)
{
    try {
        shared_ptr<PropertyList> propertyListSP =
            dynamic_pointer_cast<PropertyList>(getNotifyable(name,
                                                      idTypePropertyList));

        JSONValue::JSONObject result;
        result["parent"] = getNotifyableId(propertyListSP->getMyParent());
        result["status"] = getOnePropertyListStatus(propertyListSP);
        result["properties"] = getPropertyList(propertyListSP);

        return result;
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "Getting propertyList failed (%s)", ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getOneNotifyableStatus(
    const shared_ptr<Notifyable> &notifyableSP)
{
    try {
        if (dynamic_pointer_cast<Application >(notifyableSP)) {
            return getOneApplicationStatus(
                dynamic_pointer_cast<Application >(notifyableSP));
        }
        else if (dynamic_pointer_cast<Group >(notifyableSP)) {
            return getOneGroupStatus(
                dynamic_pointer_cast<Group >(notifyableSP));
        }
        else if (dynamic_pointer_cast<DataDistribution >(notifyableSP)) {
            return getOneDataDistributionStatus(
                dynamic_pointer_cast<DataDistribution >(notifyableSP));
        }
        else if (dynamic_pointer_cast<Node >(notifyableSP)) {
            return getOneNodeStatus(
                dynamic_pointer_cast<Node >(notifyableSP));
        }
        else if (dynamic_pointer_cast<ProcessSlot >(notifyableSP)) {
            return getOneProcessSlotStatus(
                dynamic_pointer_cast<ProcessSlot >(notifyableSP));
        }
        else if (dynamic_pointer_cast<PropertyList >(notifyableSP)) {
            return getOnePropertyListStatus(
                dynamic_pointer_cast<PropertyList >(notifyableSP));
        }
        else if (dynamic_pointer_cast<Queue >(notifyableSP)) {
            return getOneQueueStatus(
                dynamic_pointer_cast<Queue >(notifyableSP));
        }
        else {
            throw JSONRPCInvocationException("Invalid notifyable type");
        }
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CLM_LOG,
                 "getOneNotifyableStatus: Failed (%s)",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
MethodAdaptor::getOneApplicationStatus(
    const shared_ptr<Application> &applicationSP)
{
    // In clusterlib, application is a special group
    return getOneGroupStatus(applicationSP);
}

JSONValue::JSONObject
MethodAdaptor::getOneNodeStatus(
    const shared_ptr<Node> &nodeSP)
{
    // Should change this to other condition
    bool owner = nodeSP->getLockInfo(CLString::OWNERSHIP_LOCK);
    JSONValue jsonHealth;
    bool healthy = false;
    bool found = 
        nodeSP->cachedCurrentState().get(Node::HEALTH_KEY, jsonHealth);
    if (found &&
        (jsonHealth.get<JSONValue::JSONString>() == Node::HEALTH_GOOD_VALUE)) {
        healthy = true;
    }

    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = nodeSP->getKey();
    jsonObj[idNameProperty] = nodeSP->getName();
    jsonObj[idNotifyableStatus] = statusReady;

    if (owner && healthy) {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Has owner and is healthy";
    }
    else if (owner || healthy) {
        jsonObj[idNotifyableStatus] = statusWarning;
        jsonObj[idNotifyableState] = "Either no owner or not healthy";
    }
    else {
        jsonObj[idNotifyableStatus] = statusBad;
        jsonObj[idNotifyableState] = "No owner and not healthy";
    }

    return jsonObj;
}

JSONValue::JSONObject
MethodAdaptor::getOneProcessSlotStatus(
    const shared_ptr<ProcessSlot> &processSlotSP)
{
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = processSlotSP->getKey();
    jsonObj[idNameProperty] = processSlotSP->getName();
    jsonObj[idNotifyableStatus] = statusReady;

    JSONValue state;
    bool found = processSlotSP->cachedCurrentState().get(
        ProcessSlot::PROCESS_STATE_KEY, state);
    if (!found) {
        jsonObj[idNotifyableStatus] = statusInactive;
        jsonObj[idNotifyableState] = "Not being used";
    }
    else if (state.get<JSONValue::JSONString>() ==
             ProcessSlot::PROCESS_STATE_RUNNING_VALUE) {
        jsonObj[idNotifyableStatus] = statusWorking;
        jsonObj[idNotifyableState] = "Running a process";
    }
    else if (state.get<JSONValue::JSONString>() ==
             ProcessSlot::PROCESS_STATE_EXIT_VALUE) {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Finished a process";
    }
    else {
        jsonObj[idNotifyableStatus] = statusBad;
        jsonObj[idNotifyableState] = "Unknown problem";
    }

    return jsonObj;
}

JSONValue::JSONObject
MethodAdaptor::getOneGroupStatus(
    const shared_ptr<Group> &groupSP)
{
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = groupSP->getKey();
    jsonObj[idNameProperty] = groupSP->getName();
    jsonObj[idNotifyableStatus] = statusReady;
    jsonObj[idNotifyableState] = "No problems";
    string notifyableStatus;

    // Check for all groups
    NameList names = groupSP->getGroupNames();
    for (NameList::const_iterator iter = names.begin();
         iter != names.end();
         ++iter) {
        shared_ptr<Group> childGroupSP = 
            groupSP->getGroup(*iter, LOAD_FROM_REPOSITORY);
        if (childGroupSP == NULL) {
            continue;
        }
        notifyableStatus =
            getOneGroupStatus(
                childGroupSP)[idNotifyableStatus].get<JSONValue::JSONString>();
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
    names = groupSP->getDataDistributionNames();
    for (NameList::const_iterator iter = names.begin();
         iter != names.end();
         ++iter) {
        shared_ptr<DataDistribution> dataDistributionSP = 
            groupSP->getDataDistribution(*iter, LOAD_FROM_REPOSITORY);
        if (dataDistributionSP == NULL) {
            continue;
        }
        notifyableStatus =
            getOneDataDistributionStatus(dataDistributionSP)
            [idNotifyableStatus].get<JSONValue::JSONString>();
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
    names = groupSP->getNodeNames();
    for (NameList::const_iterator iter = names.begin();
         iter != names.end();
         ++iter) {
        shared_ptr<Node> nodeSP = groupSP->getNode(
            *iter, LOAD_FROM_REPOSITORY);
        if (nodeSP == NULL) {
            continue;
        }
        notifyableStatus =
            getOneNodeStatus(
                nodeSP)[idNotifyableStatus].get<JSONValue::JSONString>();
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

JSONValue::JSONObject
MethodAdaptor::getOneDataDistributionStatus(
    const shared_ptr<DataDistribution> &dataDistributionSP)
{
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = dataDistributionSP->getKey();
    jsonObj[idNameProperty] = dataDistributionSP->getName();
    if (dataDistributionSP->cachedShards().getCount() <= 0) {
        jsonObj[idNotifyableStatus] = statusInactive;
    }
    else if (dataDistributionSP->cachedShards().isCovered()) {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Covered and ready";
    }
    else {
        jsonObj[idNotifyableStatus] = statusBad;
        jsonObj[idNotifyableState] = "Not covered";
    }

    return jsonObj;
}

JSONValue::JSONObject
MethodAdaptor::getOnePropertyListStatus(
    const shared_ptr<PropertyList> &propertyListSP)
{
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = propertyListSP->getKey();
    jsonObj[idNameProperty] = propertyListSP->getName();
    if (propertyListSP->cachedKeyValues().getKeys().size() == 0) {
        jsonObj[idNotifyableStatus] = statusInactive;
        jsonObj[idNotifyableState] = "No keys used";
    }
    else {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Keys are being used";
    }
    return jsonObj;
}

JSONValue::JSONObject
MethodAdaptor::getOneQueueStatus(
    const shared_ptr<Queue> &queueSP)
{
    JSONValue::JSONObject jsonObj;
    jsonObj[idProperty] = queueSP->getKey();
    jsonObj[idNameProperty] = queueSP->getName();
    if (queueSP->size() == 0) {
        jsonObj[idNotifyableStatus] = statusInactive;
        jsonObj[idNotifyableState] = "Empty queue";
    }
    else {
        jsonObj[idNotifyableStatus] = statusReady;
        jsonObj[idNotifyableState] = "Queue is being used";
    }
    return jsonObj;
}

JSONValue::JSONObject
MethodAdaptor::getOneShardStatus(
    Shard &shard)
{
    JSONValue::JSONObject jsonObj;
    ostringstream oss;
    oss << "start=" << JSONCodec::encode(shard.getStartRange().toJSONValue())
        << ", end=" << JSONCodec::encode(shard.getEndRange().toJSONValue());
    shared_ptr<Notifyable> notifyableSP = shard.getNotifyable();
    if (notifyableSP == NULL) {
        jsonObj[idProperty] = "N/A";
        jsonObj[idNameProperty] = "N/A";
        jsonObj[idNotifyableStatus] = "N/A";
        jsonObj[idNotifyableState] = "N/A";
    }
    else {
        jsonObj[idProperty] = notifyableSP->getKey();
        jsonObj[idNameProperty] = notifyableSP->getName();
        JSONValue::JSONObject jsonNotifyableObject =
            getOneNotifyableStatus(notifyableSP);
        jsonObj[idNotifyableStatus] =
            jsonNotifyableObject[idNotifyableStatus];
        jsonObj[idNotifyableState] = oss.str();
    }
    return jsonObj;
}

JSONValue::JSONArray
MethodAdaptor::getApplicationStatus(
    const JSONValue::JSONArray &ids)
{
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin();
         iter != ids.end();
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneApplicationStatus(
                    dynamic_pointer_cast<Application>(
                        getNotifyable(id, idTypeApplication))));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid ID (%s)", ex.what());
        }
    }

    return status;
}

JSONValue::JSONArray
MethodAdaptor::getNodeStatus(
    const JSONValue::JSONArray &ids)
{
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin();
         iter != ids.end();
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneNodeStatus(dynamic_pointer_cast<Node>(
                                     getNotifyable(id, idTypeNode))));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid ID (%s)", ex.what());
        }
    }

    return status;
}

JSONValue::JSONArray
MethodAdaptor::getProcessSlotStatus(
    const JSONValue::JSONArray &ids)
{
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin();
         iter != ids.end();
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneProcessSlotStatus(
                    dynamic_pointer_cast<ProcessSlot>(
                        getNotifyable(id, idTypeProcessSlot))));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid ID (%s)", ex.what());
        }
    }

    return status;
}

JSONValue::JSONArray
MethodAdaptor::getGroupStatus(
    const JSONValue::JSONArray &ids)
{
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin();
         iter != ids.end();
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneGroupStatus(dynamic_pointer_cast<Group>(
                    getNotifyable(id, idTypeGroup))));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid ID (%s)", ex.what());
        }
    }

    return status;
}

JSONValue::JSONArray
MethodAdaptor::getDataDistributionStatus(
    const JSONValue::JSONArray &ids)
{
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin();
         iter != ids.end();
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneDataDistributionStatus(
                    dynamic_pointer_cast<DataDistribution>(
                        getNotifyable(id, idTypeDataDistribution))));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid ID (%s)", ex.what());
        }
    }

    return status;
}

JSONValue::JSONArray
MethodAdaptor::getPropertyListStatus(
    const JSONValue::JSONArray &ids)
{
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin();
         iter != ids.end();
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOnePropertyListStatus(
                    dynamic_pointer_cast<PropertyList>(
                        getNotifyable(id, idTypePropertyList))));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid ID (%s)", ex.what());
        }
    }

    return status;
}

JSONValue::JSONArray
MethodAdaptor::getQueueStatus(
    const JSONValue::JSONArray &ids)
{
    JSONValue::JSONArray status;
    for (JSONValue::JSONArray::const_iterator iter = ids.begin();
         iter != ids.end();
         ++iter) {
        try {
            JSONValue::JSONObject id = iter->get<JSONValue::JSONObject>();
            status.push_back(
                getOneQueueStatus(
                    dynamic_pointer_cast<Queue>(
                        getNotifyable(id, idTypeQueue))));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid ID (%s)", ex.what());
        }
    }

    return status;
}

JSONValue::JSONArray
MethodAdaptor::getShardStatus(
    vector<Shard> &shardVec)
{
    JSONValue::JSONArray status;
    for (vector<Shard>::iterator it = shardVec.begin();
         it != shardVec.end();
         it++) {
        try {
            status.push_back(getOneShardStatus(*it));
        }
        catch (const ::clusterlib::Exception &ex) {
            LOG_WARN(CLM_LOG, "Invalid shard (%s)", ex.what());
        }
    }
    return status;
}

JSONValue::JSONArray
MethodAdaptor::getChildrenLockBids(
    JSONValue::JSONString notifyableKey)
{
    shared_ptr<Notifyable> notifyableSP = 
        m_rootSP->getNotifyableFromKey(notifyableKey);
    NameList bidList = notifyableSP->getLockBids(
        CLString::NOTIFYABLE_LOCK, true);
    JSONValue::JSONArray bidArr;
    NameList::const_iterator bidListIt;
    for (bidListIt = bidList.begin();
         bidListIt != bidList.end();
         ++bidListIt) {
        bidArr.push_back(*bidListIt);
    }

    return bidArr;
}

}

}

}

