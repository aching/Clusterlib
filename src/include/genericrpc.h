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

#ifndef _GENERICRPC_H_
#define _GENERICRPC_H_

namespace clusterlib {

class GenericRPC : public virtual ::json::rpc::JSONRPC {
  public:
    virtual std::string getName();

    /**
     * Only checks that this is one element that is a JSONObject.
     *
     * @param paramArr the JSONArray of parameters
     * @return true if passes, false if fails
     */
    virtual bool checkParams(const ::json::JSONValue::JSONArray &paramArr);
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
     */
    GenericRequest(Client *client, 
                   const std::string &requestName) 
        : ClusterlibRPCRequest(client),
          m_requestName(requestName) {}

    /**
     * Overload to use the requestName instead of getName()
     *
     * @param destination a const char * to the queue name
     */
    virtual void sendRequest(const void *destination);

  private:
    /** 
     * Use this name instead of getName()
     */
    std::string m_requestName;
};

}

#endif
