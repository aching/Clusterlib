#ifndef _INCLUDED_MAIN_H_
#define _INCLUDED_MAIN_H_
#include <map>
#include <memory>
#include <string>
#include <log4cxx/logger.h>
#include <boost/regex.hpp>
#include <clusterlib.h>
#include "xmlconfig.h"
#include "httpd.h"
#include "jsonrpc_httpd_adaptor.h"
#include "clusterlib_jsonrpc_adaptor.h"
#include "zookeeper_jsonrpc_adaptor.h"

extern const char *gInkBuildStamp;
extern const char *appName;

namespace zookeeper { namespace ui {
    class ZooKeeperUIServer :
        public virtual httpd::HttpServerAccessHandler,
        public virtual httpd::HttpServerIncludeHandler {
    public:
        static ZooKeeperUIServer *getInstance();
        static void destroyInstance();
        void start();
        void stop();
        void parseArgs(int argc, const char *const*argv);
        void printVersion();
        void printUsage();
        AccessPermission canAccess(const httpd::HttpContext &context);
        void includeHandler(const std::string &reference, httpd::HttpContext *context);
    private:
        static ZooKeeperUIServer *singleton;
        static log4cxx::LoggerPtr logger;
        std::auto_ptr<boost::regex> allowedHosts;
        ZooKeeperUIServer();
        ~ZooKeeperUIServer();
        configurator::Configuration config;
        std::auto_ptr<json::rpc::JSONRPCManager> rpcManager;
        std::auto_ptr<httpd::HttpServer> httpd;
        std::auto_ptr<json::rpc::HttpServerAdaptor> adaptor;
        std::auto_ptr<clusterlib::rpc::json::MethodAdaptor> clusterRpcMethod;
        std::auto_ptr<zookeeper::rpc::json::MethodAdaptor> zookeeperRpcMethod;
        std::auto_ptr<clusterlib::Factory> clusterFactory;
    };
}}
#endif
