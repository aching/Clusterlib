/*
 * main.cc --
 *
 * Implementation of the main function that uses an activenode object.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include <iostream>
#include "clusterlib.h"
#include "activenodeparams.h"
#include "activenode.h"

using namespace std;
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
    Queue *recvQueue = activeNode.getActiveNode()->getQueue(
        ClusterlibStrings::DEFAULT_RECV_QUEUE, 
        true);
    Queue *completedQueue = activeNode.getActiveNode()->getQueue(
        ClusterlibStrings::DEFAULT_COMPLETED_QUEUE, 
        true);
    PropertyList *rpcMethodHandlerPropertylist = 
        activeNode.getActiveNode()->getPropertyList(
            ClusterlibStrings::DEFAULTPROPERTYLIST,
            true);
    /* Try to clear the property list if possible wihin 0.5 seconds */
    bool gotLock = rpcMethodHandlerPropertylist->acquireLockWaitMsecs(500);
    if (gotLock) {
        rpcMethodHandlerPropertylist->clear();
        rpcMethodHandlerPropertylist->publish();
        rpcMethodHandlerPropertylist->releaseLock();
    }

    auto_ptr<ClusterlibRPCManager> rpcManager(
        new ClusterlibRPCManager(activeNode.getRoot(),
                                 recvQueue, 
                                 completedQueue, 
                                 completedQueueMaxSize, 
                                 rpcMethodHandlerPropertylist));

    /* Create and register available methods. */
    auto_ptr<StartProcessMethod> startProcessMethod(new StartProcessMethod());
    rpcManager->registerMethod(startProcessMethod->getName(),
                               startProcessMethod.get());
    auto_ptr<StopProcessMethod> stopProcessMethod(new StopProcessMethod());
    rpcManager->registerMethod(stopProcessMethod->getName(),
                               stopProcessMethod.get());
    
    vector<ClusterlibRPCManager *> rpcManagerVec;
    rpcManagerVec.push_back(rpcManager.get());
    return activeNode.run(rpcManagerVec);
}
