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

#ifndef _CL_JSONRPCMETHODHANDLER_H_
#define _CL_JSONRPCMETHODHANDLER_H_

namespace clusterlib {

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
