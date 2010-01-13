/*
 * jsonrpcresponsehandler.cc --
 *
 * Implementation of the JSONRPCResponseHandler class.
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

/* Wait up to 0.1 seconds for a queue element */
static const uint64_t respQueueTimeOut = 100;

namespace clusterlib
{

void 
JSONRPCResponseHandler::handleUserEvent(Event e)
{
    TRACE(CL_LOG, "handleUserEvent");
        
    if (m_respQueue == NULL) {
        throw InconsistentInternalStateException(
            "handleUserEvent: No response queue exists!!!");
        return;
    }
    
    if (m_respQueue->empty()) {
        LOG_DEBUG(CL_LOG, 
                  "handleUserEvent: Empty response queue on event %u",
                  e);
        return;
    }
    
    bool timedOut = false;
    JSONValue jsonValue;
    JSONValue::JSONObject respObj;
    string response;
    LOG_DEBUG(CL_LOG, "handleUserEvent: Starting to take");
    /* Process all responses in the queue that make the timeout */
    response = m_respQueue->take(respQueueTimeOut, &timedOut);
    while (timedOut == false) {
        LOG_DEBUG(CL_LOG,
                  "handleUserEvent: Got response (%s)", 
                  response.c_str());
        try {
            jsonValue = JSONCodec::decode(response);
            /*
             * For any response, find the request and mark it completed
             * (possibly notifying the client).  Put the response in the
             * ClientImpl.
             */
            respObj = jsonValue.get<JSONValue::JSONObject>();
            m_client->getOps()->setIdResponse(
                respObj["id"].get<JSONValue::JSONString>(),
                respObj);
            m_client->getOps()->getResponseSignalMap()->signalPredMutexCond(
                respObj["id"].get<JSONValue::JSONString>());
        }
        catch (const JSONException &ex) {
            LOG_WARN(CL_LOG,
                     "handleUserEvent: Got non-valid JSON-RPC data (%s), "
                     "moving to completed queue (%s)",
                     response.c_str(),
                     m_completedQueue->getKey().c_str());
            m_completedQueue->put(response);
        }
        
        /* Try to get the next response. */
        response = m_respQueue->take(respQueueTimeOut, &timedOut);
    }
}

}
