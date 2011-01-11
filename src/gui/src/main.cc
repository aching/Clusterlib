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

#include "gui.h"
#include "apr_getopt.h"
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
