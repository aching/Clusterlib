/*
 * genericrpc.h --
 *
 * Definition of class GenericRPC; it will create/respond to a generic
 * JSON-RPC request.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_GENERICRPC_H_
#define _CL_GENERICRPC_H_

namespace clusterlib {

class GenericRPC : public virtual ::json::rpc::JSONRPC {
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
