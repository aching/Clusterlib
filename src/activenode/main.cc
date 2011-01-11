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

#include <iostream>
#include "clusterlib.h"
#include "activenodeparams.h"
#include "activenode.h"

using namespace std;
using namespace boost;
using namespace clusterlib;
using namespace json;
using namespace json::rpc;
using namespace activenode;

/**
 * The maximum size of the completed queue.
 */
static const int32_t completedQueueMaxSize = 10;

int main(int argc, char* argv[]) 
{
    ActiveNodeParams params;
    params.parseArgs(argc, argv);

    auto_ptr<Factory> factory(new Factory(params.getZkServerPortList()));
    ActiveNode activeNode(params, factory.get());
    shared_ptr<Queue> recvQueueSP = activeNode.getActiveNode()->getQueue(
        CLString::DEFAULT_RECV_QUEUE, 
        CREATE_IF_NOT_FOUND);
    shared_ptr<Queue> completedQueueSP = activeNode.getActiveNode()->getQueue(
        CLString::DEFAULT_COMPLETED_QUEUE, 
        CREATE_IF_NOT_FOUND);
    shared_ptr<PropertyList> rpcMethodHandlerPropertyListSP = 
        activeNode.getActiveNode()->getPropertyList(
            CLString::DEFAULT_PROPERTYLIST,
            CREATE_IF_NOT_FOUND);
    /* Try to clear the PropertyList if possible wihin 0.5 seconds */
    {
        NotifyableLocker l(rpcMethodHandlerPropertyListSP,
                           CLString::NOTIFYABLE_LOCK,
                           DIST_LOCK_EXCL,
                           500);
        if (l.hasLock()) {
            rpcMethodHandlerPropertyListSP->cachedKeyValues().clear();
            rpcMethodHandlerPropertyListSP->cachedKeyValues().publish();
        }        
    }

    auto_ptr<ClusterlibRPCManager> rpcManager(
        new ClusterlibRPCManager(activeNode.getRoot(),
                                 recvQueueSP, 
                                 completedQueueSP, 
                                 completedQueueMaxSize, 
                                 rpcMethodHandlerPropertyListSP));
    /*
     * Deliberately added an ClusterlibRPCManager that has no methods
     * as an example.  Create and register methods if desired.  For
     * example
     * 
     * rpcManager->registerMethod(...
     */
    vector<ClusterlibRPCManager *> rpcManagerVec;
    rpcManagerVec.push_back(rpcManager.get());
    return activeNode.run(rpcManagerVec);
}
