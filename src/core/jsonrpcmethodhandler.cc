/*
 * jsonrpcmethodhandler.cc --
 *
 * Implementation of the JSONRPCMethodHandler class.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace clusterlib;
using namespace json;
using namespace json::rpc;

/* Wait up to 1 second for a queue element */
static const uint64_t recvQueueMsecTimeOut = 1000;

namespace clusterlib
{

void 
JSONRPCMethodHandler::handleUserEvent(Event e)
{
    TRACE(CL_LOG, "handleUserEvent");
        
    if (m_recvQueue == NULL) {
        throw InconsistentInternalStateException(
            "handleUserEvent: No receiving queue exists!!!");
        return;
    }
    
    if (m_recvQueue->empty()) {
        LOG_DEBUG(CL_LOG, 
                  "handleUserEvent: Empty receiving queue on event %u",
                  e);
        return;
    }
    
    string request;
    bool found = m_recvQueue->takeWaitMsecs(recvQueueMsecTimeOut, request);
    if (!found) {
        LOG_DEBUG(CL_LOG, 
                  "handleUserEvent: Waited %llu msecs and "
                  "couldn't find any elements",
                  recvQueueMsecTimeOut);
        return;
    }
    
    /*
     * Do the requests for method registered in the RPC Manager
     *
     * Handling return value:
     *
     * If DEFAULT_RESP_QUEUE exists and is valid, write result to
     * that queue.  Otherwise write result to DEFAULT_COMPLETED_QUEUE.
     *
     */
    LOG_DEBUG(CL_LOG,
              "handleUserEvent: Got request (%s) and invoking on root"
              " (%s), with completed queue (%s)", 
              request.c_str(),
              m_root->getKey().c_str(),
              m_completedQueue->getKey().c_str());
    m_rpcManager->invokeAndResp(request,
                                m_root,
                                m_completedQueue);
}

}
