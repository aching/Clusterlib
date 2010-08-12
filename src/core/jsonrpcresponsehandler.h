/*
 * jsonrpcresponsehandler.cc --
 *
 * Definition of class JSONRPCResponseHandler; it represents a built-in
 * handler for JSON-RPC responses in clusterlib.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#ifndef _CL_JSONRPCRESPONSEHANDLER_H_
#define _CL_JSONRPCRESPONSEHANDLER_H_

namespace clusterlib {

/**
 * Handle JSON-RPC responses in the response queue.
 */
class JSONRPCResponseHandler : public UserEventHandler
{
  public:
    /**
     * Constructor.
     */
    JSONRPCResponseHandler(const boost::shared_ptr<Queue> &respQueueSP,
                           const boost::shared_ptr<Queue> &completedQueueSP,
                           Client *client,
                           SignalMap *responseSignalMap)
        : UserEventHandler(respQueueSP, EN_QUEUECHILDCHANGE, NULL, true),
          m_respQueueSP(respQueueSP),
          m_completedQueueSP(completedQueueSP),
          m_responseSignalMap(responseSignalMap) 
    {
        m_client = dynamic_cast<ClientImpl *>(client);
    }
    
    virtual void handleUserEvent(Event e);

  private:
    /**
     * The response queue for JSON-RPC responses.
     */
    boost::shared_ptr<Queue> m_respQueueSP;

    /**
     * The completed queue for JSON-RPC responses (or errors).
     */
    boost::shared_ptr<Queue> m_completedQueueSP;

    /**
     * The clusterlib client for this handler.
     */
    ClientImpl *m_client;

    /**
     * Signal map pointer for responses
     */
    SignalMap *m_responseSignalMap;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_JSONRPCRESPONSEHANDLER_H_ */
