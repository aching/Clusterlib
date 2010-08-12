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
using namespace boost;
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
    m_rootSP = m_client->getRoot();
    m_activeNodeGroupSP = m_rootSP->getApplication(groupVec[0], 
                                                   CREATE_IF_NOT_FOUND);
    for (size_t i = 1; i < m_params.getGroupVec().size(); i++) {
        m_activeNodeGroupSP = m_activeNodeGroupSP->getGroup(
            groupVec[i], CREATE_IF_NOT_FOUND);
    }

    string nodeName = m_params.getNodeName();

    /* 
     * Start updating the node health and setting up the ProcessSlot
     * objects as soon as ownership is acquired.
     */
    Mutex activeNodeMutex;
    m_activeNodeSP = 
        m_activeNodeGroupSP->getNode(nodeName, CREATE_IF_NOT_FOUND);
    if (m_params.getEnableNodeOwnership()) {
        m_activeNodeSP->acquireOwnership();
    }
    m_activeNodePeriodicCheck = 
        new ActiveNodePeriodicCheck(m_params.getCheckMsecs(), 
                                    m_activeNodeSP,
                                    m_predMutexCond);
    m_periodicVec.push_back(m_activeNodePeriodicCheck);
    m_factory->registerPeriodicThread(*m_activeNodePeriodicCheck);
    
    /* Get rid of all the previous ProcessSlot objects */
    NameList nl = m_activeNodeSP->getProcessSlotNames();
    shared_ptr<ProcessSlot> processSlotSP;
    for (size_t i = 0; i < nl.size(); i++) {
        processSlotSP =  m_activeNodeSP->getProcessSlot(nl[i],
                                                        LOAD_FROM_REPOSITORY);
        if (processSlotSP != NULL) {
            processSlotSP->remove(true);
        }
    }
    int32_t maxProcessSlots = m_params.getNumProcs();

    {
        NotifyableLocker l(m_activeNodeSP);

        m_activeNodeSP->cachedProcessSlotInfo().setMaxProcessSlots(
            maxProcessSlots);
        m_activeNodeSP->cachedProcessSlotInfo().publish();
    }

    Periodic *processUpdater = NULL;
    for (int32_t i = 0; i < m_params.getNumProcs(); i++) {
        ostringstream oss;
        oss << "slot_" << i;
        processSlotSP = m_activeNodeSP->getProcessSlot(oss.str(), 
                                                       CREATE_IF_NOT_FOUND);
        processUpdater = 
            new ProcessSlotUpdater(ProcessSlotUpdaterFrequency, processSlotSP);
        m_periodicVec.push_back(processUpdater);
        m_factory->registerPeriodicThread(*processUpdater);
    }
}

ActiveNode::~ActiveNode()
{    
    if (m_params.getEnableNodeOwnership()) {
        m_activeNodeSP->releaseOwnership();
    }
}

const shared_ptr<Node> &
ActiveNode::getActiveNode()
{
    TRACE(CL_LOG, "getActiveNode");
    
    return m_activeNodeSP;
}

const shared_ptr<Root> &
ActiveNode::getRoot()
{
    TRACE(CL_LOG, "getRoot");

    return m_rootSP;
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

    {
        NotifyableLocker l(m_activeNodeSP);

        m_activeNodeSP->cachedProcessSlotInfo().setEnable(true);
        m_activeNodeSP->cachedProcessSlotInfo().publish();
    }

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
