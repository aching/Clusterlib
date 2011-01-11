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

#ifndef _CL_MICROHTTPSERVER_H_
#define _CL_MICROHTTPSERVER_H_

#include "httpd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <microhttpd.h>
#include <fstream>

namespace httpd { 

namespace microhttpd {

class MicroHttpServer : public virtual HttpServer
{
  public:
    MicroHttpServer(uint16_t port, 
                    const std::string &rootDirectory, 
                    bool ipv6 = false);

    ~MicroHttpServer();

    void start();

    void stop();

  private:
    struct HttpFileContext {
        HttpContext *context;
        bool includeHandlerProcess;
        std::string buffer;
        std::ifstream file;
        MicroHttpServer *_this;
    };
    
    MHD_Daemon *httpDaemon;

    static std::string getSockAddrPresentation(
        const struct sockaddr *addr, socklen_t addrlen);

    static int accessHandlerCallback(void *cls, 
                                     MHD_Connection *connection,
                                     const char *url, 
                                     const char *method,
                                     const char *version, 
                                     const char *upload_data, 
                                     size_t *upload_data_size, 
                                     void **con_cls);

    static int acceptPolicyCallback(
        void *cls, const struct sockaddr *addr, socklen_t addrlen);
    
    static int requestCompletedCallback(void *cls, 
                                        MHD_Connection *connection, 
                                        void **con_cls, 
                                        MHD_RequestTerminationCode toe);

    static int keyValueCallback(
        void *cls, MHD_ValueKind kind, const char *key, const char *value);

#ifdef MHD_MAX_SIZE_T
    static ssize_t contentReaderCallback(
        void *cls, uint64_t pos, char *buf, size_t max);
#else
    static int contentReaderCallback(
        void *cls, uint64_t pos, char *buf, int max);
#endif

    static void contentReaderFreeCallback(void *cls);

    static off_t getFileSize(const std::string &file);

    static bool isDirectory(const std::string &file);

    int accessHandler(MHD_Connection *connection, 
                      const char *url, 
                      const char *method,
                      const char *version, 
                      const char *upload_data, 
                      size_t *upload_data_size, 
                      void **con_cls);

    int acceptPolicy(const struct sockaddr *addr, socklen_t addrlen);

    int requestCompleted(MHD_Connection *connection, 
                         void **con_cls, 
                         MHD_RequestTerminationCode toe);

    int dispatchRequest(MHD_Connection *connection, HttpContext *context);

    int contentReader(HttpFileContext *fc, uint64_t pos, char *buf, int max);
};

}

}

#endif
