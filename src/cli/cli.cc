/*
 * cli.cc --
 *
 * Implementation of the command line interface
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include <sys/utsname.h>
#include <string.h>
#include <sys/wait.h>
#include "clusterlibinternal.h"
#include "cliparams.h"
#include "cliformat.h"
#include "generalcommands.h"

using namespace std;
using namespace clusterlib;
using namespace json;
using namespace json::rpc;

/**
 * The command line interface may be used in a shell-like environment
 * or to execute a stand alone command.
 */
CliParams *CliParams::m_params = NULL;
int main(int argc, char* argv[]) 
{
    CliParams *params = CliParams::getInstance();

    /* Parse the arguments */
    params->parseArgs(argc, argv);
    
    /* 
     * Force the log level to be set to 0, special case for a command.
     */
    SetLogLevel *setLogLevelCommand = new SetLogLevel;
    vector<string> logLevelArgVec;
    stringstream logLevelSs;
    logLevelSs << params->getLogLevel();
    logLevelArgVec.push_back(logLevelSs.str());
    setLogLevelCommand->setArgVec(vector<string>(logLevelArgVec));
    setLogLevelCommand->action();

    /*
     * Initialize the factory
     */
    params->initFactoryAndClient();
                                  
    /*
     * Enable the JSON-RPC response handler and create the appropriate
     * response queue for this client.
     */
    Root *root = params->getClient()->getRoot();
    Application *cliApp = root->getApplication(
        ClusterlibStrings::DEFAULT_CLI_APPLICATION, true);
    Queue *respQueue = cliApp->getQueue(Factory::getHostnamePidTid() + 
        ClusterlibStrings::DEFAULT_RESP_QUEUE, true);
    string respQueueKey = respQueue->getKey();
    Queue *completedQueue = cliApp->getQueue(Factory::getHostnamePidTid() + 
        ClusterlibStrings::DEFAULT_COMPLETED_QUEUE, true);
    string completedQueueKey = completedQueue->getKey();    
    params->getFactory()->createJSONRPCResponseClient(respQueue,
                                                      completedQueue);

    /* Register the commands after connecting */
    params->registerCommand(setLogLevelCommand);
    params->registerCommand(new RemoveNotifyable(params->getClient()));
    params->registerCommand(new GetChildren(params->getClient()));
    params->registerCommand(new GetLockBids(params->getClient()));
    params->registerCommand(new GetAttributes(params->getClient()));
    params->registerCommand(new AddApplication(params->getClient()));
    params->registerCommand(new AddGroup(params->getClient()));
    params->registerCommand(new AddDataDistribution(params->getClient()));
    params->registerCommand(new AddNode(params->getClient()));
    params->registerCommand(new AddPropertyList(params->getClient()));
    params->registerCommand(new AddQueue(params->getClient()));
    params->registerCommand(new JSONRPCCommand(params->getClient(), 
                                               respQueue));
    params->registerCommand(new Help(params));
    params->registerCommand(new Quit(params));
    
    /* Register the arguments */
    params->registerCommand(new BoolArg());
    params->registerCommand(new IntegerArg());
    params->registerCommand(new StringArg());
    params->registerCommand(new NotifyableArg(params->getClient()));

    /* Keep getting commands until done. */
    while (!params->finished()) {
        params->parseAndRunLine();
    }
    
    /* Clean up */
    map<string, CliCommand *>::iterator it;
    for (it = params->getCommandMap()->begin(); 
         it != params->getCommandMap()->end();
         ++it) {
        delete it->second;
    }

    /* Clean up the Factory to clear out all the events and clients */
    delete params->getFactory();

    /* Initialize the factory again to clean up our objects */
    params->initFactoryAndClient();
    root = params->getClient()->getRoot();
    respQueue = dynamic_cast<Queue *>(
        root->getNotifyableFromKey(respQueueKey));
    if (respQueue != NULL) {
        respQueue->remove();
    }
    else {
        cerr << "Couldn't find response queue: " << respQueueKey << endl;
    }
    completedQueue = dynamic_cast<Queue *>(
        root->getNotifyableFromKey(completedQueueKey));
    if (completedQueue != NULL) {
        completedQueue->remove();
    }
    else {
        cerr << "Couldn't find completed queue: " << completedQueueKey << endl;
    }
    delete params->getFactory();

    return 0;
}
