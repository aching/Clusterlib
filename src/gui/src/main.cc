#include "main.h"
#include "zookeeperperiodiccheck.h"
#include "xmlconfig.h"
#include "mhd_httpd.h"
#include <cstdlib>
#include <string>
#include <apr_getopt.h>
#include <signal.h>
#include <iostream>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace std;
using namespace log4cxx;
using namespace httpd;
using namespace boost;
using namespace json::rpc;

const char *appName = "zkuiserver";
LoggerPtr logger(Logger::getLogger("Main"));

namespace zookeeper { namespace ui {
    ZooKeeperUIServer *ZooKeeperUIServer::singleton = NULL;
    LoggerPtr ZooKeeperUIServer::logger(Logger::getLogger("zookeeper.ui.ZooKeeperUIServer"));

    // Default configuration
    const char *defaultConfig[][2] = {
        {"httpd.port", "2188"},
        {"httpd.rootDirectory", ""},
        {"httpd.ipv6", "false"},
        {"httpd.sessionlife", "600"},
        {"zookeeper.servers", "localhost:2181"},
        // Mark the end of default configurations.
        {NULL, NULL}
    };

    const char *jsonRpcPage = "/jsonrpc";
    
    void ZooKeeperUIServer::destroyInstance() {
        if (singleton) {
            singleton->stop();
            delete singleton;
            singleton = NULL;
        }
    }

    ZooKeeperUIServer *ZooKeeperUIServer::getInstance() {
        if (singleton == NULL) {
            singleton = new ZooKeeperUIServer();
        }

        return singleton;
    }

    ZooKeeperUIServer::~ZooKeeperUIServer() {
        stop();
    }

    ZooKeeperUIServer::ZooKeeperUIServer()
        : httpd(NULL) {
    }

    HttpServerAccessHandler::AccessPermission ZooKeeperUIServer::canAccess(const HttpContext &context) {
        if (allowedHosts.get() == NULL) {
            // When there is no allowed host list, by default it is allowing all
            return NEXT;
        }

        return regex_match(context.client, *allowedHosts) ? ALLOW : DENY;
    }

    void ZooKeeperUIServer::includeHandler(
        const string &reference, HttpContext *context) {
        if (reference == "ZKUI_VERSION") {
            context->response.body = gInkBuildStamp;
        } else if (reference == "ZOOKEEPER_SERVERS") {
            context->response.body = config["zookeeper.servers"];
        } else {
            throw HttpServerException(reference + 
                                      " include directive is not supported.");
        }
    }
    
    void ZooKeeperUIServer::printVersion() {
        cout << appName << ": " << gInkBuildStamp << endl;
    }
    
    void ZooKeeperUIServer::printUsage() {
        printVersion();
        cout << "Usage: " << appName
             << " -c|--config <configFile> [-h|--help] [-v|--version]" << endl;
    }

    void ZooKeeperUIServer::parseArgs(int argc, const char *const*argv) {
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
            {NULL, 0, 0, NULL},
        };

        apr_status_t status;
        int optCh;
        const char *optArg;
        while ((status = apr_getopt_long(getopt, options, &optCh, &optArg)) != APR_EOF) {
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
            case 'h':
            default:
                printUsage();
                exit(0);
                break;
            }
        }
        
        apr_pool_destroy(pool);

        if (configFile.size() == 0) {
            printUsage();
            exit(1);
        }

        // Set default configuration
        for (uint32_t i=0;defaultConfig[i][0] != NULL;++i) {
            config[defaultConfig[i][0]] = defaultConfig[i][1];
        }

        // Parse the XML configuration.
        try {
            configurator::XmlConfig::Parse(configFile, &config);
        } catch (configurator::ConfigurationException &ex) {
            cerr << "The configuration file is invalid." << endl;
            cerr << ex.what() << endl;
            exit(1);
        }

        if (config.find("logger.configuration") != config.end()) {
            try {
                PropertyConfigurator::configure(config["logger.configuration"]);
            } catch (helpers::Exception &) {
                cerr << "Logger configuration is invalid." << endl;
                cerr << "Fall back on default configuration." << endl;
                BasicConfigurator::configure();
            }
        } else {
            BasicConfigurator::configure();
        }

        LOG4CXX_DEBUG(logger, "Loaded configuration:");
        for (configurator::Configuration::const_iterator iter = config.begin(); iter != config.end(); ++iter) {
            LOG4CXX_DEBUG(logger, iter->first + ":" + iter->second);
        }
    }

    void ZooKeeperUIServer::start() {
        if (httpd.get()) {
            return;
        }

        LOG4CXX_INFO(logger, "Starting server...");

        httpd.reset(new microhttpd::MicroHttpServer((uint16_t)atoi(config["httpd.port"].c_str()), config["httpd.rootDirectory"], config["httpd.ipv6"] == "true"));
        rpcManager.reset(new JSONRPCManager());
        adaptor.reset(new HttpServerAdaptor(httpd.get(), rpcManager.get()));
        clusterFactory.reset(
            new clusterlib::Factory(config["zookeeper.servers"]));
        clusterlib::Client *client = clusterFactory->createClient();
        zookeeperPeriodicCheck.reset(new clusterlib::ZookeeperPeriodicCheck(
                                         60*1000,
                                         config["zookeeper.servers"],
                                         client->getRoot()));
        clusterFactory->registerPeriodicThread(*zookeeperPeriodicCheck.get());
        clusterRpcMethod.reset(
            new clusterlib::rpc::json::MethodAdaptor(
                clusterFactory->createClient()));
        zookeeperRpcMethod.reset(
            new zookeeper::rpc::json::MethodAdaptor(
                config["zookeeper.servers"]));
        configurator::Configuration::const_iterator configIter = 
            config.find("httpd.contenttypes");

        // Set content type
        if (configIter != config.end()) {
            // Add a comma for parsing simplicity
            string types = configIter->second;
            types += ",";
            string::size_type pos = 0, lastpos = 0;
            while ((pos = types.find(",", lastpos)) != string::npos) {
                string chunk = types.substr(lastpos, pos - lastpos);
                string::size_type spos = chunk.find("|");
                if (spos != string::npos) {
                    // This is a valid chunk, as "extension|MIME type"
                    httpd->registerContentType(chunk.substr(0, spos), chunk.substr(spos + 1));
                }

                // Ignore all invalid chunks
                lastpos = pos + 1;
            }
        }

        // Generate list of allowed host regular expressions
        configIter = config.find("httpd.allowedhosts");
        if (configIter == config.end()) {
            allowedHosts.reset(NULL);
        } else {
            allowedHosts.reset(new regex(configIter->second, regex::perl | regex::icase));
        }

        httpd->setSessionLife((uint32_t)atol(config["httpd.sessionlife"].c_str()));
        httpd->registerPageHandler(jsonRpcPage, adaptor.get());
        httpd->registerAccessHandler(this);
        httpd->registerIncludeHandler("ZOOKEEPER_SERVERS", this);
        httpd->registerIncludeHandler("ZKUI_VERSION", this);
        rpcManager->registerMethod(
            "addNotifyableFromKey", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "removeNotifyableFromKey", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getApplicationStatus", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getGroupStatus", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getNodeStatus", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getDataDistributionStatus", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getNotifyableAttributesFromKey", clusterRpcMethod.get());        
        rpcManager->registerMethod(
            "setNotifyableAttributesFromKey", clusterRpcMethod.get());        
        rpcManager->registerMethod(
            "removeNotifyableAttributesFromKey", clusterRpcMethod.get()); 
        rpcManager->registerMethod(
            "getNotifyableChildrenFromKey", clusterRpcMethod.get());        
        rpcManager->registerMethod(
            "getApplications", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getApplication", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getProperties", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getGroup", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getDataDistribution", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getNode", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "getChildrenLockBids", clusterRpcMethod.get());
        rpcManager->registerMethod(
            "zoo_exists", zookeeperRpcMethod.get());
        rpcManager->registerMethod(
            "zoo_get", zookeeperRpcMethod.get());
        rpcManager->registerMethod(
            "zoo_set", zookeeperRpcMethod.get());
        rpcManager->registerMethod(
            "zoo_get_children", zookeeperRpcMethod.get());

        httpd->start();
        LOG4CXX_INFO(logger, "Server started.");
    }

    void ZooKeeperUIServer::stop() {
        if (!httpd.get()) {
            return;
        }

        LOG4CXX_INFO(logger, "Stopping server...");

        httpd->stop();
        httpd.reset(NULL);
        adaptor.reset(NULL);
        zookeeperRpcMethod.reset(NULL);
        clusterRpcMethod.reset(NULL);
        clusterFactory->cancelPeriodicThread(*zookeeperPeriodicCheck.get());
        clusterFactory.reset(NULL);
        zookeeperPeriodicCheck.reset(NULL);
        rpcManager->clearMethods();

        LOG4CXX_INFO(logger, "Server stopped.");
    }
}}

void signalHandler(int signal) {
    switch (signal) {
    case SIGHUP:
    case SIGQUIT:
    case SIGINT:
    case SIGTERM:
        LOG4CXX_INFO(logger, "Signal caught, shuting down.");
        zookeeper::ui::ZooKeeperUIServer::destroyInstance();
        exit(0);
        break;
    }
}

int main(int argc, const char *const*argv) {
    apr_app_initialize(&argc, &argv, NULL);
    atexit(apr_terminate);
    atexit(zookeeper::ui::ZooKeeperUIServer::destroyInstance);
    signal(SIGHUP, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    zookeeper::ui::ZooKeeperUIServer *server = zookeeper::ui::ZooKeeperUIServer::getInstance();

    server->parseArgs(argc, argv);
    server->start();

    while (true) {
        sleep(60*60*1000);
    }

    return 0;
}
