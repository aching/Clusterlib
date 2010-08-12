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

namespace clusterlib {

void 
JSONRPCMethodHandler::handleUserEvent(Event e)
{
    TRACE(CL_LOG, "handleUserEvent");
        
    if (m_rpcManager->getRecvQueue() == NULL) {
        throw InconsistentInternalStateException(
            "handleUserEvent: No receiving queue exists!!!");
        return;
    }
    
    if (m_rpcManager->getRecvQueue()->empty()) {
        LOG_DEBUG(CL_LOG, 
                  "handleUserEvent: Empty receiving queue on event %u",
                  e);
        return;
    }
    
    string request;
    bool found = m_rpcManager->getRecvQueue()->takeWaitMsecs(
        recvQueueMsecTimeOut, request);
    if (!found) {
        LOG_DEBUG(CL_LOG, 
                  "handleUserEvent: Waited %" PRIu64 " msecs and "
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
              m_rpcManager->getRoot()->getKey().c_str(),
              m_rpcManager->getCompletedQueue()->getKey().c_str());
    m_rpcManager->invokeAndResp(request);
}

}
