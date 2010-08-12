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
 
namespace clusterlib  {

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
     *        It may NOT be the same client that is used for the 
     *        JSONRPCResponseClient as this can cause problems with events not 
     *        being delivered.
     * @param data user-defined data that can be associated with the request
     */
    explicit ClusterlibRPCRequest(Client *client, ClientData data = NULL);

    /**
     * In ClusterlibRPCRequest, the destination is the receiving queue
     * of the destination Notifyable.
     *
     * @param destination The Notifyable key of the receiving queue as a 
     *        JSONString.
     */
    virtual void setDestination(const json::JSONValue &destination);

    /**
     * In ClusterlibRPCRequest, the destination is the receiving queue
     * of the destination Notifyable.
     *
     * @return The Notifyable key of the receiving queue as a 
     *        JSONString.
     */
    virtual json::JSONValue getDestination();

    virtual void sendRequest();

    virtual void waitResponse();

    virtual bool waitMsecsResponse(int64_t msecsTimeout);

    virtual const json::JSONValue &getResponseResult() const;

    virtual const json::JSONValue &getResponseError() const;

    virtual const json::JSONValue &getResponseId() const;

    virtual const json::JSONValue::JSONObject &getResponse() const;

    virtual ClientData getClientData();

    virtual void setClientData(ClientData data);

    virtual ~ClusterlibRPCRequest() {}

    /**
     * Get the response queue.
     *
     * @return the current response queue key
     */
    const ::json::JSONValue::JSONString &getRespQueueKey() const
    {
        return m_respQueueKey;
    }

    /**
     * Set the response queue.
     * 
     * @param respQueueKey the response queue key or empty if no
     *        response desired.
     */
    void setRespQueueKey(const ::json::JSONValue::JSONString &respQueueKey)
    {
        m_respQueueKey = respQueueKey;
    }

    /**
     * Every RPC request derived class must implement the marshaling
     * of its member data into a JSONArray.  This function is used
     * internally prior to sending the request, but may also be used
     * for debugging.
     * 
     * @return the marshaled JSONArray params
     */
    virtual ::json::JSONValue::JSONArray marshalParams() = 0;

    /**
     * Helper function to ensure that a json object is properly
     * configured as a json-rpc request.
     *
     * @param rpcObj the object to check.
     * @return true if valid, false otherwise
     */
    static bool isValidJSONRPCRequest(
        const ::json::JSONValue::JSONObject &rpcObj);

  private:
    /**
     * The clusterlib client that will be used for the response
     * waiting.  This can be the same client as the response
     * processor since it doesn't wait for user-level events internally..
     */
    ClientImpl *m_client;

    /**
     * The clusterlib Root
     */
    boost::shared_ptr<Root> m_rootSP;

    /**
     * The destination Queue
     */
    boost::shared_ptr<Queue> m_destinationQueueSP;

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

    /**
     * The key of the response queue (empty means no response desired).
     */
    json::JSONValue::JSONString m_respQueueKey;
};

class ClusterlibRPCMethod
    : public json::rpc::JSONRPCMethod
{
  public:
    /**
     * Constructor
     */
    ClusterlibRPCMethod()
        : m_RPCManager(NULL) {}

    /**
     * Destructor
     */
    virtual ~ClusterlibRPCMethod() {}

    /**
     * This function is user-defined and is called just prior to the
     * JSONRPCMethod::invoke() function.  It sets the arguments for
     * each member variables.  It is also called after checkParams(),
     * so the implementor need not worry whether all the params exist.
     *
     * @param paramArr the param array from the request
     */
    virtual void unmarshalParams(
        const ::json::JSONValue::JSONArray &paramArr) = 0;

    /**
     * This function is intended for method designers to use to update
     * the status of their RPC methods while they are running.  This
     * provides a window into the status of the RPC method while it is
     * happening.  Each attempt will try for 100ms.  If the
     * m_RPCMethodHandlerPropertyList given to the
     * ClusterlibRPCManager is NULL, no status will be reported.
     *
     * @param status user-defined status update
     * @param maxRetries the number of attempts to update the status before 
     *        giving up (count fail due to other simultaneous updates).  
     *        -1 means try forever, and 0 means no retry.
     * @param maxStatusesShown the maximum number of statuses to display,
     *        0, means no status will be shown, -1 indicates all statuses
     *        will be shown
     */
    bool setMethodStatus(const std::string &status, 
                         int32_t maxRetries = 5,
                         int32_t maxStatusesShown = 20);

    /**
     * Set the rpc manager.  Not intended for users, but set during
     * JSONRPCManager::registerMethod().
     *
     * @param rPCManager pointer to the manager (Cannot be NULL)
     */
    void setRPCManager(ClusterlibRPCManager *rPCManager);

    /**
     * Get the rpc manager.  Cannot be called prior to
     * registerMethod() being called on this object or else will
     * throw.  This allows the user-defined methods to be able to get
     * the clusterlib root.
     *
     * @return pointer to the rpc manager.  If NULL, an exception will
     *         be thrown.
     */
    ClusterlibRPCManager *getRPCManager();

  private:
    /**
     * If set, this is RPC manager that is associated with this method.
     */
    ClusterlibRPCManager *m_RPCManager;
};

/**
 * When instantiated, this class can be added as a client to
 * clusterlib to handle various JSON methods.
 */
class ClusterlibRPCManager 
    : public json::rpc::JSONRPCManager 
{
  public:
    /**
     * Constructor.
     *
     * @param rootSP Clusterlib root
     * @param recvQueueSP Queue where this client receives JSON-RPC requests
     * @param completedQueueSP Queue where this client places responses or 
     *        errors for JSON-RPC requests if no destination is specified.
     * @param completedQueueMaxSize the maximum number of elements in the 
     *        completedQueue, -1 for infinite, 0 for none.
     * @param rpcMethodHandlerPropertyListSP If set, the rpcManager will update
     *        rpcMethodHandlerPropertyList with the current request and status
     *        information
     */
    ClusterlibRPCManager(
        const boost::shared_ptr<Root> &rootSP,
        const boost::shared_ptr<Queue> &recvQueueSP,
        const boost::shared_ptr<Queue> &completedQueueSP,
        int32_t completedQueueMaxSize,
        const boost::shared_ptr<PropertyList> &rpcMethodHandlerPropertyListSP);

    /**
     * Destructor.
     */
    virtual ~ClusterlibRPCManager() {}
        
    /**
     * Invoke() is used for real RPC.  For clusterlib JSON-RPC, the
     * following method will invoked the registered method and put the
     * result on the DEFAULT_RESP_QUEUE if valid.  Otherwise, it will
     * put the result on the DEFAULT_COMPLETED_QUEUE.
     *
     * @param rpcInvocation the JSON encoded JSON-RPC string
     * @param persistence the persistence used to store a persistable state
     */
    virtual void invokeAndResp(
        const std::string &rpcInvocation,
        ::json::rpc::StatePersistence *persistence = NULL);

    /**
     * Get the root
     *
     * @return the root
     */
    virtual const boost::shared_ptr<Root> &getRoot()
    {
        return m_rootSP;
    }

    /**
     * Get the recv queue
     *
     * @return the recv queue
     */
    virtual const boost::shared_ptr<Queue> &getRecvQueue()
    {
        return m_recvQueueSP;
    }

    /**
     * Get the completed queue
     *
     * @return the completed queue
     */
    virtual const boost::shared_ptr<Queue> &getCompletedQueue()
    {
        return m_completedQueueSP;
    }

    /**
     * Get the rpc method handler property list
     *
     * @return pointer to m_RPCMethodHandlerPropertyList
     */
    virtual const boost::shared_ptr<PropertyList> &
    getRPCMethodHandlerPropertyList()
    {
        return m_RPCMethodHandlerPropertyListSP;
    }

  private:
    /**
     * This function is intended make debugging easier by updating a
     * property list with the current request being worked on.  If the
     * m_RPCMethodHandlerPropertyList given to the
     * ClusterlibRPCManager is NULL, no status will be reported.  The request 
     *
     * @param jsonRequest the current request
     * @param startingRequest if true, starting, else, finishing
     * @param maxRetries the number of attempts to update the request before 
     *        giving up (count fail due to other simultaneous updates).  
     *        -1 means try forever, and 0 means no retry.
     */
    bool setBasicRequestStatus(
        const ::json::JSONValue &jsonRequest,
        bool startingRequest,
        int32_t maxRetries = 5);
        
  private:
    /**
     * The clusterlib root
     */
    boost::shared_ptr<Root> m_rootSP;

    /**
     * The queue that the manager is waiting for requests
     */
    boost::shared_ptr<Queue> m_recvQueueSP;
    
    /**
     * The queue that used to specify where completed requests will go
     * (limited to a finite size (m_completedQueueMaxSize).
     */
    boost::shared_ptr<Queue> m_completedQueueSP;
    
    /**
     * The maximum size of m_completedQueue, -1 for infinite, 0 for don't use.
     */
    int32_t m_completedQueueMaxSize;

    /**
     * If set, this is the property list that will report the updates
     * of the currently running JSON-RPC request.
     */
    boost::shared_ptr<PropertyList> m_RPCMethodHandlerPropertyListSP;
};

}

#endif
