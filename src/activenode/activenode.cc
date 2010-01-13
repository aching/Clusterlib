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
 * Check the health of this node
 */
class NodeHealthChecker : public HealthChecker {
  public:
    NodeHealthChecker(Node *node) 
        : m_node(node) {}

    virtual HealthReport checkHealth() {
        if (m_node->getUseProcessSlots()) {
            return clusterlib::HealthReport(
                clusterlib::HealthReport::HS_HEALTHY, 
                "Process slots can still be used");
        }
        else {
            return clusterlib::HealthReport(
                clusterlib::HealthReport::HS_UNHEALTHY,
                "Process slots not allowed");
        }
    }

  private:
    Node *m_node;
};

int main(int argc, char* argv[]) 
{
    ActiveNodeParams params;
    params.parseArgs(argc, argv);

    auto_ptr<JSONRPCManager> rpcManager(new JSONRPCManager());
    auto_ptr<Factory> factory(new Factory(params.getZkServerPortList()));
    Client *client = factory->createClient();
    Root *root = client->getRoot();

    /* Create and register available methods. */
    auto_ptr<StartProcessMethod> startProcessMethod(
        new StartProcessMethod(client));
    rpcManager->registerMethod(startProcessMethod->getName(),
                               startProcessMethod.get());
    auto_ptr<StopProcessMethod> stopProcessMethod(
        new StopProcessMethod(client));
    rpcManager->registerMethod(stopProcessMethod->getName(),
                               stopProcessMethod.get());

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
    activeNode->initializeConnection(true);
    NodeHealthChecker nodeHealthChecker(activeNode);
    activeNode->registerHealthChecker(&nodeHealthChecker);
    
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

    /* Setup the receiving queue and associated handler */
    Queue *recvQueue =
        activeNode->getQueue(ClusterlibStrings::DEFAULT_RECV_QUEUE, 
                             true);
    Queue *completedQueue =
        activeNode->getQueue(ClusterlibStrings::DEFAULT_COMPLETED_QUEUE, 
                             true);
    factory->createJSONRPCMethodClient(recvQueue,
                                       completedQueue,
                                       rpcManager.get());
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
    activeNode->unregisterHealthChecker();
    vector<UserEventHandler *>::iterator handlerVecIt;
    for (handlerVecIt = handlerVec.begin(); 
         handlerVecIt != handlerVec.end();
         handlerVecIt++) {
        client->cancelHandler(*handlerVecIt);
        delete *handlerVecIt;
    }

    return 0;
}
