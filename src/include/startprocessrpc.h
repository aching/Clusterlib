/*
 * startprocessrpc.h --
 *
 * Definition of class StartProcessRPC; it starts processes in the
 * process slots available to the ActiveNode.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_STARTPROCESSRPC_H_
#define _CL_STARTPROCESSRPC_H_

namespace clusterlib {

class StartProcessRPC : public virtual ::json::rpc::JSONRPC {
  public:
    virtual std::string getName();

    /** 
     * Starts a process.
     *
     * jsonObj keys:
     * Required key: JSONOBJECTKEY_NOTIFYABLEKEY, val: JSONString
     * Optional key: JSONOBJECTKEY_ADDENV,        val: JSONArray
     * Optional key: JSONOBJECTKEY_PATH,          val: JSONString
     * Optional key: JSONOBJECTKEY_COMMAND,       val: JSONString
     *
     * Either none or all of the optional keys must be set or else an
     * error will occur.  The executable arguments (specified) will be
     * set first.  Then the process will be started.  The original
     * object is returned.
     *
     * @param paramArr an array with one element (the map of key-value pairs)
     * @return true if sucess, false if failure
     */    
    virtual bool checkParams(const ::json::JSONValue::JSONArray &paramArr);
};

/**
 * Definition of class StartProcessMethod for servers.
 */
class StartProcessMethod
    : public virtual ::json::rpc::JSONRPCMethod,
      public virtual StartProcessRPC 
{
  public:
    /**
     * Constructor.
     */
    StartProcessMethod(clusterlib::Client *client);

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
 * Definition of class StartProcessRequest for clients.
 */
class StartProcessRequest 
    : public virtual ClusterlibRPCRequest,
      public virtual StartProcessRPC
{
  public:
    /**
     * Constructor.
     */
    StartProcessRequest(Client *client) : ClusterlibRPCRequest(client) {}
};

}

#endif
