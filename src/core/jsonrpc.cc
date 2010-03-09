#include "clusterlibinternal.h"

using namespace std;
using namespace log4cxx;
using namespace clusterlib;

DEFINE_LOGGER(JRPC_LOG, "json.rpc");

namespace json { namespace rpc {

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
        /* Check and get the expected params */
        methodIter->second->checkInitParams(params, true);
        /* Call the appropriate method */
        return generateResponse(
            methodIter->second->invoke(method, params, persistence), id);
        LOG_INFO(JRPC_LOG,
                 "JSON-PRC invocation succeeded (%s)"
                 ,method.c_str());
    } catch (Exception &ex) {
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
                              int32_t defaultCompletedQueueMaxSize,
                              PropertyList *methodStatusPropertyList,
                              StatePersistence *persistence) const
{
    TRACE(JRPC_LOG, "invokeAndResp");
    
    JSONValue jsonInput, jsonResult;
    JSONValue::JSONObject inputObj;
    JSONValue::JSONObject::const_iterator jsonInputIt;
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
        const JSONValue::JSONArray &paramArr = 
            inputObj["params"].get<JSONValue::JSONArray>();
        if (paramArr.size() == 0) {
            LOG_WARN(JRPC_LOG, 
                     "invokeAndResp: No params for the request, so putting "
                     "result in default completed queue (%s)",
                     defaultCompletedQueue->getKey().c_str());
            defaultCompletedQueue->put(result);
            return;
        }
        const JSONValue::JSONObject &paramObj = 
            paramArr[0].get<JSONValue::JSONObject>();
        jsonInputIt = paramObj.find(
            ClusterlibStrings::JSONOBJECTKEY_RESPQUEUEKEY);
        if (jsonInputIt != paramObj.end()) {
            string respQueueKey = 
            jsonInputIt->second.get<JSONValue::JSONString>();
            Queue *respQueue = dynamic_cast<Queue *>(
                root->getNotifyableFromKey(respQueueKey));
            if (respQueue != NULL) {
                respQueue->put(result);
                /*
                 * Also add to the completed queue if the
                 * defaultCompletedQueue if > 0
                 */
                if (defaultCompletedQueueMaxSize > 0) {
                    defaultCompletedQueue->put(result);
                }
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

        /* 
         * Try to make sure that the defaultCompletedQueue size is not
         * exceeded 
         */
        const int32_t maxRetries = 3;
        int32_t retriesUsed = 0;
        string tmpElement;
        while (retriesUsed < maxRetries) {
            if (defaultCompletedQueue->acquireLockWaitMsecs(100)) {
                while (defaultCompletedQueue->size() > 
                       defaultCompletedQueueMaxSize) {
                    defaultCompletedQueue->take(tmpElement);
                }
                break;
            }
            LOG_WARN(JRPC_LOG,
                     "invokeAndResp: Failed to get the lock with retriesUsed ="
                     " %d (max %d)",
                     retriesUsed,
                     maxRetries);
            ++retriesUsed;
        }
    }
    catch (const Exception &ex) {
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
