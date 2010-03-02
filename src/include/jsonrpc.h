/*
 * jsonrpc.h --
 *
 * Default functionality for handling JSON-RPC as well as clusterlib
 * messaging.
 *
 * $Header$
 * $Revision: 241007 $
 * $Date: 2009-12-15 09:22:26 +0000 (Tue, 15 Dec 2009) $
 */

#ifndef _CL_JSONRPC_H_
#define _CL_JSONRPC_H_
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>

/**
 * Defines the namespace of JSON-RPC manager. JSON-RPC manager is
 * responsible to unwrap parameters and wrap results according to
 * JSON-RPC specification 1.0.
 */
namespace json { namespace rpc {

/**
 * Defines the exception class which can be thrown by RPC methods.
 */
class JSONRPCInvocationException : public virtual JSONException {
  public:
    /**
     * Creates an instance of JSONRPCInvocationException with error 
     * message.
     *
     * @param message the error message.
     */
    explicit JSONRPCInvocationException(const std::string &message);
};

/**
 * Defines a state which can be persisted across calls.
 */
class PersistableState {
  public:
    virtual ~PersistableState() {}
};
 
/**
 * Defines a state persistence, which persists the
 * PersistableState across calls. When the state is managed by the
 * state persistence, the ownership of the pointer should be
 * transfered to the persistence. The persistence will free the
 * pointer as necessary (set, erase, etc).
 */
class StatePersistence {
  public:
    virtual ~StatePersistence() {}
    virtual PersistableState *get(const std::string &name) = 0;
    virtual void set(const std::string &name, PersistableState *state) = 0;
    virtual void erase(const std::string &name) = 0;
};

/**
 * Abstract base class to be inherited by JSONRPCMethod and JSONRPCRequest
 */
class JSONRPC {
  public:
    /**
     * Destructor
     */
    virtual ~JSONRPC() {}

    /**
     * Get the name of this JSON-RPC
     */
    virtual std::string getName() = 0;

    /**
     * Check parameters of this JSON-RPC
     *
     * @param paramArr the parameters of this object
     * @return true if success, false if failure
     */
    virtual bool checkParams(const JSONValue::JSONArray &paramArr) = 0;
};
 
/**
 * Defines a JSON-RPC method.  This method should be registered with a
 * JSONRPCManager to process incoming requests.
 */
 class JSONRPCMethod 
     : public virtual JSONRPC
{
  public:
    /**
     * Destroys the RPC method.
     */
    virtual ~JSONRPCMethod() {}
    
    /**
     * Invokes the RPC method.
     *
     * @param params the array of parameters of the method.
     * @param persistence the persistence used to store a persistable
     *        state.
     * @throws JSONRPCInvocationException if any error occurs during the
     *         invocation.
     * @return the return value of the method.
     */
    virtual JSONValue invoke(const std::string &name, 
                             const JSONValue::JSONArray &params, 
                             StatePersistence *persistence) = 0;
};

/**
 * Defines a JSON-RPC Request.  Clients use this object to make
 * requests to servers understand JSON-RPC.
 */ 
class JSONRPCRequest 
    : public virtual JSONRPC
{
  public:
    /**
     * Prepares the RPC request for submission.  The request is
     * checked as well.  Must be called prior to sendRequest.
     *
     * @param paramObj the params that will be used for the next request
     */
    virtual void prepareRequest(const JSONValue::JSONArray &paramArr) = 0;
    
    /**
     * Send the request to the destination.
     *
     * @param destination implementation-dependent destination
     */
    virtual void sendRequest(const void *destination) = 0;

    /**
     * Wait unconditionally  for the response.
     */
    virtual void waitResponse() = 0;

    /**
     * Wait for a number of milliseconds for the response.
     *
     * @param msecTimeout the amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @return true if response exists
     */
    virtual bool waitMsecsResponse(int64_t msecsTimeout) = 0;

    /**
     * Get response after waitResponse() has succeeded.
     *
     * @return the JSONValue for this RPC.
     */
    virtual const JSONValue::JSONObject &getResponse() = 0;

    /**
     * Get the user-defined data associated with the request.
     */
    virtual clusterlib::ClientData getClientData() = 0;
    
    /**
     * Set the user-defined data associated with the request.
     */
    virtual void setClientData(clusterlib::ClientData data) = 0;
};

/**
 * Defines a class which manages multiple JSON-RPC methods, and is
 * responsible to invoke registered method according to the
 * incoming JSON-RPC object. The result will be wrapped according
 * to JSON-RPC specification 1.0.
 */
class JSONRPCManager {
  public:
    /**
     * Registers the RPC method. Each RPC method should have
     * unique name. Registration will fail if the same name has
     * already been registered.
     *
     * @param name the name of the RPC method.
     * @param method the RPC method to be registered.
     * @return true if the registration succeeds; false otherwise.
     */
    virtual bool registerMethod(const std::string &name, 
                                JSONRPCMethod *method);
    /**
     * Unregisters the RPC method. Each RPC method should have unique 
     * name. Unregistration will fail if the name does not exist.
     *
     * @param name the name of the RPC method.
     * @return true if the unregistration succeeds; false otherwise.
     */
    virtual bool unregisterMethod(const std::string &name);
    
    /**
     * Unregisters all RPC methods.
     */
    virtual void clearMethods();
    
    /**
     * Get the name of all registered RPC methods.
     */
    virtual std::vector<std::string> getMethodNames();
    
    /**
     * Invokes a RPC method according to the JSON-RPC specification 1.0. 
     * The method to be invoked must be registered.
     *
     * @param rpcInvocation the encoded JSON-RPC object.
     * @param persistence the persistence used to store a persistable state.
     * @return the encoded JSON-RPC result object.
     */
    virtual JSONValue invoke(const JSONValue &rpcInvocation, 
                             StatePersistence *persistence = NULL) const;

    /**
     * Invoke() is used for real RPC.  For clusterlib JSON-RPC, the
     * following method will invoked the registered method and put the
     * result on the DEFAULT_RESP_QUEUE if valid.  Otherwise, it will
     * put the result on the DEFAULT_COMPLETED_QUEUE.
     *
     * @param rpcInvocation the JSON encoded JSON-RPC string
     * @param root the clusterlib root pointer
     * @param defaultCompletedQueue the queue to put the result in if no resp
     *        queue is in the rpcInvocation
     * @param persistence the persistence used to store a persistable state
     */
    virtual void invokeAndResp(
        const std::string &rpcInvocation,
        clusterlib::Root *root,
        clusterlib::Queue *defaultCompletedQueue,
        StatePersistence *persistence = NULL) const;
    
    /**
     * Destroys the RPC manager instance.
     */
    virtual ~JSONRPCManager();

  private:
    /**
     * Defines the map type to store RPC methods.
     */
    typedef std::map<std::string, JSONRPCMethod *> RPCMethodMap;
    
    /**
     * Represents the registered RPC methods. Keys are names.
     */
    RPCMethodMap rpcMethods;
    
    /**
     * Generates the error JSON-RPC response.
     * @param message the error message.
     * @param id the JSON-RPC invoke ID.
     * @return the encoded error JSON-RPC response object.
     */
    static JSONValue generateErrorResponse(const std::string &message, 
                                           const JSONValue &id);
    
    /**
     * Generates the successful JSON-RPC response.
     *
     * @param ret the returned value of the method.
     * @param id the JSON-RPC invoke ID.
     * @return the encoded successful JSON-RPC response object.
     */
    static JSONValue generateResponse(const JSONValue &ret, 
                                      const JSONValue &id);    
};
}}

#endif
