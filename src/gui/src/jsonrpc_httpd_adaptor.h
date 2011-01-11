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

#ifndef _INCLUDED_JSONRPC_HTTPD_ADAPTOR_H_
#define _INCLUDED_JSONRPC_HTTPD_ADAPTOR_H_

#include "jsonrpc.h"
#include "httpd.h"

DEFINE_LOGGER(HTTP_LOG, "json.rpc.HttpServerAdaptor");

namespace json { 

namespace rpc {

class HttpServerAdaptor 
    : public virtual httpd::HttpServerPageHandler 
{
  public:
    HttpServerAdaptor(httpd::HttpServer *server, JSONRPCManager *manager);
    ~HttpServerAdaptor();
    void pageHandler(const std::string &path, httpd::HttpContext *context);
  private:
    httpd::HttpServer *server;
    JSONRPCManager *manager;
    
    class HttpSessionPersistableState 
        : public virtual httpd::HttpSessionState {
      public:
        HttpSessionPersistableState(PersistableState *state);
        ~HttpSessionPersistableState();
        PersistableState *get();
      private:
        PersistableState *state;
    };
    
    class HttpSessionStatePersistence : public virtual StatePersistence {
      public:
        HttpSessionStatePersistence(httpd::HttpSession *session);
        ~HttpSessionStatePersistence();
        PersistableState *get(const std::string &name);
        void set(const std::string &name, PersistableState *state);
        void erase(const std::string &name);
      private:
        httpd::HttpSession *session;
    };
};

}

}

#endif
