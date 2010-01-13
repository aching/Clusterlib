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

#ifndef _CLUSTERLIBRPC_H_
#define _CLUSTERLIBRPC_H_
 
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
     */
    ClusterlibRPCRequest(Client *client);

    virtual void prepareRequest(
        const ::json::JSONValue::JSONArray &paramArr);

    /**
     * Sends request (Implementation specific destination) to a
     * specific queue.
     *
     * @param destination pointer to a const char * that is a Queue key
     */
    virtual void sendRequest(const void *destination);

    virtual bool waitResponse(int64_t timeout = 0);

    virtual const ::json::JSONValue::JSONObject &getResponse();

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
};

}

#endif
