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

namespace json { 

/**
 * Defines the namespace of JSON-RPC manager. JSON-RPC manager is
 * responsible to unwrap parameters and wrap results according to
 * JSON-RPC specification 1.0.
 */
namespace rpc {

/**
 * Defines the exception class which can be thrown by RPC methods.
 */
class JSONRPCInvocationException 
    : public ::json::Exception 
{
  public:
    /**
     * Creates an instance of JSONRPCInvocationException with error 
     * message.
     *
     * @param msg the error message.
     */
    explicit JSONRPCInvocationException(const std::string &msg) 
        : ::json::Exception(msg) {}
};

/**
 * RPC exception when the request parameters are invalid.
 */
class JSONRPCParamsException 
    : public JSONRPCInvocationException
{
  public:
    /**
     * Creates an instance of JSONRPCInvocationException with error 
     * message.
     *
     * @param msg the error message.
     */
    explicit JSONRPCParamsException(const std::string &msg) 
        : JSONRPCInvocationException(msg) {}
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
    /**
     * Get the persistable state.
     *
     * @param name the name of the object
     * @return the persistable state
     */
    virtual PersistableState *get(const std::string &name) = 0;
    
    /**
     * Set the persitable state.
     *
     * @param name the name of the object
     * @param state the new state
     */
    virtual void set(const std::string &name, PersistableState *state) = 0;

    /**
     * Erase the state of the object.
     *
     * @param name the name of the object
     */
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
     * Get the name of this JSON-RPC (used as the JSON-RPC method).
     */
    virtual const std::string &getName() const = 0;

    /**
     * Check the parameters of this JSON-RPC
     *
     * @param paramArr the parameters of this object
     * @throws if there is a problem
     */
    virtual void checkParams(const JSONValue::JSONArray &paramArr) = 0;
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
     * @param name the name of the RPC method
     * @param param the array of parameters of the method.
     * @param persistence the persistence used to store a persistable
     *        state.
     * @throws JSONRPCInvocationException if any error occurs during the
     *         invocation.
     * @return the return value of the method.
     */
    virtual ::json::JSONValue invoke(
        const std::string &name, 
        const ::json::JSONValue::JSONArray &param, 
        ::json::rpc::StatePersistence *persistence) = 0;
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
     * @param msecsTimeout the amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @return true if response exists
     */
    virtual bool waitMsecsResponse(int64_t msecsTimeout) = 0;

    /**
     * Get the JSON response result (only after getting a response).
     *
     * @return a reference to the JSONValue::JSONNull if there was an
     *         error, else get the user-defined result.
     */
    virtual const json::JSONValue &getResponseResult() const = 0;

    /**
     * Get the JSON response error (only after getting a response).
     *
     * @return a reference to the JSONValue::JSONNull if no error, else
     *         get the user-defined error.
     */
    virtual const json::JSONValue &getResponseError() const = 0;

    /**
     * Get the JSON response id (only after getting a response).
     *
     * @return a reference to the JSONValue that is used to match the
     *          request with the response.
     */
    virtual const json::JSONValue &getResponseId() const = 0;

    /**
     * Get the full response object - result, error, and id (only
     * after getting a response).
     *
     * @return a reference to the JSONObject that is the response
     */
    virtual const json::JSONValue::JSONObject &getResponse() const = 0;

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
                             StatePersistence *persistence = NULL);

    /**
     * Destroys the RPC manager instance.
     */
    virtual ~JSONRPCManager();

  private:
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

    /**
     * Get the lock
     */
    clusterlib::Mutex *getLock();

  private:
    /**
     * Make this class thread-safe
     */
    clusterlib::Mutex m_lock;

    /**
     * Defines the map type to store RPC methods.
     */
    typedef std::map<std::string, JSONRPCMethod *> RPCMethodMap;
    
    /**
     * Represents the registered RPC methods. Keys are names.
     */
    RPCMethodMap m_rpcMethods;    
};

}}

#endif
