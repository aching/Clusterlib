#include "gui.h"
#include "zookeeper_jsonrpc_adaptor.h"
#include <time.h>

using namespace json;
using namespace json::rpc;
using namespace log4cxx;
using namespace std;

namespace zookeeper { namespace rpc { namespace json {
    LoggerPtr MethodAdaptor::logger(
        Logger::getLogger("zookeeper.rpc.json.MethodAdaptor"));

const std::string &
MethodAdaptor::getName() const
{
    return name;
}

void
MethodAdaptor::checkParams(const JSONValue::JSONArray &paramArr)
{
    JSONCodec::encode(paramArr);
}

MethodAdaptor::MethodAdaptor(const string &s) 
    : servers(s),
      name("zookeeper::rpc::json::MethodAdaptor")
{
    if (pthread_cond_init(&cond, NULL)) {
        throw JSONRPCInvocationException("Cannot create a condition signal.");
    }
    if (pthread_mutex_init(&mutex, NULL)) {
        pthread_cond_destroy(&cond);
        throw JSONRPCInvocationException("Cannot create a mutex.");
        }
    connectionState = ZOO_EXPIRED_SESSION_STATE;
    zkHandle = NULL;
    reconnect();
}

MethodAdaptor::~MethodAdaptor() {
    zookeeper_close(zkHandle);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
    
void MethodAdaptor::reconnect() {
    if (connectionState == ZOO_CONNECTED_STATE) {
        // Why reconnect?
        return;
    }
    if (zkHandle != NULL) {
        // If we have previous handle, close it first.
        LOG4CXX_INFO(logger, "Close existing ZooKeeper connection");
        zookeeper_close(zkHandle);
        zkHandle = NULL;
    }
    
    // Connect asynchronously
    LOG4CXX_INFO(logger, "Establishing ZooKeeper connection");
    zkHandle = zookeeper_init(servers.c_str(), 
                              staticGlobalWatcher, 
                              SESSION_TIMEOUT * 1000, 
                              NULL, 
                              this, 
                              0);
    if (!zkHandle) {
        throw JSONRPCInvocationException("Cannot create a zookeeper handle.");
    }
    
    int state;
    pthread_mutex_lock(&mutex);
    state = connectionState;
    
    if (state != ZOO_CONNECTED_STATE) {
        // Wait for the connection
        timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec += CONNECTION_TIMEOUT;
        pthread_cond_timedwait(&cond, &mutex, &time);
        state = connectionState;
    }
    pthread_mutex_unlock(&mutex);
    
    if (state != ZOO_CONNECTED_STATE) {
        LOG4CXX_ERROR(logger, "Failed to establish ZooKeeper connection");
        throw JSONRPCInvocationException("ZooKeeper server connection cannot be established (timed-out).");
        }
    
    LOG4CXX_INFO(logger, "Established ZooKeeper connection");
}
    
void
MethodAdaptor::staticGlobalWatcher(zhandle_t *zkHandle, 
                                   int type, 
                                   int state, 
                                   const char *path, 
                                   void *context) {
    reinterpret_cast<MethodAdaptor *>(
        context)->globalWatcher(type, state, path);
}

void MethodAdaptor::globalWatcher(int type, int state, const char *path) {
    if (type == ZOO_SESSION_EVENT ) {
        LOG4CXX_INFO(logger, "Switching ZooKeeper connection state from " << connectionState << " to " << state);
        pthread_mutex_lock(&mutex);
        connectionState = state;
        if (connectionState == ZOO_CONNECTED_STATE) {
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
}

JSONValue
MethodAdaptor::invoke(const std::string &name, 
                      const JSONValue::JSONArray &param, 
                      StatePersistence *persistence) {
    if (name == "zoo_exists") {
        if (param.size() != 1 || param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException("Method '" + name + "' requires one string parameter.");
        }
        return zooExists(param[0].get<JSONValue::JSONString>());
    } else if (name == "zoo_get") {
        if (param.size() != 1 || param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException("Method '" + name + "' requires one string parameter.");
        }
        return zooGet(param[0].get<JSONValue::JSONString>());
    } else if (name == "zoo_set") {
        if (param.size() != 2 || param[0].type() != typeid(JSONValue::JSONString) || param[1].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException("Method '" + name + "' requires two string parameters.");
        }
        return zooSet(param[0].get<JSONValue::JSONString>(),
                      param[1].get<JSONValue::JSONString>());
    } else if (name == "zoo_get_children") {
        if (param.size() != 1 || param[0].type() != typeid(JSONValue::JSONString)) {
            throw JSONRPCInvocationException("Method '" + name + "' requires one string parameter.");
        }
        return zooGetChildren(param[0].get<JSONValue::JSONString>());
    } else {
        throw JSONRPCInvocationException("Unknown method '" + name + "' invoked.");
    }
}

JSONValue::JSONBoolean
MethodAdaptor::zooExists(JSONValue::JSONString path) {
    int ret;
    do {
        ret = zoo_exists(zkHandle, path.c_str(), 0, NULL);
        if (ret != ZINVALIDSTATE) break;
        reconnect();
    } while (true);
    
    switch (ret) {
        case ZOK:
            return true;
        case ZNONODE:
            return false;
        default:
            throw JSONRPCInvocationException(string("Error occurred in ZooKeeper (") + zerror(ret) + ")");
    }
}
    
JSONValue::JSONString
MethodAdaptor::zooGet(JSONValue::JSONString path) {
    char buffer[1024*1024];
    int bufLen = 1024*1024;
    int ret;
    do {
        ret = zoo_get(zkHandle, path.c_str(), 0, buffer, &bufLen, NULL);
        if (ret != ZINVALIDSTATE) break;
        reconnect();
        } while (true);
    
    switch (ret) {
        case ZOK:
            return string(buffer, 0, bufLen);
        default:
            throw JSONRPCInvocationException(string("Error occurred in ZooKeeper (") + zerror(ret) + ")");
    }
}
    
JSONValue::JSONString
MethodAdaptor::zooSet(JSONValue::JSONString path,
                      JSONValue::JSONString data) {
    int ret;
    do {
        ret = zoo_set(zkHandle, path.c_str(), data.c_str(), data.length(), -1);
        if (ret != ZINVALIDSTATE) break;
        reconnect();
    } while (true);
    
    switch (ret) {
        case ZOK:
            return string("0");
        default:
            throw JSONRPCInvocationException(
                string("Error occurred in ZooKeeper (") + zerror(ret) + ")");
    }
}

JSONValue::JSONArray
MethodAdaptor::zooGetChildren(JSONValue::JSONString path) {
    String_vector children;
    int ret;
    do {
        ret = zoo_get_children(zkHandle, path.c_str(), 0, &children);
        if (ret != ZINVALIDSTATE) break;
        reconnect();
    } while (true);
    
    JSONValue::JSONArray array;
    
    switch (ret) {
        case ZOK:
            for (int i = 0; i < children.count; ++i) {
                array.push_back(string(children.data[i]));
            }
            deallocate_String_vector(&children);
            return array;
        default:
            throw JSONRPCInvocationException(string("Error occurred in ZooKeeper (") + zerror(ret) + ")");
    }
}

}}}
