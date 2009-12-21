#include "clusterlibinternal.h"
#include "jsonrpc.h"

using namespace std;
using namespace log4cxx;
using namespace clusterlib;

DEFINE_LOGGER(JRPC_LOG, "json.rpc");

namespace json { namespace rpc {

JSONRPCInvocationException::JSONRPCInvocationException(
    const string &message) : JSONException(message) {
}

auto_ptr<JSONRPCManager> JSONRPCManager::singleton;
    
JSONRPCManager *JSONRPCManager::getInstance() {
    if (!singleton.get()) {
        singleton.reset(new JSONRPCManager());
    }
    
    return singleton.get();
}

JSONRPCManager::JSONRPCManager() 
{
}

JSONRPCManager::~JSONRPCManager() 
{
}

bool 
JSONRPCManager::registerMethod(const string &name, JSONRPCMethod *method) {
    if (rpcMethods.find(name) != rpcMethods.end()) {
        return false;
    }
    
    rpcMethods.insert(make_pair(name, method));
    return true;
}

bool JSONRPCManager::unregisterMethod(const string &name) {
    if (rpcMethods.find(name) == rpcMethods.end()) {
        return false;
    }
    
    rpcMethods.erase(name);
    return true;
}

void 
JSONRPCManager::clearMethods() {
    rpcMethods.clear();
}

vector<string> 
JSONRPCManager::getMethodNames() {
    vector<string> methodNames;
    
    for (RPCMethodMap::const_iterator iter = rpcMethods.begin(); 
         iter != rpcMethods.end(); 
         ++iter) {
        methodNames.push_back(iter->first);
    }
    
    return methodNames;
}

JSONValue 
JSONRPCManager::generateErrorResponse(const string &message, 
                                      const JSONValue &id) {
    JSONValue::JSONObject obj;
    obj["result"] = JSONValue::Null;
    obj["error"] = message;
    obj["id"] = id;
    LOG_WARN(JRPC_LOG, "Error invoking JSON-RPC method (%s)", message.c_str());
    return obj;
}

JSONValue 
JSONRPCManager::generateResponse(const JSONValue &ret, const JSONValue &id) {
    JSONValue::JSONObject obj;
    obj["result"] = ret;
    obj["error"] = JSONValue::Null;
    obj["id"] = id;
    return obj;
}

JSONValue 
JSONRPCManager::invoke(const JSONValue &rpcInvocation, 
                       StatePersistence *persistence) const {
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
    } catch (JSONValueException &ex) {
        return generateErrorResponse(
            string("Invalid JSON-RPC object with unexpected type.\n") + 
            ex.what(), id);
    }

    RPCMethodMap::const_iterator methodIter = rpcMethods.find(method);
    if (methodIter == rpcMethods.end()) {
        // Method not found
        return generateErrorResponse(
            string("RPC method '") + method + "' is not found.", id);
    }

    LOG_INFO(JRPC_LOG, "Invoking JSON-RPC method (%s)", method.c_str());

    try {
        // Call the method
        return generateResponse(
            methodIter->second->invoke(method, params, persistence), id);
        LOG_INFO(JRPC_LOG,
                 "JSON-PRC invocation succeeded (%s)"
                 ,method.c_str());
    } catch (JSONException &ex) {
        // Error occurred when invoking the method
        return generateErrorResponse(
            string("Error occurred when invoking the method.\n") + ex.what(),
            id);
    }
}

void
JSONRPCManager::invokeAndResp(const string &rpcInvocation,
                              Root *root,
                              Queue *defaultCompletedQueue,
                              StatePersistence *persistence) const
{
    TRACE(JRPC_LOG, "invokeAndResp");
    
    JSONValue jsonInput, jsonResult;
    JSONValue::JSONObject inputObj;
    JSONValue::JSONObject::iterator jsonInputIt;
    string result;
    try {
        jsonInput = JSONCodec::decode(rpcInvocation);
        jsonResult = invoke(jsonInput, persistence);
        result = JSONCodec::encode(jsonResult);
        LOG_DEBUG(JRPC_LOG, 
                  "invokeAndResp: Invoked on input (%s) and returned (%s)",
                  rpcInvocation.c_str(),
                  result.c_str());
        inputObj = jsonInput.get<JSONValue::JSONObject>();
        jsonInputIt = inputObj.find(ClusterlibStrings::DEFAULT_RESP_QUEUE);
        if (jsonInputIt != inputObj.end()) {
            string respQueueKey = 
            jsonInputIt->second.get<JSONValue::JSONString>();
            Queue *respQueue = dynamic_cast<Queue *>(
                root->getNotifyableFromKey(respQueueKey));
            if (respQueue != NULL) {
                respQueue->put(result);
            }
            else {
                LOG_WARN(JRPC_LOG,
                         "invokeAndResp: Tried to put result in user "
                         "selected queue (%s) and failed, so putting "
                         "result in default completed queue (%s)",
                         respQueueKey.c_str(),
                         defaultCompletedQueue->getKey().c_str());
                defaultCompletedQueue->put(result);
            }
        }
        else {
            defaultCompletedQueue->put(result);
        }
    }
    catch (const JSONParseException &ex) {
        JSONValue::JSONObject jsonObject;
        string queueElement = JSONCodec::encode(jsonObject);
        LOG_WARN(CL_LOG,
                 "invokeAndResp: Couldn't parse or service command (%s) "
                 "and adding element (%s) to the DEFAULT_COMPLETED_QUEUE",
                  JSONCodec::encode(rpcInvocation).c_str(),
                 queueElement.c_str());
        defaultCompletedQueue->put(queueElement);
    }
}

}}
