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
#include "activenodeperiodiccheck.h"
#include "processslotupdater.h"
#include "activenodeparams.h"
#include "activenode.h"

using namespace std;
using namespace clusterlib;
using namespace json;
using namespace json::rpc;

namespace activenode {

static const size_t hostnameSize = 255;

/**
 * Run check every 2 seconds for each ProcessSlot.
 */
static const int64_t ProcessSlotUpdaterFrequency = 2 * 1000;

ActiveNode::ActiveNode(const ActiveNodeParams &params, Factory *factory)
    : m_params(params), 
      m_factory(factory) 
{

    vector<string> groupVec = m_params.getGroupVec();
    if (groupVec.size() <= 0) {
        LOG_FATAL(CL_LOG, "No groups found in the group vector");
        return;
    }

    /* Go to the group to add the node */
    m_client = m_factory->createClient();
    m_root = m_client->getRoot();
    m_activeNodeGroup = m_root->getApplication(groupVec[0], 
                                               CREATE_IF_NOT_FOUND);
    for (size_t i = 1; i < m_params.getGroupVec().size(); i++) {
        m_activeNodeGroup = m_activeNodeGroup->getGroup(groupVec[i], 
                                                        CREATE_IF_NOT_FOUND);
    }

    string nodeName = m_params.getNodeName();

    /* 
     * Start updating the node health and setting up the ProcessSlot
     * objects as soon as ownership is acquired.
     */
    Mutex activeNodeMutex;
    m_activeNode = m_activeNodeGroup->getNode(nodeName, CREATE_IF_NOT_FOUND);
    if (m_params.getEnableNodeOwnership()) {
        m_activeNode->acquireOwnership();
    }
    m_activeNodePeriodicCheck = 
        new ActiveNodePeriodicCheck(m_params.getCheckMsecs(), 
                                    m_activeNode,
                                    m_predMutexCond);
    m_periodicVec.push_back(m_activeNodePeriodicCheck);
    m_factory->registerPeriodicThread(*m_activeNodePeriodicCheck);
    
    /* Get rid of all the previous ProcessSlot objects */
    NameList nl = m_activeNode->getProcessSlotNames();
    ProcessSlot *processSlot = NULL;
    for (size_t i = 0; i < nl.size(); i++) {
        processSlot =  m_activeNode->getProcessSlot(nl[i]);
        if (processSlot != NULL) {
            processSlot->remove(true);
        }
    }
    int32_t maxProcessSlots = m_params.getNumProcs();

    m_activeNode->acquireLock();

    m_activeNode->cachedProcessSlotInfo().setMaxProcessSlots(maxProcessSlots);
    m_activeNode->cachedProcessSlotInfo().publish();

    m_activeNode->releaseLock();
    
    Periodic *processUpdater = NULL;
    for (int32_t i = 0; i < m_params.getNumProcs(); i++) {
        ostringstream oss;
        oss << "slot_" << i;
        processSlot = m_activeNode->getProcessSlot(oss.str(), 
                                                   CREATE_IF_NOT_FOUND);
        processUpdater = 
            new ProcessSlotUpdater(ProcessSlotUpdaterFrequency, processSlot);
        m_periodicVec.push_back(processUpdater);
        m_factory->registerPeriodicThread(*processUpdater);
    }
}

ActiveNode::~ActiveNode()
{    
    if (m_params.getEnableNodeOwnership()) {
        m_activeNode->releaseOwnership();
    }
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

    m_activeNode->acquireLock();

    m_activeNode->cachedProcessSlotInfo().setEnable(true);
    m_activeNode->cachedProcessSlotInfo().publish();

    m_activeNode->releaseLock();

    /* Wait until a signal to stop */
    m_predMutexCond.predWaitMsecs(-1);

    LOG_INFO(CL_LOG, "run: Cleaning up...");
    
    vector<Periodic *>::iterator periodicVecIt;
    for (periodicVecIt = m_periodicVec.begin();
         periodicVecIt != m_periodicVec.end();
         ++periodicVecIt) {
        m_factory->cancelPeriodicThread(*(*periodicVecIt));
        delete *periodicVecIt;
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

Periodic *
ActiveNode::getActiveNodePeriodicCheck()
{
    return m_activeNodePeriodicCheck;
}

}
