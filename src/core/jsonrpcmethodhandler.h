/*
 * jsonrpcmethodhandler.cc --
 *
 * Definition of class JSONRPCMethodHandler; it represents a built-in
 * handler for JSON-RPC methods in clusterlib.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#ifndef _JSONRPCMETHODHANDLER_H_
#define _JSONRPCMETHODHANDLER_H_

namespace clusterlib
{

/**
 * Handle JSON-RPC requests in the receiving queue.
 */
class JSONRPCMethodHandler : public UserEventHandler
{
  public:
    /**
     * Constructor.
     */
    JSONRPCMethodHandler(Queue *recvQueue,
                         Queue *completedQueue,
                         Root *root,
                         ::json::rpc::JSONRPCManager *rpcManager)
        : UserEventHandler(recvQueue, EN_QUEUECHILDCHANGE, NULL, true),
          m_recvQueue(recvQueue),
          m_completedQueue(completedQueue),
          m_root(root),
          m_rpcManager(rpcManager) {}
    
    virtual void handleUserEvent(Event e);

  private:
    /**
     * The receiving queue for JSON-RPC requests.
     */
    Queue *m_recvQueue;

    /**
     * The completed queue for processed JSON-RPC requests that have
     * no response queue or that are unable to be decoded.
     */
    Queue *m_completedQueue;

    /**
     * The clusterlib root pointer.
     */
    Root *m_root;

    /**
     * The RPC manager that will invoke the correct method to handle
     * the JSON-RPC requests.
     */
    ::json::rpc::JSONRPCManager *m_rpcManager;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_JSONRPCMETHODHANDLER_H_ */
