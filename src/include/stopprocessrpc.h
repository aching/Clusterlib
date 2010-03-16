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
    virtual const std::string &getName() const;

    /** 
     * Check the params.
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
     */
    virtual void checkParams(const ::json::JSONValue::JSONArray &paramArr);

    const json::JSONValue &getProcessSlotKey() const
    {
        return m_processSlotKey;
    }

    void setProcessSlotKey(const json::JSONValue::JSONString &processSlotKey)
    {
        m_processSlotKey = processSlotKey;
    }

  private:
    /**
     * jSON parameter process slot key (JSONString)
     */
    json::JSONValue m_processSlotKey;
};

/**
 * Definition of class StopProcessMethod for servers.
 */
class StopProcessMethod
    : public virtual ClusterlibRPCMethod,
      public virtual StopProcessRPC 
{
  public:
    /**
     * Constructor.
     */
    StopProcessMethod() {}

    virtual ::json::JSONValue invoke(
        const std::string &name, 
        const ::json::JSONValue::JSONArray &param, 
        ::json::rpc::StatePersistence *persistence);

    virtual void unmarshalParams(
        const ::json::JSONValue::JSONArray &paramArr);
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

    virtual ::json::JSONValue::JSONArray marshalParams();
};

}

#endif
