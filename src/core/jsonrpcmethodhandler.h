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

#ifndef _CL_JSONRPCMETHODHANDLER_H_
#define _CL_JSONRPCMETHODHANDLER_H_

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
                         int32_t completedQueueMaxSize,
                         PropertyList *methodStatusPropertyList,
                         Root *root,
                         ::json::rpc::JSONRPCManager *rpcManager)
        : UserEventHandler(recvQueue, EN_QUEUECHILDCHANGE, NULL, true),
          m_recvQueue(recvQueue),
          m_completedQueue(completedQueue),
          m_completedQueueMaxSize(completedQueueMaxSize),
          m_root(root),
          m_methodStatusPropertyList(methodStatusPropertyList),
          m_rpcManager(rpcManager) {}
    
    virtual void handleUserEvent(Event e);

  private:
    /**
     * The receiving queue for JSON-RPC requests.
     */
    Queue *m_recvQueue;

    /**
     * The completed queue for processed JSON-RPC requests that have
     * completed, there is no response queue, or that are unable to be
     * decoded,.  It should always be limited to a maximum size.
     */
    Queue *m_completedQueue;

    /**
     * The maximum size of the number of processed JSON-RPC requests
     * in the m_completedQueue.  -1 indicates infinity, 0 indicates none.
     */
    int32_t m_completedQueueMaxSize;

    /**
     * The clusterlib root pointer.
     */
    Root *m_root;

    /**
     * If != NULL, this is the place where the requests and their
     * statuses are posted.
     */
    PropertyList *m_methodStatusPropertyList;

    /**
     * The RPC manager that will invoke the correct method to handle
     * the JSON-RPC requests.
     */
    ::json::rpc::JSONRPCManager *m_rpcManager;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_JSONRPCMETHODHANDLER_H_ */
