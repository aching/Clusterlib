#include "gui.h"
#include "apr_getopt.h"
#include "zookeeperuiserver.h"
#include "zookeeperperiodiccheck.h"
#include "xmlconfig.h"
#include <cstdlib>
#include <signal.h>

using namespace std;
using namespace httpd;
using namespace boost;
using namespace json::rpc;

static const char *appName = "zkuiserver";
static const string buildStamp = string(PACKAGE_STRING) + " " + __DATE__ + " - " + __TIME__;

namespace zookeeper 
{ 

namespace ui 
{
    
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
    {"zookeeper.check", "false"},
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
    map<string, string>::const_iterator directiveMapIt;
    directiveMapIt = m_directiveMap.find(reference);

    if (reference == "ZKUI_VERSION") {
        context->response.body = buildStamp;
    } 
    else if (reference == "ZOOKEEPER_SERVERS") {
        context->response.body = m_config["zookeeper.servers"];
    } 
    else if (directiveMapIt != m_directiveMap.end()) {
        context->response.body = directiveMapIt->second;
    }
    else {
        throw HttpServerException(reference + 
                                  " include directive is not supported.");
    }
}

void 
ZooKeeperUIServer::addDirective(const std::string &key,
                                const std::string &value)
{
    map<string, string>::const_iterator directiveMapIt = 
        m_directiveMap.find(key);
    if (directiveMapIt != m_directiveMap.end()) {
        LOG_WARN(ZUI_LOG, 
                 "addDirective: Key %s already exists with value %s and "
                 "replacing with value %s",
                 key.c_str(),
                 value.c_str(),
                 directiveMapIt->second.c_str());
    }
    m_directiveMap[key] = value;
}
   
clusterlib::Factory *
ZooKeeperUIServer::getClusterlibFactory()
{
    return m_clusterFactory.get();
}

bool
ZooKeeperUIServer::registerMethodRPC(const string &name,
                                     JSONRPCMethod *method)
{
    return m_rpcManager->registerMethod(name, method);
}

bool
ZooKeeperUIServer::getConfig(const string &key, string *valueP) const
{
    map<string, string>::const_iterator configIt = m_config.find(key);
    if (configIt == m_config.end()) {
        return false;
    }
    *valueP = configIt->second;
    return true;
}

void 
ZooKeeperUIServer::printVersion() 
{
    cout << appName << ": " << buildStamp << endl;
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
" -C  --zk_check        Periodically check zookeeper servers -\n"
"                       overrides config of zookeeper.check\n"
"                       (i.e. 'true' or 'false') [default=false]\n"
" -r  --root            Root directory of HTTP server - overrides config of\n"
"                       httpd.rootDirectory\n"
" -p  --httpd_port      Client port of HTTP server - overrides config of\n"
"                       httpd.port\n"
" -v  --version         Displays the version of this executable\n";
}

void
ZooKeeperUIServer::parseArgs(int argc, const char *const*argv) 
{
    map<string, string> parseArgMap;
    string configFile;
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
        {"zk_check", 'C', 1, "Check zookeeper server list."},
        {"root", 'r', 1, "Server root path (httpd.rootDirectory)."},
        {"httpd_port", 'p', 1, "Server port (httpd.port)."},
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
                parseArgMap["zookeeper.servers"] = optArg;
                break;
            case 'C':
                parseArgMap["zookeeper.check"] = optArg;
                break;
            case 'r':
                parseArgMap["httpd.rootDirectory"] = optArg;
                break;
            case 'p':
                parseArgMap["httpd.port"] = optArg;
                break;
            case 'h':
            default:
                printUsage();
                exit(0);
                break;
        }
    }
    
    apr_pool_destroy(pool);
    
    /* Need the configuration file or the basic options */
    if (configFile.empty() && 
        ((parseArgMap.find("zookeeper.servers") == parseArgMap.end()) ||
         (parseArgMap.find("httpd.rootDirectory") == parseArgMap.end()))) {
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
    
    /* Override the XML configuration with any command line arguments. */
    map<string, string>::const_iterator parseArgMapIt;
    for (parseArgMapIt = parseArgMap.begin();
         parseArgMapIt != parseArgMap.end();
         ++parseArgMapIt) {
        m_config[parseArgMapIt->first] = parseArgMapIt->second;
    }

    LOG_INFO(ZUI_LOG, "Loaded configuration:");
    for (configurator::Configuration::const_iterator iter = m_config.begin(); 
         iter != m_config.end(); 
         ++iter) {
        LOG_INFO(ZUI_LOG,
                 "%s:%s", 
                 iter->first.c_str(), 
                 iter->second.c_str());
    }

}

void
ZooKeeperUIServer::init() 
{
    /* Initialize everything */
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
    if (m_config["zookeeper.check"] == "true") {
        m_zookeeperPeriodicCheck.reset(new clusterlib::ZookeeperPeriodicCheck(
            60*1000,
            m_config["zookeeper.servers"],
            client->getRoot()));
        m_clusterFactory->registerPeriodicThread(
            *m_zookeeperPeriodicCheck.get());
    }
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
    
    /* Generate list of allowed host regular expressions */
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
    
    /*
     * Register the include handler for all the possible replacements
     * that all use the same function.
     */
    m_httpd->registerIncludeHandler("ZOOKEEPER_SERVERS", this);
    m_httpd->registerIncludeHandler("ZKUI_VERSION", this);
    map<string, string>::const_iterator directiveMapIt;
    for (directiveMapIt = m_directiveMap.begin();
         directiveMapIt != m_directiveMap.end();
         ++directiveMapIt) {
        m_httpd->registerIncludeHandler(directiveMapIt->first,
                                        this);
    }

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
    m_rpcManager->registerMethod(
        "zoo_create", m_zookeeperRpcMethod.get());
    m_rpcManager->registerMethod(
        "zoo_delete", m_zookeeperRpcMethod.get());
}

void
ZooKeeperUIServer::start() 
{
    LOG_INFO(ZUI_LOG, "Starting server...");
        
    m_httpd->start();
    LOG_INFO(ZUI_LOG, "Server started.");
}

void
ZooKeeperUIServer::stop() {
    if (!m_httpd.get()) {
        return;
    }

    LOG_INFO(ZUI_LOG, "Stopping server...");
    
    m_httpd->stop();
    m_httpd.reset(NULL);
    m_adaptor.reset(NULL);
    m_zookeeperRpcMethod.reset(NULL);
    m_clusterRpcMethod.reset(NULL);
    m_clusterFactory->cancelPeriodicThread(*m_zookeeperPeriodicCheck.get());
    m_clusterFactory.reset(NULL);
    m_zookeeperPeriodicCheck.reset(NULL);
    m_rpcManager->clearMethods();
    
    LOG_INFO(ZUI_LOG, "Server stopped.");
}

}}
