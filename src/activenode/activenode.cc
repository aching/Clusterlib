/*
 * activenode.cc --
 *
 * Implementation of the activenode process
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
#include "activenodeparams.h"
#include "activenodejsonrpcadaptor.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace clusterlib;
using namespace json;
using namespace json::rpc;

static const size_t hostnameSize = 255;
/* Wait 2 seconds for a queue element */
static const uint64_t commandQueueTimeOut = 2000;
volatile bool shutdown = false;

/**
 * Start/stop process handler 
 */
class ProcessHandler : public UserEventHandler
{
  public:
    ProcessHandler(Notifyable *np,
                   Event mask,
                   ClientData cd,
                   Mutex *globalMutex) 
        : UserEventHandler(np, mask, cd), 
          m_globalMutex(globalMutex) {}

    virtual void handleUserEvent(Event e)
    {
        TRACE(CL_LOG, "handleUserEvent");

        ProcessSlotImpl *processSlot = 
            dynamic_cast<ProcessSlotImpl *>(getNotifyable());
        if (processSlot == NULL) {
            LOG_WARN(CL_LOG, 
                     "handleUserEvent: No process slot for this event!");
            return;
        }
           
        ProcessSlot::ProcessState desiredProcessState = 
            processSlot->getDesiredProcessState();
        ProcessSlot::ProcessState currentProcessState = 
            processSlot->getCurrentProcessState();
        
        if (desiredProcessState == ProcessSlot::RUNNING) {
            if ((currentProcessState == ProcessSlot::RUNNING) ||
                (currentProcessState == ProcessSlot::STARTED)) {
                LOG_WARN(CL_LOG, "handleUserEvent: Already running");
                return;
            }

            processSlot->setCurrentProcessState(ProcessSlot::STARTED);
            pid_t pid = processSlot->startLocal();
            if (pid != -1) {
                processSlot->setCurrentProcessState(ProcessSlot::RUNNING);
                processSlot->setPID(pid);
                
            }
            else {
                processSlot->setCurrentProcessState(ProcessSlot::FAILED);
                processSlot->setPID(-1);
            }
        }
        else if (desiredProcessState == ProcessSlot::STOPPED) {
            if (currentProcessState == ProcessSlot::RUNNING) {
                processSlot->stopLocal();
            }
            else {
                LOG_WARN(CL_LOG, "handleUserEvent: Nothing to stop!");
            }
        }
    }
    virtual ~ProcessHandler() {}

  private:
    Mutex *m_globalMutex;
};


/**
 * Handle commands in the receiving queue.
 */
class CommandQueueHandler : public UserEventHandler
{
  public:
    CommandQueueHandler(Queue *commandQueue,
                        Queue *defaultCompletedQueue,
                        Root *root,
                        Event mask,
                        ClientData cd,
                        Mutex *globalMutex)
        : UserEventHandler(commandQueue, mask, cd),
          m_globalMutex(globalMutex),
          m_root(root),
          m_commandQueue(commandQueue),
          m_defaultCompletedQueue(defaultCompletedQueue) {}
    virtual void handleUserEvent(Event e)
    {
        TRACE(CL_LOG, "handleUserEvent");

        if (m_commandQueue == NULL) {
            throw InconsistentInternalStateException(
                "handleUserEvent: No command queue exists!!!");
            return;
        }

        if (m_commandQueue->empty()) {
            LOG_DEBUG(CL_LOG, 
                      "handleUserEvent: Empty command queue on event %u",
                      e);
            return;
        }

        bool timedOut = true;
        string command = m_commandQueue->take(commandQueueTimeOut, &timedOut);
        if (timedOut) {
            LOG_DEBUG(CL_LOG, 
                      "handleUserEvent: Waited %llu msecs and "
                      "couldn't find any elements",
                      commandQueueTimeOut);
            return;
        }

        /*
         * Do the commands commands registered in the RPC Manager
         *
         * Handling return value:
         *
         * If DEFAULT_RESP_QUEUE exists and is valid, write result to
         * that queue.  Otherwise write result to DEFAULT_COMPLETED_QUEUE.
         *
         */
        LOG_DEBUG(CL_LOG,
                  "handleUserEvent: Got command (%s)", 
                  command.c_str());
        JSONRPCManager *rpcManager = JSONRPCManager::getInstance();
        rpcManager->invokeAndResp(command,
                                  m_root,
                                  m_defaultCompletedQueue);
    }

  private:
    Mutex *m_globalMutex;
    Root *m_root;
    Queue *m_commandQueue;
    Queue *m_defaultCompletedQueue;
};

int main(int argc, char* argv[]) 
{
    ActiveNodeParams params;
    params.parseArgs(argc, argv);

    JSONRPCManager *rpcManager = JSONRPCManager::getInstance();
    
    auto_ptr<Factory> factory(new Factory(params.getZkServerPortList()));
    Client *client = factory->createClient();
    Root *root = client->getRoot();

    /* Create the RPC adaptor and register available methods. */
    auto_ptr<ActiveNodeJSONRPCAdaptor> jsonRpcAdaptor(
        new ActiveNodeJSONRPCAdaptor(client));
    rpcManager->registerMethod(ClusterlibStrings::RPC_START_PROCESS,
                               jsonRpcAdaptor.get());
    rpcManager->registerMethod(ClusterlibStrings::RPC_STOP_PROCESS,
                               jsonRpcAdaptor.get());

    vector<string> groupVec = params.getGroupsVec();
    if (groupVec.size() <= 0) {
        LOG_FATAL(CL_LOG, "No groups found in the group vector");
        return -1;
    }

    /* Go to the group to add the node */
    Group *group = root->getApplication(groupVec[0], true);
    for (size_t i = 1; i < params.getGroupsVec().size(); i++) {
        group = group->getGroup(groupVec[i], true);
    }

    struct utsname uts;
    int ret = uname(&uts);
    if (ret == -1) {
        LOG_FATAL(CL_LOG, "Couldn't get hostname: %s",
                  strerror(errno));
        return -1;
    }
    string hostnameString(uts.nodename);

    /* Setup all the event handlers to start processes or shutdown and
     * activate the node */
    Mutex activeNodeMutex;
    Node *activeNode = group->getNode(hostnameString, true);
    activeNode->acquireLock();
    
    /* Get rid of all the previous processes */
    NameList nl = activeNode->getProcessSlotNames();
    ProcessSlot *processSlot = NULL;
    for (size_t i = 0; i < nl.size(); i++) {
        processSlot =  activeNode->getProcessSlot(nl[i]);
        if (processSlot != NULL) {
            processSlot->remove();
        }
    }
    activeNode->setMaxProcessSlots(params.getNumProcs());

    vector<UserEventHandler *> handlerVec;
    stringstream ss;
    for (int32_t i = 0; i < params.getNumProcs(); i++) {
        ss.str("");
        ss << "slot_" << i;
        processSlot = activeNode->getProcessSlot(ss.str(), true);
        UserEventHandler *handler = new ProcessHandler(
            processSlot, 
            EN_PROCESSSLOTDESIREDSTATECHANGE, 
            NULL, 
            &activeNodeMutex);
        handlerVec.push_back(handler);
        /* Start the handler */
        processSlot->getDesiredProcessState();
        client->registerHandler(handler);
    }

    /* Setup the command queue and associated handlers */
    Queue *commandQueue =
        activeNode->getQueue(ClusterlibStrings::DEFAULT_RECV_QUEUE, 
                             true);
    Queue *defaultCompletedQueue =
        activeNode->getQueue(ClusterlibStrings::DEFAULT_COMPLETED_QUEUE, 
                             true);
    UserEventHandler *handler = new CommandQueueHandler(
        commandQueue,
        defaultCompletedQueue,
        root,
        EN_QUEUECHILDCHANGE,
        NULL,
        &activeNodeMutex);
    handlerVec.push_back(handler);
    client->registerHandler(handler);
    activeNode->releaseLock();
    activeNode->setUseProcessSlots(true);

    /* Check if process clean up is needed every so many seconds and
     * update process state. */
    int stat_loc;
    pid_t pid = -1;
    ProcessSlotImpl *processSlotImpl = NULL;
    while (shutdown == false) {
        pid = waitpid(-1, &stat_loc, WNOHANG);
        if (pid != -1) {
            nl = activeNode->getProcessSlotNames();
            for (size_t i = 0; i < nl.size(); i++) {
                processSlotImpl = dynamic_cast<ProcessSlotImpl *>(
                    activeNode->getProcessSlot(nl[i]));
                if (processSlotImpl != NULL) {
                    if (processSlotImpl->getPID() == pid) {
                        processSlotImpl->setCurrentProcessState(
                            ProcessSlot::FINISHED);
                        processSlotImpl->setPID(-1);
                    }
                }
            }
        }
        ::sleep(2);
        LOG_DEBUG(CL_LOG, "waited 2 seconds...");
    }

    /* Clean up */
    vector<UserEventHandler *>::iterator handlerVecIt;
    for (handlerVecIt = handlerVec.begin(); 
         handlerVecIt != handlerVec.end();
         handlerVecIt++) {
        client->cancelHandler(*handlerVecIt);
        delete *handlerVecIt;
    }

    return 0;
}
