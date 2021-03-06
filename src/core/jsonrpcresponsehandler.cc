/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace clusterlib;
using namespace json;
using namespace json::rpc;

/* Wait up to 0.5 seconds for a queue element */
static const int64_t respQueueMsecTimeOut = 500;

namespace clusterlib {

void 
JSONRPCResponseHandler::handleUserEvent(Event e)
{
    TRACE(CL_LOG, "handleUserEvent");

    if (m_respQueueSP == NULL) {
        throw InconsistentInternalStateException(
            "handleUserEvent: No response queue exists!!!");
        return;
    }
    
    if (m_respQueueSP->empty()) {
        LOG_DEBUG(CL_LOG, 
                  "handleUserEvent: Empty response queue on event %u",
                  e);
        return;
    }
    
    JSONValue jsonValue;
    JSONValue::JSONObject respObj;
    string response;
    LOG_DEBUG(CL_LOG, "handleUserEvent: Starting to take");
    /* Process all responses in the queue that make the timeout */
    bool found = m_respQueueSP->takeWaitMsecs(respQueueMsecTimeOut, response);
    while (found == true) {
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
        catch (const json::Exception &ex) {
            LOG_WARN(CL_LOG,
                     "handleUserEvent: Got non-valid JSON-RPC data (%s), "
                     "moving to completed queue (%s)",
                     response.c_str(),
                     m_completedQueueSP->getKey().c_str());
            m_completedQueueSP->put(response);
        }
        
        /* Try to get the next response. */
        found = m_respQueueSP->takeWaitMsecs(respQueueMsecTimeOut, response);
    }
}

}
