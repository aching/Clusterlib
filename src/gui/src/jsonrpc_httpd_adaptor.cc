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
#include "jsonrpc_httpd_adaptor.h"

using namespace std;
using namespace httpd;

namespace json {

namespace rpc {

HttpServerAdaptor::HttpServerAdaptor(HttpServer *s, JSONRPCManager *m) :
    server(s), manager(m) {
}

HttpServerAdaptor::~HttpServerAdaptor() {
    server = NULL;
    manager = NULL;
}

void HttpServerAdaptor::pageHandler(const string &path, 
                                    HttpContext *context) {
    size_t pos = 0;
    
    context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] = 
        "application/json";
    context->response.responseCode = HTTP_OK;
    
    if (context->request.method == HTTP_METHOD_GET) {
        LOG4CXX_INFO(HTTP_LOG, "Getting all JSON-RPC method information.");
        JSONValue::JSONArray methods;
        vector<string> methodNames = manager->getMethodNames();
        
        for (vector<string>::const_iterator iter = methodNames.begin();
             iter != methodNames.end(); 
             ++iter) {
            methods.push_back(*iter);
        }
        
        JSONValue::JSONObject discovery;
        discovery["transport"] = HTTP_METHOD_POST;
        discovery["envelope"] = "JSON-RPC-1.0";
        discovery["methods"] = methods;
        context->response.body = JSONCodec::encode(discovery);
        
        return;
    }
    
    HttpSessionStatePersistence persistence(context->session);
    LOG_INFO(HTTP_LOG, "Dispatching the JSON-RPC invocation.");
    
    try {
        
        do {
            // We may have multiple calls
            JSONValue invoke = 
                JSONCodec::decode(context->request.body, &pos);
            
            // Call and encode the result
            context->response.body += 
                JSONCodec::encode(manager->invoke(invoke, &persistence));
        } while (pos < context->request.body.size());
    } catch (const JSONParseException &ex) {
        LOG_WARN(HTTP_LOG, "Not a valid JSON object (%s)", ex.what());
        // Not a valid JSON invocation
        context->response.responseCode = HTTP_SERVICE_UNAVAILABLE;
        context->response.body = ex.what();
        context->response.headerMap[HTTP_HEADER_CONTENT_TYPE] 
            = "text/plain";
    }
}

HttpServerAdaptor::HttpSessionPersistableState::HttpSessionPersistableState(
    PersistableState *s) :
    state(s) 
{
}

HttpServerAdaptor::HttpSessionPersistableState::~HttpSessionPersistableState()
{
    delete state;
    state = NULL;
}

PersistableState *HttpServerAdaptor::HttpSessionPersistableState::get() 
{
    return state;
}

HttpServerAdaptor::HttpSessionStatePersistence::HttpSessionStatePersistence(
    HttpSession *s) :
    session(s) 
{
}

HttpServerAdaptor::HttpSessionStatePersistence::~HttpSessionStatePersistence()
{
}

PersistableState *HttpServerAdaptor::HttpSessionStatePersistence::get(
    const string &name) {
    HttpSession::HttpSessionStateMap::iterator iter = 
        session->state.find(name);
    if (iter == session->state.end()) {
        return NULL;
    }

    HttpSessionPersistableState *state = 
        dynamic_cast<HttpSessionPersistableState *>(iter->second);
    return (state == NULL) ? NULL : state->get();
}

void HttpServerAdaptor::HttpSessionStatePersistence::set(
    const string &name, PersistableState *state) {
    HttpSession::HttpSessionStateMap::iterator iter = 
        session->state.find(name);
    if (iter != session->state.end()) {
        delete iter->second;
        iter->second = NULL;
    }
    
    session->state[name] = new HttpSessionPersistableState(state);
}

void HttpServerAdaptor::HttpSessionStatePersistence::erase(const string &name)
{
    HttpSession::HttpSessionStateMap::iterator iter = 
        session->state.find(name);
    if (iter != session->state.end()) {
        delete iter->second;
        iter->second = NULL;
        session->state.erase(iter);
    }
}

}

}
