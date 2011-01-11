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

#ifndef _CL_GENERICRPC_H_
#define _CL_GENERICRPC_H_

namespace clusterlib  {

/**
 * Creates/responds to a generic JSON-RPC request.
 */
class GenericRPC : public virtual ::json::rpc::JSONRPC
{
  public:
    virtual const std::string &getName() const;

    /**
     * Only checks that this is one element that is a JSONObject.
     *
     * @param paramArr the JSONArray of parameters
     */
    virtual void checkParams(const ::json::JSONValue::JSONArray &paramArr);

    /**
     * Get the current request name.
     */
    const std::string &getRequestName() const
    {
        return m_requestName;
    }

    /**
     * Set the request name so getName() gets this instead of a
     * generic request.
     */
    void setRequestName(const std::string &requestName) 
    {
        m_requestName = requestName;
    }

    const json::JSONValue::JSONArray &getRPCParams() const
    {
        return m_RPCParams;
    }

    void setRPCParams(const json::JSONValue::JSONArray &rPCParams)
    {
        m_RPCParams = rPCParams;
    }

  private:
    /** 
     * Use this name for getName()
     */
    std::string m_requestName;

    /**
     * JSON parameter RPC params (JSONArray)
     */
    json::JSONValue::JSONArray m_RPCParams;
};

/**
 * Definition of class GenericRequest for clients.
 */
class GenericRequest 
    : public virtual ClusterlibRPCRequest,
      public virtual GenericRPC
{
  public:
    /**
     * Constructor.
     *
     * @param client the client context to use/associate with this request
     * @param requestName the name of the request
     * @param data user-defined data that can be associated with the request
     */
    GenericRequest(Client *client, 
                   const std::string &requestName,
                   ClientData data = NULL) 
        : ClusterlibRPCRequest(client, data)
    {
        GenericRPC::setRequestName(requestName);
    }

    virtual ::json::JSONValue::JSONArray marshalParams();
};

}

#endif
