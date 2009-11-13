#include "jsonrpc.h"

using namespace std;
using namespace log4cxx;

namespace json { namespace rpc {
    LoggerPtr JSONRPCManager::logger(Logger::getLogger("json.rpc.JSONRPCManager"));

    JSONRPCInvocationException::JSONRPCInvocationException(const string &message) : JSONException(message) {
    }

    auto_ptr<JSONRPCManager> JSONRPCManager::singleton;

    JSONRPCManager *JSONRPCManager::getInstance() {
        if (!singleton.get()) {
            singleton.reset(new JSONRPCManager());
        }

        return singleton.get();
    }

    JSONRPCManager::JSONRPCManager() {
    }

    JSONRPCManager::~JSONRPCManager() {
    }

    bool JSONRPCManager::registerMethod(const string &name, JSONRPCMethod *method) {
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

    void JSONRPCManager::clearMethods() {
        rpcMethods.clear();
    }

    vector<string> JSONRPCManager::getMethodNames() {
        vector<string> methodNames;

        for (RPCMethodMap::const_iterator iter = rpcMethods.begin(); iter != rpcMethods.end(); ++iter) {
            methodNames.push_back(iter->first);
        }

        return methodNames;
    }

    JSONValue JSONRPCManager::generateErrorResponse(const string &message, const JSONValue &id) {
        JSONValue::JSONObject obj;
        obj["result"] = JSONValue::Null;
        obj["error"] = message;
        obj["id"] = id;
        LOG4CXX_WARN(logger, "Error invoking JSON-RPC method(" << message << ")");
        return obj;
    }

    JSONValue JSONRPCManager::generateResponse(const JSONValue &ret, const JSONValue &id) {
        JSONValue::JSONObject obj;
        obj["result"] = ret;
        obj["error"] = JSONValue::Null;
        obj["id"] = id;
        return obj;
    }

    JSONValue JSONRPCManager::invoke(const JSONValue &rpcInvocation, StatePersistence *persistence) const {
        string method;
        JSONValue::JSONArray params;
        JSONValue id;
        try {
            JSONValue::JSONObject rpcObj = rpcInvocation.get<JSONValue::JSONObject>();
            if (rpcObj.size() != 3 || rpcObj.find("method") == rpcObj.end() || rpcObj.find("params") == rpcObj.end() || rpcObj.find("id") == rpcObj.end()) {
                return generateErrorResponse("Invalid JSON-RPC object, attribute missing or extra property found.", id);
            }
            id = rpcObj["id"];
            method = rpcObj["method"].get<JSONValue::JSONString>();
            params = rpcObj["params"].get<JSONValue::JSONArray>();
        } catch (JSONValueException &ex) {
            return generateErrorResponse(string("Invalid JSON-RPC object with unexpected type.\n") + ex.what(), id);
        }

        RPCMethodMap::const_iterator methodIter = rpcMethods.find(method);
        if (methodIter == rpcMethods.end()) {
            // Method not found
            return generateErrorResponse(string("RPC method '") + method + "' is not found.", id);
        }

        LOG4CXX_INFO(logger, "Invoking JSON-RPC method (" << method << ")");

        try {
            // Call the method
            return generateResponse(methodIter->second->invoke(method, params, persistence), id);
            LOG4CXX_INFO(logger, "JSON-PRC invocation succeeded (" << method << ")");
        } catch (JSONException &ex) {
            // Error occurred when invoking the method
            return generateErrorResponse(string("Error occurred when invoking the method.\n") + ex.what(), id);
        }
    }
}}
