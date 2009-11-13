#ifndef _INCLUDED_JSONRPC_H_
#define _INCLUDED_JSONRPC_H_
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <log4cxx/logger.h>
#include <clusterlib.h>

/**
 * Defines the namespace of JSON-RPC manager. JSON-RPC manager is responsible to unwrap parameters
 * and wrap results according to JSON-RPC specification 1.0.
 */
namespace json { namespace rpc {
    /**
     * Defines the exception class which can be thrown by RPC methods.
     */
    class JSONRPCInvocationException : public virtual JSONException {
    public:
        /**
         * Creates an instance of JSONRPCInvocationException with error message.
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
     * Defines a state persistence, which persists the PersistableState across calls. When the
     * state is managed by the state persistence, the ownership of the pointer should be transfered
     * to the persistence. The persistence will free the pointer as necessary (set, erase, etc).
     */
    class StatePersistence {
    public:
        virtual ~StatePersistence() {}
        virtual PersistableState *get(const std::string &name) = 0;
        virtual void set(const std::string &name, PersistableState *state) = 0;
        virtual void erase(const std::string &name) = 0;
    };

    /**
     * Defines a JSON-RPC method.
     */
    class JSONRPCMethod {
    public:
        /**
         * Destroys the RPC method.
         */
        virtual ~JSONRPCMethod() {}
        /**
         * Invokes the RPC method.
         * @param params the array of parameters of the method.
         * @param persistence the persistence used to store a persistable state.
         * @throws JSONRPCInvocationException if any error occurs during the invocation.
         * @return the return value of the method.
         */
        virtual JSONValue invoke(const std::string &name, const JSONValue::JSONArray &params, StatePersistence *persistence) = 0;
    };

    /**
     * Defines a class which manages multiple JSON-RPC methods, and is responsible to
     * invoke registered method according to the incoming JSON-RPC object. The result will be
     * wrapped according to JSON-RPC specification 1.0.
     */
    class JSONRPCManager {
    public:
        /**
         * Gets the singleton instance of the RPC manager. This method is not thread-safe for the first
         * call. Subsquent calls are thread-safe. DO NOT destroy the instance returned by this method.
         * @return the singleton instance of RPC manager.
         */
        static JSONRPCManager *getInstance();
        /**
         * Registers the RPC method. Each RPC method should have unique name. Registration will fail if
         * the same name has already been registered.
         * @param name the name of the RPC method.
         * @param method the RPC method to be registered.
         * @return true if the registration succeeds; false otherwise.
         */
        virtual bool registerMethod(const std::string &name, JSONRPCMethod *method);
        /**
         * Unregisters the RPC method. Each RPC method should have unique name. Unregistration will fail if
         * the name does not exist.
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
         * Invokes a RPC method according to the JSON-RPC specification 1.0. The method to be invoked must be registered.
         * @param rpcInvocation the encoded JSON-RPC object.
         * @param persistence the persistence used to store a persistable state.
         * @return the encoded JSON-RPC result object.
         */
        virtual JSONValue invoke(const JSONValue &rpcInvocation, StatePersistence *persistence = NULL) const;
        /**
         * Destroys the RPC manager instance.
         */
        virtual ~JSONRPCManager();
    protected:
        /**
         * Defines the map type to store RPC methods.
         */
        typedef std::map<std::string, JSONRPCMethod *> RPCMethodMap;
        /**
         * Represents the singleton instance of the RPC manager.
         */
        static std::auto_ptr<JSONRPCManager> singleton;
        /**
         * Represents the registered RPC methods. Keys are names.
         */
        RPCMethodMap rpcMethods;
        /**
         * Creates a new instance of JSONRPCManager. This private constructor prohibits the creation of
         * a new instance of JSONRPCManager.
         */
        JSONRPCManager();
        /**
         * Generates the error JSON-RPC response.
         * @param message the error message.
         * @param id the JSON-RPC invoke ID.
         * @return the encoded error JSON-RPC response object.
         */
        static JSONValue generateErrorResponse(const std::string &message, const JSONValue &id);
        /**
         * Generates the successful JSON-RPC response.
         * @param ret the returned value of the method.
         * @param id the JSON-RPC invoke ID.
         * @return the encoded successful JSON-RPC response object.
         */
        static JSONValue generateResponse(const JSONValue &ret, const JSONValue &id);
        /**
         * Represents the logger.
         */
        static log4cxx::LoggerPtr logger;
    };
}}
#endif
