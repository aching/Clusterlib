#include "gui.h"
#include PATH_APR_GETOPT_H
#include "zookeeperuiserver.h"
#include "zookeeperperiodiccheck.h"
#include "xmlconfig.h"
#include <cstdlib>
#include <string>
#include <signal.h>
#include <iostream>

using namespace std;
using namespace httpd;
using namespace boost;
using namespace json::rpc;

static void 
signalHandler(int signal) {
    switch (signal) {
        case SIGHUP:
        case SIGQUIT:
        case SIGINT:
        case SIGTERM:
            LOG_INFO(GUI_LOG, "Signal caught, shuting down.");
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

    zookeeper::ui::ZooKeeperUIServer *server = 
        zookeeper::ui::ZooKeeperUIServer::getInstance();

    server->parseArgs(argc, argv);
    server->init();
    server->start();

    while (true) {
        sleep(60*60*1000);
    }

    return 0;
}
