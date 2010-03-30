/* 
 * startprocessmethod.cc --
 *
 * Implementation of the StartProcessMethod class.

 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace clusterlib;
using namespace json;
using namespace json::rpc;
using namespace log4cxx;
using namespace std;

namespace clusterlib {

const string &
StartProcessRPC::getName() const
{
    return ClusterlibStrings::RPC_START_PROCESS;
}

void
StartProcessRPC::checkParams(const JSONValue::JSONArray &paramArr)
{
    if (paramArr.size() != 1) {
        throw JSONRPCInvocationException(
            "checkParams: Expecting one array element");
    }
    JSONValue::JSONObject paramObj = 
        paramArr[0].get<JSONValue::JSONObject>();
    JSONValue::JSONObject::const_iterator paramObjIt = paramObj.find(
        ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY);
    if (paramObjIt == paramObj.end()) {
        throw JSONRPCInvocationException("checkParams: No notifyable!");
    }
    paramObjIt = paramObj.find(ClusterlibStrings::JSONOBJECTKEY_ADDENV);
    if (paramObjIt == paramObj.end()) {
        throw JSONRPCInvocationException("checkParams: No addenv!");
    }
    paramObjIt = paramObj.find(ClusterlibStrings::JSONOBJECTKEY_PATH);
    if (paramObjIt == paramObj.end()) {
        throw JSONRPCInvocationException("checkParams: No path!");
    }
    paramObjIt = paramObj.find(ClusterlibStrings::JSONOBJECTKEY_COMMAND);
    if (paramObjIt == paramObj.end()) {
        throw JSONRPCInvocationException("checkParams: No command!");
    }
}

JSONValue 
StartProcessMethod::invoke(const std::string &name, 
                           const JSONValue::JSONArray &param, 
                           StatePersistence *persistence) {        
    TRACE(CL_LOG, "invoke");
    if (param.size() != 1 || 
        param[0].type() != typeid(JSONValue::JSONObject)) {
        throw JSONRPCInvocationException(
            "Method '" + name + "' requires one object parameter.");
    }
    try {
        JSONValue::JSONObject jsonObj = param[0].get<JSONValue::JSONObject>();
        JSONValue::JSONObject::const_iterator jsonObjIt;
        JSONValue::JSONObject::const_iterator addEnvIt, pathIt, commandIt;
        jsonObjIt = jsonObj.find(
            ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY);
        if (jsonObjIt == jsonObj.end()) {
            throw JSONRPCInvocationException("invoke: No notifyable!");
        }
        if (jsonObjIt->second.type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                string("invoke: ") + 
                ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY + 
                " is not a string");
        }
        ProcessSlot *processSlot = dynamic_cast<ProcessSlot *>(
            getRPCManager()->getRoot()->getNotifyableFromKey(
                jsonObjIt->second.get<JSONValue::JSONString>()));  
        if (processSlot == NULL) {
            throw JSONRPCInvocationException(
                string("invoke: No notifyable for " + 
                       jsonObjIt->second.get<JSONValue::JSONString>()));
        }
        uint32_t execArgsCount = 0;
        addEnvIt = jsonObj.find(ClusterlibStrings::JSONOBJECTKEY_ADDENV);
        if (addEnvIt != jsonObj.end()) {
            ++execArgsCount;
        }
        pathIt = jsonObj.find(ClusterlibStrings::JSONOBJECTKEY_PATH);
        if (pathIt != jsonObj.end()) {
            ++execArgsCount;
        }
        commandIt = jsonObj.find(ClusterlibStrings::JSONOBJECTKEY_COMMAND);
        if (commandIt != jsonObj.end()) {
            ++execArgsCount;
        }
        if ((execArgsCount != 0) && (execArgsCount != 3)) {
            LOG_ERROR(CL_LOG, 
                      "invoke: Tried to set %u args (should be 0 or 3)",
                      execArgsCount);
            throw JSONRPCInvocationException(
                "invoke: Exec args have problems");
        }

        /* 
         * Convert the JSONArray to an array of strings and setup the
         * executable arguments 
         */
        if (execArgsCount == 3) {
            vector<string> addEnv;
            JSONValue::JSONArray jsonAddEnv =   
                addEnvIt->second.get<JSONValue::JSONArray>();
            for (size_t i = 0; i < jsonAddEnv.size(); ++i) {
                addEnv.push_back(jsonAddEnv[i].get<JSONValue::JSONString>());
            }
            
            processSlot->setExecArgs(
                addEnv,
                pathIt->second.get<JSONValue::JSONString>(),
                commandIt->second.get<JSONValue::JSONString>());
        }

        processSlot->acquireLock();
        processSlot->start();
        processSlot->releaseLock();

        JSONValue::JSONObject retObj = jsonObj;
        retObj[ClusterlibStrings::JSONOBJECTKEY_METHOD] = 
            ClusterlibStrings::RPC_START_PROCESS;
        return retObj;
    } 
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CL_LOG, 
                 "invoke: Failed to finish command with error: %s",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

void
StartProcessMethod::unmarshalParams(const JSONValue::JSONArray &paramArr)
{
    TRACE(CL_LOG, "unmarshalParams");

    JSONValue::JSONObject paramObj = paramArr[0].get<JSONValue::JSONObject>();
    JSONValue::JSONObject::const_iterator paramObjIt = paramObj.find(
        ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY);
    setProcessSlotKey(paramObjIt->second.get<JSONValue::JSONString>());
    paramObjIt = paramObj.find(ClusterlibStrings::JSONOBJECTKEY_ADDENV);
    setAddEnv(paramObjIt->second.get<JSONValue::JSONArray>());
    paramObjIt = paramObj.find(ClusterlibStrings::JSONOBJECTKEY_PATH);
    setPath(paramObjIt->second.get<JSONValue::JSONString>());
    paramObjIt = paramObj.find(ClusterlibStrings::JSONOBJECTKEY_COMMAND);
    setCommand(paramObjIt->second.get<JSONValue::JSONString>());
}

JSONValue::JSONArray
StartProcessRequest::marshalParams()
{
    TRACE(CL_LOG, "marshalParams");

    JSONValue::JSONArray jsonArr;
    JSONValue::JSONObject jsonObj;
    if (getProcessSlotKey().type() == typeid(JSONValue::JSONNull)) {
        throw JSONRPCInvocationException(
            "marshalParams: No process slot key");
    }
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY] = 
        getProcessSlotKey();
    if (getAddEnv().type() == typeid(JSONValue::JSONNull)) {
        throw JSONRPCInvocationException("marshalParams: No add env");
    }
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV] = getAddEnv();
    if (getPath().type() == typeid(JSONValue::JSONNull)) {
        throw JSONRPCInvocationException("marshalParams: No path");
    }
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH] = getPath();
    if (getCommand().type() == typeid(JSONValue::JSONNull)) {
        throw JSONRPCInvocationException("marshalParams: No command");
    }
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND] = getCommand();
    
    jsonArr.push_back(jsonObj);
    return jsonArr;
}

}
