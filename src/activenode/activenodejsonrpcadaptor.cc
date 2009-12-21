/* 
 * activenodejsonrpcadaptor.cc --
 *
 * Implementation of the ActiveNodeJSONRPCAdaptor class.

 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"
#include "activenodejsonrpcadaptor.h"

using namespace clusterlib;
using namespace json;
using namespace json::rpc;
using namespace log4cxx;
using namespace std;

ActiveNodeJSONRPCAdaptor::ActiveNodeJSONRPCAdaptor(clusterlib::Client *client) 
    : m_client(client) 
{
    m_root = m_client->getRoot();
}

JSONValue 
ActiveNodeJSONRPCAdaptor::invoke(const std::string &name, 
                                 const JSONValue::JSONArray &param, 
                                 StatePersistence *persistence) {        
    if (name == ClusterlibStrings::RPC_START_PROCESS) {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one object parameter.");
        }
        return startProcess(param[0].get<JSONValue::JSONObject>());
    } 
    else if (name == ClusterlibStrings::RPC_STOP_PROCESS) {
        if (param.size() != 1 || 
            param[0].type() != typeid(JSONValue::JSONObject)) {
            throw JSONRPCInvocationException(
                "Method '" + name + "' requires one object parameter.");
        }
        return stopProcess(param[0].get<JSONValue::JSONObject>());
    } 
    else {
        throw JSONRPCInvocationException(
            "Unknown method '" + name + "' invoked.");
    }
}

JSONValue::JSONObject
ActiveNodeJSONRPCAdaptor::startProcess(
    const JSONValue::JSONObject &jsonObj) {
    TRACE(CL_LOG, "startProcess");
    try {
        JSONValue::JSONObject::const_iterator jsonObjIt;
        JSONValue::JSONObject::const_iterator addEnvIt, pathIt, commandIt;
        jsonObjIt = jsonObj.find(
            ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY);
        if (jsonObjIt == jsonObj.end()) {
            throw JSONRPCInvocationException("startProcess: No notifyable!");
        }
        if (jsonObjIt->second.type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                string("startProcess: ") + 
                ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY + 
                " is not a string");
        }
        ProcessSlot *processSlot = dynamic_cast<ProcessSlot *>(
            m_root->getNotifyableFromKey(
                jsonObjIt->second.get<JSONValue::JSONString>()));  
        if (processSlot == NULL) {
            throw JSONRPCInvocationException(
                string("startProcess: No notifyable for " + 
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
                      "startProcess: Tried to set %u args (should be 0 or 3)",
                      execArgsCount);
            throw JSONRPCInvocationException(
                "startProcess: Exec args have problems");
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

        processSlot->start();

        JSONValue::JSONObject retObj = jsonObj;
        retObj[ClusterlibStrings::JSONOBJECTKEY_METHOD] = 
            ClusterlibStrings::RPC_START_PROCESS;
        return retObj;
    } 
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CL_LOG, 
                 "startProcess: Failed to finish command with error: %s",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

JSONValue::JSONObject
ActiveNodeJSONRPCAdaptor::stopProcess(
    const JSONValue::JSONObject &jsonObj) {
    TRACE(CL_LOG, "stopProcess");
    try {
        JSONValue::JSONObject::const_iterator jsonObjIt;
        jsonObjIt = jsonObj.find(
            ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY);
        if (jsonObjIt == jsonObj.end()) {
            throw JSONRPCInvocationException("stopProcess: No notifyable!");
        }
        if (jsonObjIt->second.type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException(
                string("stopProcess: ") + 
                ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY + 
                " is not a string");
        }
        ProcessSlot *processSlot = dynamic_cast<ProcessSlot *>(
            m_root->getNotifyableFromKey(
                jsonObjIt->second.get<JSONValue::JSONString>()));  
        if (processSlot == NULL) {
            throw JSONRPCInvocationException(
                string("stopProcess: No notifyable for " + 
                       jsonObjIt->second.get<JSONValue::JSONString>()));
        }
        JSONValue::JSONObject::const_iterator signalIt;
        signalIt = jsonObj.find(ClusterlibStrings::JSONOBJECTKEY_SIGNAL);
        if (signalIt != jsonObj.end()) {
            processSlot->stop(signalIt->second.get<JSONValue::JSONInteger>());
        }
        else {
            processSlot->stop();
        }

        JSONValue::JSONObject retObj = jsonObj;
        retObj[ClusterlibStrings::JSONOBJECTKEY_METHOD] = 
            ClusterlibStrings::RPC_STOP_PROCESS;
        return retObj;
    }
    catch (const ::clusterlib::Exception &ex) {
        LOG_WARN(CL_LOG, 
                 "stopProcess: Failed to finish command with error: %s",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}
