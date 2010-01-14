/* 
 * stopprocessmethod.cc --
 *
 * Implementation of the StopProcessMethod class.

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

string
StopProcessRPC::getName()
{
    return ClusterlibStrings::RPC_STOP_PROCESS;
}

bool
StopProcessRPC::checkParams(const JSONValue::JSONArray &paramArr)
{
    try {
        if (paramArr.size() != 1) {
            LOG_ERROR(CL_LOG,
                      "checkParams: Expecting one array element, got %d",
                      paramArr.size());
            return false;
        }
        JSONValue::JSONObject paramObj = 
            paramArr[0].get<JSONValue::JSONObject>();
        JSONValue::JSONObject::const_iterator paramObjIt = paramObj.find(
            ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY);
        if (paramObjIt == paramObj.end()) {
            throw JSONRPCInvocationException("checkParams: No notifyable!");
        }

        return true;
    }
    catch (const JSONRPCInvocationException &ex) {
        LOG_WARN(CL_LOG, "checkParams: Failed with %s", ex.what());
        return false;
    }
}

StopProcessMethod::StopProcessMethod(Client *client) 
    : m_client(client) 
{
    m_root = m_client->getRoot();
}

JSONValue 
StopProcessMethod::invoke(const std::string &name, 
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
            m_root->getNotifyableFromKey(
                jsonObjIt->second.get<JSONValue::JSONString>()));  
        if (processSlot == NULL) {
            throw JSONRPCInvocationException(
                string("invoke: No notifyable for " + 
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
                 "invoke: Failed to finish command with error: %s",
                 ex.what());
        throw JSONRPCInvocationException(ex.what());
    }
}

}