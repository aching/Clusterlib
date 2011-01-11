/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef _CL_ZOOKEEPERUISERVER_H_
#define _CL_ZOOKEEPERUISERVER_H_

#include <map>
#include <memory>
#include <string>
#include <boost/regex.hpp>
#include <clusterlib.h>
#include "xmlconfig.h"
#include "httpd.h"
#include "jsonrpc_httpd_adaptor.h"
#include "clusterlib_jsonrpc_adaptor.h"
#include "zookeeper_jsonrpc_adaptor.h"

DEFINE_LOGGER(ZUI_LOG, "zookeeper.ui.ZooKeeperUIServer");

namespace zookeeper { 

namespace ui {

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
     * Initialize the http server, zookeeper, clusterlib, etc.
     * Register all JSON-RPCs and include handlers.
     */
    void init();

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
     * Changes directives that have been registered already.  Set the
     * response body of the context.
     *
     * @param reference Reference string
     * @param context HttpContext to modify.
     */
    void includeHandler(const std::string &reference, 
                        httpd::HttpContext *context);

    /**
     * Adds a user-defined directive to be replaced in the html
     * content.  Users should use this function to setup directives
     * prior to parsing the arguments (parseArgs is called).
     *
     * @param key Directive to replace
     * @param value Replaced text
     */
    void addDirective(const std::string &key,
                      const std::string &value);

    /**
     * Access the Clusterlib Factory.  Be careful when doing this as
     * it is also being used by an RPC handler.
     *
     * @return a pointer to the Clusterlib Factory
     */
    clusterlib::Factory *getClusterlibFactory();

    /**
     * Register user-defined methods (in addition to the already
     * provided ones).  This should be done prior to start().
     *
     * @param name the name of the RPC method.
     * @param method the RPC method to be registered.
     * @return true if the registration succeeds; false otherwise.
     */
    bool registerMethodRPC(const std::string &name,
                           json::rpc::JSONRPCMethod *method);

    /**
     * Get a value from the configuration (after parseArgs()).
     *
     * @param key Key to retrieve
     * @param valueP Final value
     * @return true if found, false otherwise
     */ 
    bool getConfig(const std::string &key, std::string *valueP) const;

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
     * User defined directives.
     */
    std::map<std::string, std::string> m_directiveMap;

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

}

}

#endif
