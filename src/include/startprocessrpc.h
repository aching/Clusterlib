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

class StartProcessRPC : public virtual ::json::rpc::JSONRPC
{
  public:
    virtual const std::string &getName() const;

    /** 
     * Checks the parameters.
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

    const json::JSONValue &getAddEnv() const
    {
        return m_addEnv;
    }

    void setAddEnv(const json::JSONValue::JSONArray &addEnv)
    {
        m_addEnv = addEnv;
    }

    const json::JSONValue &getPath() const
    {
        return m_path;
    }

    void setPath(const json::JSONValue::JSONString &path)
    {
        m_path = path;
    }

    const json::JSONValue &getCommand() const
    {
        return m_command;
    }

    void setCommand(const json::JSONValue::JSONString &command)
    {
        m_command = command;
    }

  private:
    /**
     * JSON parameter process slot key (JSONString)
     */
    json::JSONValue m_processSlotKey;
    
    /**
     * JSON parameter added environment (JSONArray)
     */
    json::JSONValue m_addEnv;

    /**
     * JSON parameter path (JSONString)
     */
    json::JSONValue m_path;

    /**
     * JSON parameter command (JSONString)
     */
    json::JSONValue m_command;
};

/**
 * Definition of class StartProcessMethod for servers.
 */
class StartProcessMethod
    : public virtual ClusterlibRPCMethod,
      public virtual StartProcessRPC 
{
  public:
    /**
     * Constructor.
     */
    StartProcessMethod() {}

    virtual ::json::JSONValue invoke(
        const std::string &name, 
        const ::json::JSONValue::JSONArray &param, 
        ::json::rpc::StatePersistence *persistence);

    virtual void unmarshalParams(
        const ::json::JSONValue::JSONArray &paramArr);
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
    StartProcessRequest(Client *client) 
        : ClusterlibRPCRequest(client) {}

    virtual ::json::JSONValue::JSONArray marshalParams();
};

}

#endif
