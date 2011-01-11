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

#ifndef _CL_CLUSTERLIBRPCREQUESTMANAGER_H_
#define _CL_CLUSTERLIBRPCREQUESTMANAGER_H_
 
namespace clusterlib  {

/**
 * Manages mulitple ClusterlibRPCRequest objects.
 */ 
class ClusterlibRPCBulkRequest
{
  public:
    /**
     * Add a request to the object.  The allocated memory is now owned
     * by this object and will be released during destruction.
     *
     * @param request The heap-allocated request now owned by this object.
     */
    void addRequest(ClusterlibRPCRequest *request);

    /**
     * Send all the requests.  User may have also chosen to have sent
     * the requests themselves and then only called waitAll().
     */
    void sendAll();

    /**
     * Wait for all the responses.  Can be called multiple times after
     * the requests have been sent.
     *
     * @param maxWaitMsecs The maximum msecs to wait for all the responses.
     * @param perRequestWaitMsecs The time to wait for each request (0 
     *        means no wait)
     * @param pollIntervalMsecs Msecs to wait in between retrying all responses
     * @param method If set, will update the status.
     * @return The number of remaining requests (0 is done)
     */
    uint32_t waitAll(int64_t maxWaitMsecs, 
                     int64_t perRequestWaitMsecs,
                     int64_t pollIntervalMsecs,
                     ClusterlibRPCMethod *method = NULL);

    /**
     * Destructor.
     */
    ~ClusterlibRPCBulkRequest();

  private:
    /**
     * Thread safe object.
     */
    RdWrLock m_rdWrLock;

    /**
     * The vector of requests.
     */
    std::vector<ClusterlibRPCRequest *> m_requestVec;
};

}

#endif
