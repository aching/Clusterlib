/*
 * clusterlibrpc.h --
 *
 * Default functionality for creating/handling JSON-RPC requests in
 * clusterlib.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef _CL_CLUSTERLIBRPC_H_
#define _CL_CLUSTERLIBRPC_H_
 
namespace clusterlib 
{

/**
 * Defines a clusterlib JSON-RPC Request.  Clients use this object to make
 * requests to clusterlib clients that understand JSON-RPC.
 */ 
class ClusterlibRPCRequest
    : public virtual ::json::rpc::JSONRPCRequest 
{
  public:
    /**
     * Constructor.
     *
     * @param client the client context to use/associate with this request
     * @param data user-defined data that can be associated with the request
     */
    ClusterlibRPCRequest(Client *client, ClientData data = NULL);

    virtual void prepareRequest(
        const ::json::JSONValue::JSONArray &paramArr);

    /**
     * Sends request (Implementation specific destination) to a
     * specific queue.
     *
     * @param destination pointer to a const char * that is a Queue key
     */
    virtual void sendRequest(const void *destination);

    virtual void waitResponse();

    virtual bool waitMsecsResponse(int64_t msecsTimeout);

    virtual const ::json::JSONValue::JSONObject &getResponse();

    virtual ClientData getClientData();

    virtual void setClientData(ClientData data);

    virtual ~ClusterlibRPCRequest() {}

    static bool isValidJSONRPCRequest(
        const ::json::JSONValue::JSONObject &rpcObj);

  protected:
    /**
     * The clusterlib client
     */
    ClientImpl *m_client;

    /**
     * The clusterlib root
     */
    Root *m_root;

    /**
     * The JSON-RPC parameters
     */
    ::json::JSONValue::JSONArray m_paramArr;

    /**
     * JSON-RPC id that was sent with the request
     */
    std::string m_id;

    /**
     * Already get response?
     */
    bool m_gotResponse;

    /**
     * The JSON-RPC response object
     */
    ::json::JSONValue::JSONObject m_response;

    /**
     * Allow the user to keep a pointer to user data.
     */
    ClientData m_data;
};

}

#endif
