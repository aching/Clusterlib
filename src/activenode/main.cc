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

int main(int argc, char* argv[]) 
{
    ActiveNodeParams params;
    params.parseArgs(argc, argv);

    auto_ptr<JSONRPCManager> rpcManager(new JSONRPCManager());
    auto_ptr<Factory> factory(new Factory(params.getZkServerPortList()));
    ActiveNode activeNode(params, factory.get());
    Client *client = factory->createClient();

    /* Create and register available methods. */
    auto_ptr<StartProcessMethod> startProcessMethod(
        new StartProcessMethod(client));
    rpcManager->registerMethod(startProcessMethod->getName(),
                               startProcessMethod.get());
    auto_ptr<StopProcessMethod> stopProcessMethod(
        new StopProcessMethod(client));
    rpcManager->registerMethod(stopProcessMethod->getName(),
                               stopProcessMethod.get());
    
    vector<JSONRPCManager *> rpcManagerVec;
    rpcManagerVec.push_back(rpcManager.get());
    return activeNode.run(rpcManagerVec);
}
