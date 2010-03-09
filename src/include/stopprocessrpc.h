/*
 * stopprocessrpc.h --
 *
 * Definition of class StopProcessRPC; it stops processes in the
 * process slots available to the ActiveNode.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_STOPPROCESSRPC_H_
#define _CL_STOPPROCESSRPC_H_

namespace clusterlib {

class StopProcessRPC : public virtual ::json::rpc::JSONRPC {
  public:
    virtual std::string getName();

    /** 
     * Stop a process.
     *
     * jsonObj keys:
     * Required key: JSONOBJECTKEY_NOTIFYABLEKEY, val: JSONString
     * Optional key: JSONOBJECTKEY_SIGNAL,        val: JSONInteger
     *
     * Stops a process on the notifyable key with the given signal (or
     * default if not given).  If success, will return the original
     * object.
     *
     * @param paramArr an array with one element (the map of key-value pairs)
     * @return true if scuess, false if failure
     */
    virtual bool checkInitParams(const ::json::JSONValue::JSONArray &paramArr,
                                 bool initialize);
};

/**
 * Definition of class StopProcessMethod for servers.
 */
class StopProcessMethod
    : public virtual ::json::rpc::JSONRPCMethod,
      public virtual StopProcessRPC 
{
  public:
    /**
     * Constructor.
     */
    StopProcessMethod(clusterlib::Client *client);

    virtual ::json::JSONValue invoke(
        const std::string &name, 
        const ::json::JSONValue::JSONArray &param, 
        ::json::rpc::StatePersistence *persistence);
  private:
    /**
     * The clusterlib client pointer
     */
    clusterlib::Client *m_client;
    
    /**
     * The clusterlib root pointer
     */
    clusterlib::Root *m_root;
};

/**
 * Definition of class StopProcessRequest for clients.
 */
class StopProcessRequest 
    : public virtual ClusterlibRPCRequest,
      public virtual StopProcessRPC
{
  public:
    /**
     * Constructor.
     */
    StopProcessRequest(Client *client) : ClusterlibRPCRequest(client) {}
};

}

#endif
