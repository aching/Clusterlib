#ifndef _CL_ZOOKEEPERUISERVER_H_
#define _CL_ZOOKEEPERUISERVER_H_

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

namespace zookeeper { namespace ui {

class ZooKeeperUIServer :
    public virtual httpd::HttpServerAccessHandler,
    public virtual httpd::HttpServerIncludeHandler 
{
  public:
    /**
     * Get the singleton.
     * 
     * @return Pointer to the singleton.
     */
    static ZooKeeperUIServer *getInstance();

    /**
     * Shut down the service and clean up the memory.
     */
    static void destroyInstance();

    /**
     * Start the httpd server.
     */
    void start();

    /**
     * Stop the httpd server.
     */
    void stop();

    /**
     * Parse the command line arguments.
     *
     * @param argc Number of arguments
     * @param argv Argument array.
     */
    void parseArgs(int argc, const char *const*argv);

    /**
     * Print the gInkBuildStamp to stdout.
     */
    void printVersion();

    /**
     * Print the usage to stdout.
     */
    void printUsage();

    /**
     * Check for which hosts can access this service.
     *
     * @param context Client context.
     */
    AccessPermission canAccess(const httpd::HttpContext &context);

    /**
     * Set the response body of the context.
     *
     * @param reference Reference string
     * @param context HttpContext to modify.
     */
    void includeHandler(const std::string &reference, 
                        httpd::HttpContext *context);

    /**
     * Access the Clusterlib Factory
     *
     * @return a pointer to the Clusterlib Factory
     */
    clusterlib::Factory *getClusterlibFactory();

  private:
    /**
     * Private constructor.
     */
    ZooKeeperUIServer();
    
    /**
     * Private destructor.
     */
    ~ZooKeeperUIServer();

  private:
    /**
     * Pointer to the only instantiation.
     */
    static ZooKeeperUIServer *m_singleton;
    
    /**
     * Hosts that are allowed access.
     */
    std::auto_ptr<boost::regex> m_allowedHosts;

    /**
     * Configuration.
     */
    configurator::Configuration m_config;

    /**
     * Manages RPC requests.
     */
    std::auto_ptr<json::rpc::JSONRPCManager> m_rpcManager;

    /**
     * Httpd server.
     */
    std::auto_ptr<httpd::HttpServer> m_httpd;

    /**
     * Allows the httpd server and rpcManager to work together.
     */
    std::auto_ptr<json::rpc::HttpServerAdaptor> m_adaptor;

    /**
     * Can handle Clusterlib RPC requests.
     */
    std::auto_ptr<clusterlib::rpc::json::MethodAdaptor> m_clusterRpcMethod;

    /**
     * Can handle Zookeeper RPC requests.
     */
    std::auto_ptr<zookeeper::rpc::json::MethodAdaptor> m_zookeeperRpcMethod;

    /**
     * Monitors the Zookeeper cluster that it is connected to.
     */
    std::auto_ptr<clusterlib::Periodic> m_zookeeperPeriodicCheck;

    /**
     * Clusterlib Factory pointer.
     */
    std::auto_ptr<clusterlib::Factory> m_clusterFactory;
};

}}

#endif
