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

#ifndef _INCLUDED_ZOOKEEPER_JSONRPC_ADAPTOR_H_
#define _INCLUDED_ZOOKEEPER_JSONRPC_ADAPTOR_H_

#include "jsonrpc_httpd_adaptor.h"
extern "C" {
#include <c-client-src/zookeeper.h>
}

DEFINE_LOGGER(ZKM_LOG, "zookeeper.rpc.json.MethodAdaptor");

namespace zookeeper { 

namespace rpc { 

namespace json {

const int32_t SESSION_TIMEOUT = 30;
const int32_t CONNECTION_TIMEOUT = 5;

/**
 * Handles the ZooKeeper related RPCs.
 */
class MethodAdaptor : public virtual ::json::rpc::JSONRPCMethod 
{
  public:        
    /**
     * Constructor.
     */
    MethodAdaptor(const std::string &servers);
    
    virtual const std::string &getName() const;
    
    virtual void checkParams(const ::json::JSONValue::JSONArray &paramArr);
    
    ::json::JSONValue invoke(const std::string &name, 
                             const ::json::JSONValue::JSONArray &param, 
                             ::json::rpc::StatePersistence *persistence);
    
    virtual ~MethodAdaptor();
    
  private:
    void reconnect();
    
    static void staticGlobalWatcher(zhandle_t *zkHandle, 
                                    int type, 
                                    int state, 
                                    const char *path, 
                                    void *context);
    
    void globalWatcher(int type, int state, const char *path);
    
    ::json::JSONValue::JSONBoolean zooExists(
        ::json::JSONValue::JSONString path);

    ::json::JSONValue::JSONString zooGet(
        ::json::JSONValue::JSONString path);
    
    ::json::JSONValue::JSONString zooSet(
        ::json::JSONValue::JSONString path,
        ::json::JSONValue::JSONString data);
    
    ::json::JSONValue::JSONArray zooGetChildren(
        ::json::JSONValue::JSONString path);

    ::json::JSONValue::JSONString zooCreate(
        ::json::JSONValue::JSONString path,
        ::json::JSONValue::JSONString value);

    ::json::JSONValue::JSONBoolean zooDelete(
        ::json::JSONValue::JSONString path,
        ::json::JSONValue::JSONBoolean recursive);
    
  private:
    std::string servers;

    std::string name;

    zhandle_t *zkHandle;

    pthread_cond_t cond;

    pthread_mutex_t mutex;

    int32_t connectionState;
};

}

}

}

#endif
