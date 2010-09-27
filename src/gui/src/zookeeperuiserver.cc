#include "gui.h"
#include "zookeeperuiserver.h"
#include "zookeeperperiodiccheck.h"
#include "xmlconfig.h"
#include <cstdlib>
#include <string>
#include <apr_getopt.h>
#include <signal.h>
#include <iostream>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace std;
using namespace httpd;
using namespace log4cxx;
using namespace boost;
using namespace json::rpc;

static const char *appName = "zkuiserver";
extern const char *gInkBuildStamp;

namespace zookeeper { namespace ui {
    
ZooKeeperUIServer *ZooKeeperUIServer::m_singleton = NULL;
    
// Default configuration
static const char *defaultConfig[][2] = 
{
    {"httpd.port", "2188"},
    {"httpd.rootDirectory", ""},
    {"httpd.contenttypes", "htm|text/html,html|text/html,xml|text/xml,js|text/javascript,css|text/css,jpg|image/jpeg,gif|image/gif,pdf|application/pdf,png|image/png,zip|application/zip,tar|application/x-tar,gz|application/x-gzip,tgz|application/x-gtar,txt|text/plain,tif|image/tif"},
    {"httpd.ipv6", "false"},
    {"httpd.sessionlife", "600"},
    {"zookeeper.servers", "localhost:2181"},
    // Mark the end of default configurations.
    {NULL, NULL}
};

const char *jsonRpcPage = "/jsonrpc";
    
void ZooKeeperUIServer::destroyInstance() 
{
    if (m_singleton) {
        m_singleton->stop();
        delete m_singleton;
        m_singleton = NULL;
    }
}

ZooKeeperUIServer *
ZooKeeperUIServer::getInstance() 
{
    if (m_singleton == NULL) {
        m_singleton = new ZooKeeperUIServer();
    }
    
    return m_singleton;
}

ZooKeeperUIServer::~ZooKeeperUIServer() 
{
    stop();
}

ZooKeeperUIServer::ZooKeeperUIServer()
    : m_httpd(NULL) 
{
}

HttpServerAccessHandler::AccessPermission 
ZooKeeperUIServer::canAccess(const HttpContext &context) 
{
    if (m_allowedHosts.get() == NULL) {
        // When there is no allowed host list, by default it is allowing all
        return NEXT;
    }

    return regex_match(context.client, *m_allowedHosts) ? ALLOW : DENY;
}

void 
ZooKeeperUIServer::includeHandler(
    const string &reference, HttpContext *context) 
{
    if (reference == "ZKUI_VERSION") {
        context->response.body = gInkBuildStamp;
    } 
    else if (reference == "ZOOKEEPER_SERVERS") {
        context->response.body = m_config["zookeeper.servers"];
    } 
    else {
        throw HttpServerException(reference + 
                                  " include directive is not supported.");
    }
}
    
clusterlib::Factory *
ZooKeeperUIServer::getClusterlibFactory()
{
    return m_clusterFactory.get();
}

void 
ZooKeeperUIServer::printVersion() 
{
    cout << appName << ": " << gInkBuildStamp << endl;
}
    
void
ZooKeeperUIServer::printUsage() 
{
    printVersion();
    cout <<
"Usage: " << appName <<
" [OPTION]... [VAR=VALUE]...\n\n"
" -h  --help            Display this help and exit.\n"
" -c  --config          Use an XML formatted configuration file for all\n"
"                       arguments.\n"
" -z  --zk_server_port  Zookeeper server port list - overrides config of\n"
"                       zookeeper.servers\n"
"                       (i.e. wm301:2181,wm302:2181)\n"
" -l  --log4cxx         Log4cxx file location - overrides config of\n"
"                       logger.configuration\n"
" -r  --root            Root directory of HTTP server - overrides config of\n"
"                       httpd.rootDirectory\n"
" -v  --version         Displays the version of this executable\n";
}

void
ZooKeeperUIServer::parseArgs(int argc, const char *const*argv) 
{
    string configFile, zookeeperServers, log4cxxFile, rootDirectory;
    apr_pool_t *pool;
    if (apr_pool_create(&pool, NULL) != APR_SUCCESS) {
        cerr << "Not enough memory for argument parsing." << endl;
        exit(1);
    }
    apr_getopt_t *getopt;
    
    if (apr_getopt_init(&getopt, pool, argc, argv) != APR_SUCCESS) {
        cout << "Parsing argument error." << endl;
        exit(1);
    }
    
    const apr_getopt_option_t options[] = {
        {"config", 'c', 1, "Specify the location of the configuration file."},
        {"version", 'v', 0, "Show the version."},
        {"help", 'h', 0, "Show this help."},
        {"zk_server_port", 'z', 1, "Zookeeper server port list."},
        {"log4cxx", 'l', 1, "Log4cxx file."},
        {"root", 'r', 1, "http.rootDirectory path."},
        {NULL, 0, 0, NULL},
    };

    apr_status_t status;
    int optCh;
    const char *optArg;
    while ((status = apr_getopt_long(getopt, options, &optCh, &optArg)) != 
           APR_EOF) {
        if (status != APR_SUCCESS) {
            printUsage();
            exit(1);
        }
        
        switch (optCh) {
            case 'c':
                configFile = optArg;
                break;
            case 'v':
                printVersion();
                exit(0);
                break;
            case 'z':
                zookeeperServers = optArg;
                break;
            case 'l':
                log4cxxFile = optArg;
                break;
            case 'r':
                rootDirectory = optArg;
                break;
            case 'h':
            default:
                printUsage();
                exit(0);
                break;
        }
    }
    
    apr_pool_destroy(pool);
    
    if (configFile.empty() && 
        (zookeeperServers.empty() || log4cxxFile.empty() || 
         rootDirectory.empty())) {
        printUsage();
        exit(1);
    }

    /* Set default configuration */
    for (uint32_t i = 0; defaultConfig[i][0] != NULL; ++i) {
        m_config[defaultConfig[i][0]] = defaultConfig[i][1];
    }

    /* Parse the XML configuration. */     
    if (!configFile.empty()) {
        try {
            configurator::XmlConfig::Parse(configFile, &m_config);
        }
        catch (configurator::ConfigurationException &ex) {
            cerr << "The configuration file is invalid." << endl;
            cerr << ex.what() << endl;
            exit(1);
        }
    }
    
    if (!log4cxxFile.empty()) {
        m_config["logger.configuration"] = log4cxxFile;
    }
    if (!zookeeperServers.empty()) {
        m_config["zookeeper.servers"] = zookeeperServers;
    }
    if (!rootDirectory.empty()) {
        m_config["httpd.rootDirectory"] = rootDirectory;
    }

    if (m_config.find("logger.configuration") != m_config.end()) {
        try {
            PropertyConfigurator::configure(m_config["logger.configuration"]);
        } 
        catch (helpers::Exception &) {
            cerr << "Logger configuration is invalid." << endl;
            cerr << "Fall back on default configuration." << endl;
            BasicConfigurator::configure();
        }
    } 
    else {
        BasicConfigurator::configure();
    }

    LOG_DEBUG(GUI_LOG, "Loaded configuration:");
    for (configurator::Configuration::const_iterator iter = m_config.begin(); 
         iter != m_config.end(); 
         ++iter) {
        LOG_DEBUG(GUI_LOG,
                  "%s:%s", 
                  iter->first.c_str(), 
                  iter->second.c_str());
    }

    // Initialize everything
    if (m_httpd.get()) {
        return;
    }

    m_httpd.reset(new microhttpd::MicroHttpServer(
                    static_cast<uint16_t>(
                        ::atoi(m_config["httpd.port"].c_str())), 
                    m_config["httpd.rootDirectory"], 
                    m_config["httpd.ipv6"] == "true"));
    m_rpcManager.reset(new JSONRPCManager());
    m_adaptor.reset(new HttpServerAdaptor(m_httpd.get(), m_rpcManager.get()));
    m_clusterFactory.reset(
        new clusterlib::Factory(m_config["zookeeper.servers"]));
    clusterlib::Client *client = m_clusterFactory->createClient();
    m_zookeeperPeriodicCheck.reset(new clusterlib::ZookeeperPeriodicCheck(
                                     60*1000,
                                     m_config["zookeeper.servers"],
                                     client->getRoot()));
    m_clusterFactory->registerPeriodicThread(*m_zookeeperPeriodicCheck.get());
    m_clusterRpcMethod.reset(new clusterlib::rpc::json::MethodAdaptor(
                               m_clusterFactory->createClient()));
    m_zookeeperRpcMethod.reset(new zookeeper::rpc::json::MethodAdaptor(
                                 m_config["zookeeper.servers"]));
    configurator::Configuration::const_iterator configIter = 
        m_config.find("httpd.contenttypes");

    /* Set content type */
    if (configIter != m_config.end()) {
        /* Add a comma for parsing simplicity */
        string types = configIter->second;
        types += ",";
        string::size_type pos = 0, lastpos = 0;
        while ((pos = types.find(",", lastpos)) != string::npos) {
            string chunk = types.substr(lastpos, pos - lastpos);
            string::size_type spos = chunk.find("|");
            if (spos != string::npos) {
                // This is a valid chunk, as "extension|MIME type"
                m_httpd->registerContentType(
                    chunk.substr(0, spos), chunk.substr(spos + 1));
            }
            
            // Ignore all invalid chunks
            lastpos = pos + 1;
        }
    }
    
    // Generate list of allowed host regular expressions
    configIter = m_config.find("httpd.allowedhosts");
    if (configIter == m_config.end()) {
        m_allowedHosts.reset(NULL);
    }
    else {
        m_allowedHosts.reset(
            new regex(configIter->second, regex::perl | regex::icase));
    }

    m_httpd->setSessionLife(static_cast<uint32_t>(
                              ::atol(m_config["httpd.sessionlife"].c_str())));
    m_httpd->registerPageHandler(jsonRpcPage, m_adaptor.get());
    m_httpd->registerAccessHandler(this);
    m_httpd->registerIncludeHandler("ZOOKEEPER_SERVERS", this);
    m_httpd->registerIncludeHandler("ZKUI_VERSION", this);

    m_rpcManager->registerMethod(
        "addNotifyableFromKey", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "removeNotifyableFromKey", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getApplicationStatus", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getGroupStatus", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getNodeStatus", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getDataDistributionStatus", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getNotifyableAttributesFromKey", m_clusterRpcMethod.get());        
    m_rpcManager->registerMethod(
        "setNotifyableAttributesFromKey", m_clusterRpcMethod.get());        
    m_rpcManager->registerMethod(
        "removeNotifyableAttributesFromKey", m_clusterRpcMethod.get()); 
    m_rpcManager->registerMethod(
        "getNotifyableChildrenFromKey", m_clusterRpcMethod.get());        
    m_rpcManager->registerMethod(
        "getApplications", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getApplication", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getProperties", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getGroup", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getDataDistribution", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getNode", m_clusterRpcMethod.get());
    m_rpcManager->registerMethod(
        "getChildrenLockBids", m_clusterRpcMethod.get());

    m_rpcManager->registerMethod(
        "zoo_exists", m_zookeeperRpcMethod.get());
    m_rpcManager->registerMethod(
        "zoo_get", m_zookeeperRpcMethod.get());
    m_rpcManager->registerMethod(
        "zoo_set", m_zookeeperRpcMethod.get());
    m_rpcManager->registerMethod(
        "zoo_get_children", m_zookeeperRpcMethod.get());
}

void
ZooKeeperUIServer::start() 
{
    LOG_INFO(GUI_LOG, "Starting server...");
        
    m_httpd->start();
    LOG_INFO(GUI_LOG, "Server started.");
}

void
ZooKeeperUIServer::stop() {
    if (!m_httpd.get()) {
        return;
    }

    LOG_INFO(GUI_LOG, "Stopping server...");
    
    m_httpd->stop();
    m_httpd.reset(NULL);
    m_adaptor.reset(NULL);
    m_zookeeperRpcMethod.reset(NULL);
    m_clusterRpcMethod.reset(NULL);
    m_clusterFactory->cancelPeriodicThread(*m_zookeeperPeriodicCheck.get());
    m_clusterFactory.reset(NULL);
    m_zookeeperPeriodicCheck.reset(NULL);
    m_rpcManager->clearMethods();
    
    LOG_INFO(GUI_LOG, "Server stopped.");
}

}}
