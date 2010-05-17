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

static string zkCmds = "Zookeeper Commands";
static string clusterlibCmds = "Clusterlib Commands";
static string cliCmds = "CLI Commands";
static string argCmds = "Arguments Commands";

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
        ClusterlibStrings::DEFAULT_CLI_APPLICATION, CREATE_IF_NOT_FOUND);
    Queue *respQueue = cliApp->getQueue(
        ProcessThreadService::getHostnamePidTid() + 
        ClusterlibStrings::DEFAULT_RESP_QUEUE, CREATE_IF_NOT_FOUND);
    string respQueueKey = respQueue->getKey();
    Queue *completedQueue = cliApp->getQueue(
        ProcessThreadService::getHostnamePidTid() + 
        ClusterlibStrings::DEFAULT_COMPLETED_QUEUE, CREATE_IF_NOT_FOUND);
    string completedQueueKey = completedQueue->getKey();    
    Client *jsonRPCResponseClient = 
        params->getFactory()->createJSONRPCResponseClient(respQueue,
                                                          completedQueue);

    /* Register the commands after connecting */
    params->registerCommandByGroup(new GetZnode(params->getFactory(),
                                         params->getClient()),
                                   zkCmds);
    params->registerCommandByGroup(new GetZnodeChildren(params->getFactory(), 
                                                        params->getClient()),
                                   zkCmds);
    
    params->registerCommandByGroup(setLogLevelCommand, clusterlibCmds);
    params->registerCommandByGroup(new RemoveNotifyable(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new GetChildren(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new GetLockBids(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new GetAttributes(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new AddApplication(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new AddGroup(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new AddDataDistribution(
                                       params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new AddNode(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new AddPropertyList(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new AddQueue(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new JSONRPCCommand(params->getClient(), 
                                                      respQueue),
                                   clusterlibCmds);
    params->registerCommandByGroup(new SetCurrentState(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new SetDesiredState(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new StartProcessSlot(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new StopProcessSlot(params->getClient()),
                                   clusterlibCmds);
    params->registerCommandByGroup(new StopActiveNode(params->getClient()),
                                   clusterlibCmds);
    
    params->registerCommandByGroup(new AddAlias(params), cliCmds);
    params->registerCommandByGroup(new RemoveAlias(params), cliCmds);
    params->registerCommandByGroup(new GetAliasReplacement(params), cliCmds);
    params->registerCommandByGroup(new Help(params), cliCmds);
    params->registerCommandByGroup(new Quit(params), cliCmds);
    
    /* Register the arguments */
    params->registerCommandByGroup(new BoolArg(), argCmds);
    params->registerCommandByGroup(new IntegerArg(), argCmds);
    params->registerCommandByGroup(new StringArg(), argCmds);
    params->registerCommandByGroup(new NotifyableArg(params->getClient()),
                                   argCmds);

    /* Keep getting commands until done. */
    while (!params->finished()) {
        params->parseAndRunLine();
    }
    
    /* Clean up */
    params->getFactory()->removeClient(jsonRPCResponseClient);
    map<string, map<string, CliCommand *> >::const_iterator groupCommandMapIt;
    map<string, CliCommand *>::const_iterator commandMapIt;
    for (groupCommandMapIt = params->getGroupCommandMap()->begin();
         groupCommandMapIt != params->getGroupCommandMap()->end();
         ++groupCommandMapIt) {
        for (commandMapIt = groupCommandMapIt->second.begin();
             commandMapIt != groupCommandMapIt->second.end();
             ++commandMapIt) {
            delete commandMapIt->second;
        }
    }

    respQueue->remove();
    completedQueue->remove();

    return 0;
}
