#ifndef _INCLUDED_ZOOKEEPER_JSONRPC_ADAPTOR_H_
#define _INCLUDED_ZOOKEEPER_JSONRPC_ADAPTOR_H_
#include "jsonrpc_httpd_adaptor.h"
extern "C" {
#include <c-client-src/zookeeper.h>
}
#include <log4cxx/logger.h>
#include <pthread.h>

namespace zookeeper { namespace rpc { namespace json {
    const int32_t SESSION_TIMEOUT = 30;
    const int32_t CONNECTION_TIMEOUT = 5;

    class MethodAdaptor : public virtual ::json::rpc::JSONRPCMethod {
    public:
        MethodAdaptor(const std::string &servers);
        ::json::JSONValue invoke(const std::string &name, const ::json::JSONValue::JSONArray &param, ::json::rpc::StatePersistence *persistence);
        virtual ~MethodAdaptor();
    private:
        static log4cxx::LoggerPtr logger;
        std::string servers;
        zhandle_t *zkHandle;
        pthread_cond_t cond;
        pthread_mutex_t mutex;
        int32_t connectionState;
        void reconnect();
        static void staticGlobalWatcher(zhandle_t *zkHandle, int type, int state, const char *path, void *context);
        void globalWatcher(int type, int state, const char *path);
        ::json::JSONValue::JSONBoolean zooExists(::json::JSONValue::JSONString path);
        ::json::JSONValue::JSONString zooGet(::json::JSONValue::JSONString path);
        ::json::JSONValue::JSONString zooSet(
            ::json::JSONValue::JSONString path,
            ::json::JSONValue::JSONString data);
        ::json::JSONValue::JSONArray zooGetChildren(::json::JSONValue::JSONString path);
    };
}}}
#endif
