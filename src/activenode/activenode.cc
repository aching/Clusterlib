/*
 * activenode.cc --
 *
 * Implementation of the activenode object.
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
#include "activenode.h"

using namespace std;
using namespace clusterlib;
using namespace json;
using namespace json::rpc;

namespace activenode {

static const size_t hostnameSize = 255;

/** 
 * Wait 1 seconds for a queue element 
 */
static const uint64_t commandQueueTimeOut = 1000;

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
           
        ProcessSlot::ProcessState desiredProcessState;
        ProcessSlot::ProcessState currentProcessState; 
        processSlot->acquireLock();
        processSlot->getDesiredProcessState(&desiredProcessState, NULL);
        processSlot->getCurrentProcessState(&currentProcessState, NULL);
        processSlot->releaseLock();

        if (desiredProcessState == ProcessSlot::RUNNING) {
            if ((currentProcessState == ProcessSlot::RUNNING) ||
                (currentProcessState == ProcessSlot::STARTED)) {
                LOG_WARN(CL_LOG, "handleUserEvent: Already started running");
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

    virtual ~NodeHealthChecker() {}

  private:
    Node *m_node;
};

ActiveNode::ActiveNode(const ActiveNodeParams &params, Factory *factory)
    : m_params(params), 
      m_factory(factory) 
{

    vector<string> groupVec = m_params.getGroupsVec();
    if (groupVec.size() <= 0) {
        LOG_FATAL(CL_LOG, "No groups found in the group vector");
        return;
    }

    /* Go to the group to add the node */
    m_client = m_factory->createClient();
    m_root = m_client->getRoot();
    m_activeNodeGroup = m_root->getApplication(groupVec[0], true);
    for (size_t i = 1; i < m_params.getGroupsVec().size(); i++) {
        m_activeNodeGroup = m_activeNodeGroup->getGroup(groupVec[i], true);
    }

    string nodeName = m_params.getNodeName();
    if (nodeName.empty()) {
        struct utsname uts;
        int ret = uname(&uts);
        if (ret == -1) {
            LOG_FATAL(CL_LOG, "Couldn't get hostname: %s",
                  strerror(errno));
        }
        nodeName = uts.nodename;
    }

    /* 
     * Setup all the event handlers to start processes or shutdown and
     * activate the node 
     */
    Mutex activeNodeMutex;
    m_activeNode = m_activeNodeGroup->getNode(nodeName, true);
    m_activeNode->initializeConnection(true);
    m_nodeHealthChecker.reset(new NodeHealthChecker(m_activeNode));
    m_activeNode->registerHealthChecker(&(*(m_nodeHealthChecker.get())));
    
    /* Get rid of all the previous processes */
    NameList nl = m_activeNode->getProcessSlotNames();
    ProcessSlot *processSlot = NULL;
    for (size_t i = 0; i < nl.size(); i++) {
        processSlot =  m_activeNode->getProcessSlot(nl[i]);
        if (processSlot != NULL) {
            processSlot->remove(true);
        }
    }
    m_activeNode->setMaxProcessSlots(m_params.getNumProcs());

    stringstream ss;
    for (int32_t i = 0; i < m_params.getNumProcs(); i++) {
        ss.str("");
        ss << "slot_" << i;
        processSlot = m_activeNode->getProcessSlot(ss.str(), true);
        UserEventHandler *handler = new ProcessHandler(
            processSlot, 
            EN_PROCESSSLOTDESIREDSTATECHANGE, 
            NULL, 
            &activeNodeMutex);
        m_handlerVec.push_back(handler);
        /* Start the handler */
        processSlot->getDesiredProcessState(NULL, NULL);
        m_client->registerHandler(handler);
    }
}

ActiveNode::~ActiveNode()
{    
}

Node *
ActiveNode::getActiveNode()
{
    TRACE(CL_LOG, "getActiveNode");
    
    return m_activeNode;
}

Root *
ActiveNode::getRoot()
{
    TRACE(CL_LOG, "getRoot");

    return m_root;
}

int32_t 
ActiveNode::run(vector<ClusterlibRPCManager *> &rpcManagerVec)
{
    TRACE(CL_LOG, "run");

    /* Setup a RPC method client for each rpcManager */
    vector<Client *> rpcClientVec;
    vector<ClusterlibRPCManager *>::const_iterator rpcManagerVecIt;    
    for (rpcManagerVecIt = rpcManagerVec.begin(); 
         rpcManagerVecIt != rpcManagerVec.end(); 
         ++rpcManagerVecIt) {
        rpcClientVec.push_back(
            m_factory->createJSONRPCMethodClient(*rpcManagerVecIt));
    }
    m_activeNode->setUseProcessSlots(true);

    /*
     * Check if process clean up is needed every so many seconds and
     * update process state. 
     */
    int stat_loc;
    pid_t pid = -1;
    ProcessSlotImpl *processSlotImpl = NULL;
    bool shutdown = false;
    NameList nl;
    do {
        pid = waitpid(-1, &stat_loc, WNOHANG);
        if (pid != -1) {
            nl = m_activeNode->getProcessSlotNames();
            for (size_t i = 0; i < nl.size(); i++) {
                processSlotImpl = dynamic_cast<ProcessSlotImpl *>(
                    m_activeNode->getProcessSlot(nl[i]));
                if (processSlotImpl != NULL) {
                    if (processSlotImpl->getPID() == pid) {
                        processSlotImpl->setCurrentProcessState(
                            ProcessSlot::FINISHED);
                        processSlotImpl->setPID(-1);
                        /* 
                         * If the desired state is RUNNING, then
                         * restart it if it stopped
                         */
                        ProcessSlot::ProcessState desiredState;
                        processSlotImpl->acquireLock();
                        processSlotImpl->getDesiredProcessState(
                            &desiredState, NULL);
                        LOG_INFO(CL_LOG,
                                 "run: PID %d died, desired state = %s",
                                 pid,
                                 ProcessSlot::getProcessStateAsString(
                                     desiredState).c_str());
                        if (desiredState == ProcessSlot::RUNNING) {
                            processSlotImpl->start();
                        }
                        LOG_INFO(
                            CL_LOG,
                            "run: Restarting the process with "
                            "exec args %s",
                            JSONCodec::encode(
                                processSlotImpl->getJsonExecArgs()).c_str());
                        processSlotImpl->releaseLock();
                    }
                }
            }
        }
        shutdown = m_predMutexCond.predWaitMsecs(1000);
    } while (shutdown == false);

    /* Clean up */
    LOG_DEBUG(CL_LOG, "run: Cleaning up...");
    m_activeNode->unregisterHealthChecker();
    vector<Client *>::iterator rpcClientVecIt;
    for (rpcClientVecIt = rpcClientVec.begin(); 
         rpcClientVecIt != rpcClientVec.end(); 
         ++rpcClientVecIt) {
        m_factory->removeClient(*rpcClientVecIt);
    }
    vector<UserEventHandler *>::iterator handlerVecIt;
    for (handlerVecIt = m_handlerVec.begin(); 
         handlerVecIt != m_handlerVec.end();
         handlerVecIt++) {
        m_client->cancelHandler(*handlerVecIt);
        delete *handlerVecIt;
    }
    
    return 0;
}

}
