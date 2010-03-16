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
    JSONRPCMethodHandler(ClusterlibRPCManager *rpcManager)
        : UserEventHandler(rpcManager->getRecvQueue(), 
                           EN_QUEUECHILDCHANGE, 
                           NULL,
                           true),
          m_rpcManager(rpcManager) {}
    
    virtual void handleUserEvent(Event e);

  private:
    /**
     * The RPC manager that will invoke the correct method to handle
     * the JSON-RPC requests.
     */
    ClusterlibRPCManager *m_rpcManager;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_JSONRPCMETHODHANDLER_H_ */
