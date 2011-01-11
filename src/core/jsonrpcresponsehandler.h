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

#ifndef _CL_JSONRPCRESPONSEHANDLER_H_
#define _CL_JSONRPCRESPONSEHANDLER_H_

namespace clusterlib {

/**
 * Handle JSON-RPC responses in the response queue for Clusterlib.
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
