#include "clusterlibinternal.h"

using namespace std;
using namespace log4cxx;
using namespace clusterlib;

DEFINE_LOGGER(JRPC_LOG, "json.rpc");

namespace json { namespace rpc {

JSONRPCManager::~JSONRPCManager() 
{
}

Mutex *
JSONRPCManager::getLock()
{
    TRACE(JRPC_LOG, "getLock");
    
    return &m_lock;
}

bool 
JSONRPCManager::registerMethod(const string &name, JSONRPCMethod *method) 
{
    Locker l(getLock());

    if (m_rpcMethods.find(name) != m_rpcMethods.end()) {
        return false;
    }

    /* 
     * If this is a ClusterlibRPCManager and the method is a
     * ClusterlibRPCMethod, then make sure to set the rpc manager pointer.
     */
    ClusterlibRPCManager *clusterlibRpcManager = 
        dynamic_cast<ClusterlibRPCManager *>(this);
    ClusterlibRPCMethod *clusterlibRpcMethod = 
        dynamic_cast<ClusterlibRPCMethod *>(method);
    if (clusterlibRpcManager && clusterlibRpcMethod) {
        clusterlibRpcMethod->setRPCManager(clusterlibRpcManager);
    }

    m_rpcMethods.insert(make_pair(name, method));
    return true;
}

bool JSONRPCManager::unregisterMethod(const string &name) 
{
    Locker l(getLock());

    if (m_rpcMethods.find(name) == m_rpcMethods.end()) {
        return false;
    }
    
    m_rpcMethods.erase(name);
    return true;
}

void 
JSONRPCManager::clearMethods() 
{
    Locker l(getLock());

    m_rpcMethods.clear();
}

vector<string> 
JSONRPCManager::getMethodNames() 
{
    Locker l(getLock());

    vector<string> methodNames;    
    for (RPCMethodMap::const_iterator iter = m_rpcMethods.begin(); 
         iter != m_rpcMethods.end(); 
         ++iter) {
        methodNames.push_back(iter->first);
    }
    
    return methodNames;
}

JSONValue 
JSONRPCManager::generateErrorResponse(const string &message, 
                                      const JSONValue &id) 
{
    JSONValue::JSONObject obj;
    obj["result"] = JSONValue::Null;
    obj["error"] = message;
    obj["id"] = id;
    LOG_WARN(JRPC_LOG, "Error invoking JSON-RPC method (%s)", message.c_str());
    return obj;
}

JSONValue 
JSONRPCManager::generateResponse(const JSONValue &ret, const JSONValue &id) 
{
    JSONValue::JSONObject obj;
    obj["result"] = ret;
    obj["error"] = JSONValue::Null;
    obj["id"] = id;
    return obj;
}

JSONValue 
JSONRPCManager::invoke(const JSONValue &rpcInvocation, 
                       StatePersistence *persistence)
{
    string method;
    JSONValue::JSONArray params;
    JSONValue id;
    try {
        JSONValue::JSONObject rpcObj = 
            rpcInvocation.get<JSONValue::JSONObject>();
        if (rpcObj.size() != 3 || 
            rpcObj.find("method") == rpcObj.end() || 
            rpcObj.find("params") == rpcObj.end() || 
            rpcObj.find("id") == rpcObj.end()) {
            return generateErrorResponse(
                "Invalid JSON-RPC object, attribute missing or "
                "extra property found.", id);
        }
        id = rpcObj["id"];
        method = rpcObj["method"].get<JSONValue::JSONString>();
        params = rpcObj["params"].get<JSONValue::JSONArray>();
    } 
    catch (JSONValueException &ex) {
        return generateErrorResponse(
            string("Invalid JSON-RPC object with unexpected type.\n") + 
            ex.what(), id);
    }

    Locker l(getLock());

    RPCMethodMap::const_iterator methodIter = m_rpcMethods.find(method);
    if (methodIter == m_rpcMethods.end()) {
        // Method not found
        return generateErrorResponse(
            string("RPC method '") + method + "' is not found.", id);
    }

    LOG_INFO(JRPC_LOG, "Invoking JSON-RPC method (%s)", method.c_str());

    try {
        /* Check the expected params */
        methodIter->second->checkParams(params);

        /*
         * Unmarshal if a ClusterlibRPCMethod and clear the
         * user-defined status 
         */
        ClusterlibRPCMethod *clusterlibRpcMethod = 
            dynamic_cast<ClusterlibRPCMethod *>(methodIter->second);
        if (clusterlibRpcMethod) {
            clusterlibRpcMethod->unmarshalParams(params);
            clusterlibRpcMethod->setMethodStatus(string(), 0, 0);
        }

        /* Call the appropriate method */
        return generateResponse(
            methodIter->second->invoke(method, params, persistence), id);
        LOG_INFO(JRPC_LOG,
                 "JSON-PRC invocation succeeded (%s)"
                 ,method.c_str());
    } 
    catch (Exception &ex) {
        /* Error occurred when invoking the method */
        return generateErrorResponse(
            string("Error occurred when invoking the method.\n") + ex.what(),
            id);
    }

    /* Shouldn't reach here. */
    return JSONValue();
}

}}
