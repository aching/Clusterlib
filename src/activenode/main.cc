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
