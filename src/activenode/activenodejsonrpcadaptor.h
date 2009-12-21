/*
 * activenodejsonrpcadaptor.h --
 *
 * Definition of class ActiveNodeJSONRPCAdaptor; it manages the
 * methods available to the ActiveNode.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _ACTIVENODEJSONRPCADAPTOR_H_
#define _ACTIVENODEJSONRPCADAPTOR_H_

/**
 * Definition of class ActiveNodeJSONRPCAdaptor.
 */
class ActiveNodeJSONRPCAdaptor : public virtual ::json::rpc::JSONRPCMethod {
  public:
    ActiveNodeJSONRPCAdaptor(clusterlib::Client *client);
    virtual ::json::JSONValue invoke(
        const std::string &name, 
        const ::json::JSONValue::JSONArray &param, 
        ::json::rpc::StatePersistence *persistence);
  private:
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
     * @param jsonObj the map of key-value pairs
     * @return a map of key-val pairs
     */
    ::json::JSONValue::JSONObject startProcess(
        const ::json::JSONValue::JSONObject &jsonObj);

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
     * @param jsonObj the map of key-value pairs
     * @return a map of key-val pairs
     */
    ::json::JSONValue::JSONObject stopProcess(
        const ::json::JSONValue::JSONObject &jsonObj);

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

#endif
