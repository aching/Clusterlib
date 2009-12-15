/*
 * cachedobjectchangehandlers.cc --
 *
 * Implementation of the cache change handlers.  As a rule, none of
 * the handlers are allowed to hold a distributed lock.  This would
 * prevent them from completing.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace boost;

namespace clusterlib
{

string 
CachedObjectChangeHandlers::getCachedObjectChangeString(
    CachedObjectChange change)
{
    switch (change) {
        case NOTIFYABLE_STATE_CHANGE:
            return "NOTIFYABLE_STATE_CHANGE";
        case APPLICATIONS_CHANGE:
            return "APPLICATIONS_CHANGE";
        case GROUPS_CHANGE:
            return "GROUPS_CHANGE";
        case DATADISTRIBUTIONS_CHANGE:
            return "DATADISTRIBUTIONS_CHANGE";
        case NODES_CHANGE:
            return "NODES_CHANGE";
        case PROCESSSLOTS_CHANGE:
            return "PROCESSSLOTS_CHANGE";
        case PROCESSSLOTS_USAGE_CHANGE:
            return "PROCESSSLOTS_USAGE_CHANGE";
        case PROCESSSLOT_PORTVEC_CHANGE:
            return "PROCESSSLOT_PORTVEC_CHANGE";
        case PROCESSSLOT_EXECARGS_CHANGE:
            return "PROCESSSLOT_EXECARGS_CHANGE";
        case PROCESSSLOT_RUNNING_EXECARGS_CHANGE:
            return "PROCESSSLOT_RUNNING_EXECARGS_CHANGE";
        case PROCESSSLOT_PID_CHANGE:
            return "PROCESSSLOT_PID_CHANGE";
        case PROCESSSLOT_DESIRED_STATE_CHANGE:
            return "PROCESSSLOT_DESIRED_STATE_CHANGE";
        case PROCESSSLOT_CURRENT_STATE_CHANGE:
            return "PROCESSSLOT_CURRENT_STATE_CHANGE";
        case PROCESSSLOT_RESERVATION_CHANGE:
            return "PROCESSSLOT_RESERVATION_CHANGE";
        case PROPERTYLISTS_CHANGE:
            return "PROPERTYLISTS_CHANGE";
        case PROPERTYLIST_VALUES_CHANGE:
            return "PROPERTYLIST_VALUES_CHANGE";
        case SHARDS_CHANGE:
            return "SHARDS_CHANGE";
        case NODE_CLIENT_STATE_CHANGE:
            return "NODE_CLIENT_STATE_CHANGE";
        case NODE_MASTER_SET_STATE_CHANGE:
            return "NODE_MASTER_SET_STATE_CHANGE";
        case NODE_CONNECTION_CHANGE:
            return "NODE_CONNECTION_CHANGE";
        case SYNCHRONIZE_CHANGE:
            return "SYNCHRONIZE_CHANGE";
        case PREC_LOCK_NODE_EXISTS_CHANGE:
            return "PREC_LOCK_NODE_EXISTS_CHANGE";
        case QUEUE_CHILD_CHANGE:
            return "QUEUE_CHILD_CHANGE";
        default:
            return "unknown change";
    }
}

Event
CachedObjectChangeHandlers::handleNotifyableStateChange(NotifyableImpl *ntp,
                                                        int32_t etype,
                                                        const string &key)
{
    TRACE(CL_LOG, "handleNotifyableStateChange");
    LOG_DEBUG(CL_LOG,
              "handleNotifyableStateChange: %s on notifyable (%s)"
              " for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              (!ntp) ? "" : ntp->getKey().c_str(),
              key.c_str());

    unsetHandlerCallbackReady(NOTIFYABLE_STATE_CHANGE, key);
    
    if (etype == ZOO_DELETED_EVENT) {
        /*
         * If this NotifyableImpl hasn't been removed (removed by a
         * thread on another FactoryOps), the actual change in state is
         * propagated through this handler.
         */
        if ((ntp) && (key.compare(ntp->getKey()) == 0)) {
            getOps()->removeNotifyableFromCacheByKey(key);
        }

        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleNotifyableStateChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_STATECHANGE;
    }

    getOps()->establishNotifyableStateChange(ntp);
    return EN_STATECHANGE;
}

Event
CachedObjectChangeHandlers::handleApplicationsChange(NotifyableImpl *ntp,
                                                     int32_t etype,
                                                     const string &key)
{
    TRACE(CL_LOG, "handleApplicationsChange");
    
    unsetHandlerCallbackReady(APPLICATIONS_CHANGE, key);

    /*
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleApplicationsChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleApplicationsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_APPSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleApplicationsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * Convert to a root object.
     */
    RootImpl *root = dynamic_cast<RootImpl *>(ntp);
    if (root == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Root * failed for %s",
                 key.c_str());
        return EN_APPSCHANGE;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getApplicationNames();

    return EN_APPSCHANGE;
}

Event
CachedObjectChangeHandlers::handleGroupsChange(NotifyableImpl *ntp,
                                               int32_t etype,
                                               const string &key)
{
    TRACE(CL_LOG, "handleGroupsChange");
    
    unsetHandlerCallbackReady(GROUPS_CHANGE, key);

    /*
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleGroupsChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleGroupsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_GROUPSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleGroupsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * Convert to group object.
     */
    GroupImpl *group = dynamic_cast<GroupImpl *>(ntp);
    if (group == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_GROUPSCHANGE;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getGroupNames(group);

    return EN_GROUPSCHANGE;
}

Event
CachedObjectChangeHandlers::handleDataDistributionsChange(NotifyableImpl *ntp,
                                                          int32_t etype,
                                                          const string &key)
{
    TRACE(CL_LOG, "handleDataDistributionsChange");
    
    unsetHandlerCallbackReady(DATADISTRIBUTIONS_CHANGE, key);

    /*
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleDataDistributionsChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleDataDistributionsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_DISTSCHANGE;
    }

    /*
     * Convert to group object.
     */
    GroupImpl *group = dynamic_cast<GroupImpl *>(ntp);
    if (group == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_DISTSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleDataDistributionsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getDataDistributionNames(group);

    return EN_DISTSCHANGE;
}

Event
CachedObjectChangeHandlers::handleNodesChange(NotifyableImpl *ntp,
                                              int32_t etype,
                                              const string &key)
{
    TRACE(CL_LOG, "handleNodesChange");
    
    unsetHandlerCallbackReady(NODES_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleNodesChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleNodesChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_NODESCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleNodesChange: %s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*
     * Convert to a group object.
     */
    GroupImpl *group = dynamic_cast<GroupImpl *>(ntp);
    if (group == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NODESCHANGE;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getNodeNames(group);

    return EN_NODESCHANGE;
}

Event
CachedObjectChangeHandlers::handleProcessSlotsChange(NotifyableImpl *ntp,
                                                     int32_t etype,
                                                     const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotsChange");
    
    unsetHandlerCallbackReady(PROCESSSLOTS_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotsChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_PROCESSSLOTSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotsChange: %s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*
     * Convert to a node object.
     */
    NodeImpl *node = dynamic_cast<NodeImpl *>(ntp);
    if (node == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Node * failed for %s",
                 key.c_str());
        return EN_PROCESSSLOTSCHANGE;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getProcessSlotNames(node);

    return EN_PROCESSSLOTSCHANGE;
}

Event
CachedObjectChangeHandlers::handleProcessSlotsUsageChange(NotifyableImpl *ntp,
                                                          int32_t etype,
                                                          const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotsUsageChange");

    Event event = EN_PROCESSSLOTSCHANGE;
    unsetHandlerCallbackReady(PROCESSSLOTS_USAGE_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotsUsageChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotsUsageChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotsUsageChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*
     * Convert to a node object.
     */
    NodeImpl *node = dynamic_cast<NodeImpl *>(ntp);
    if (node == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Node * failed for %s",
                 key.c_str());
        return event;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    node->getUseProcessSlots();
    return event;
}

Event
CachedObjectChangeHandlers::handleProcessSlotPortVecChange(NotifyableImpl *ntp,
                                                           int32_t etype,
                                                           const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotPortVecChange");

    Event event = EN_PROCESSSLOTPORTVECCHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_PORTVEC_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotPortVecChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotPortVecChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotPortVecChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*
     * Convert to a process slot object.
     */
    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(ntp);
    if (processSlot == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    processSlot->getPortVec();

    return event;
}

Event
CachedObjectChangeHandlers::handleProcessSlotExecArgsChange(
    NotifyableImpl *ntp,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotExecArgsChange");

    Event event = EN_PROCESSSLOTEXECARGSCHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_EXECARGS_CHANGE, key);

    /*                                                                         
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotExecArgsChange: deleted");
        return EN_DELETED;
    }

    /*                                                                         
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotExecArgsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotExecArgsChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*                                                                         
     * Convert to a process slot object.
     */
    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(ntp);
    if (processSlot == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    /*   
     * Data not required, only using this function to set the watch
     * again.
     */
    vector<string> addEnv;
    string path, cmd;
    processSlot->getExecArgs(addEnv, path, cmd);

    return event;
}

Event
CachedObjectChangeHandlers::handleProcessSlotRunningExecArgsChange(
    NotifyableImpl *ntp,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotRunningExecArgsChange");

    Event event = EN_PROCESSSLOTRUNNINGEXECARGSCHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_RUNNING_EXECARGS_CHANGE, key);

    /*                                                                         
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotRunningExecArgsChange: deleted");
        return EN_DELETED;
    }

    /*                                                                         
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotRunningExecArgsChange: "
                 "Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotRunningExecArgsChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*                                                                         
     * Convert to a process slot object.
     */
    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(ntp);
    if (processSlot == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    /*   
     * Data not required, only using this function to set the watch
     * again.
     */
    vector<string> addEnv;
    string path, cmd;
    processSlot->getRunningExecArgs(addEnv, path, cmd);

    return event;
}

Event
CachedObjectChangeHandlers::handleProcessSlotPIDChange(NotifyableImpl *ntp,
                                                       int32_t etype,
                                                       const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotPIDChange");

    Event event = EN_PROCESSSLOTPIDCHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_PID_CHANGE, key);

    /*                                                                         
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotPIDChange: deleted");
        return EN_DELETED;
    }

    /*                                                                         
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotPIDChange: "
                 "Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotPIDChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*                                                                         
     * Convert to a process slot object.
     */
    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(ntp);
    if (processSlot == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    /*   
     * Data not required, only using this function to set the watch
     * again.
     */
    processSlot->getPID();

    return event;
}

Event
CachedObjectChangeHandlers::handleProcessSlotDesiredStateChange(
    NotifyableImpl *ntp,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotDesiredStateChange");

    Event event = EN_PROCESSSLOTDESIREDSTATECHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_DESIRED_STATE_CHANGE, key);

    /*                                                                         
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotDesiredStateChange: deleted");
        return EN_DELETED;
    }

    /*                                                                         
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotDesiredStateChange: "
                 "Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotDesiredStateChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*                                                                         
     * Convert to a process slot object.
     */
    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(ntp);
    if (processSlot == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    /*   
     * Data not required, only using this function to set the watch
     * again.
     */
    processSlot->getDesiredProcessState();

    return event;
}
Event
CachedObjectChangeHandlers::handleProcessSlotCurrentStateChange(
    NotifyableImpl *ntp,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotCurrentStateChange");

    Event event = EN_PROCESSSLOTCURRENTSTATECHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_CURRENT_STATE_CHANGE, key);

    /*                                                                         
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotCurrentStateChange: deleted");
        return EN_DELETED;
    }

    /*                                                                         
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotCurrentStateChange: "
                 "Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotCurrentStateChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*                                                                         
     * Convert to a process slot object.
     */
    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(ntp);
    if (processSlot == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    /*   
     * Data not required, only using this function to set the watch
     * again.
     */
    processSlot->getCurrentProcessState();

    return event;
}

Event
CachedObjectChangeHandlers::handleProcessSlotReservationChange(
    NotifyableImpl *ntp,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotReservationChange");

    Event event = EN_PROCESSSLOTRESERVATIONCHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_RESERVATION_CHANGE, key);

    /*                                                                         
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotReservationChange: deleted");
        return EN_DELETED;
    }

    /*                                                                         
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotReservationChange: "
                 "Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotReservationChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*                                                                         
     * Convert to a process slot object.
     */
    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(ntp);
    if (processSlot == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    /*   
     * Data not required, only using this function to set the watch
     * again.
     */
    processSlot->getReservationName();

    return event;
}







Event
CachedObjectChangeHandlers::handlePropertyListsChange(NotifyableImpl *ntp,
                                                      int32_t etype,
                                                      const string &key)
{
    TRACE(CL_LOG, "handlePropertyListsChange");
    
    unsetHandlerCallbackReady(PROPERTYLISTS_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handlePropertyListsChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handlePropertyListsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_PROPLISTSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handlePropertyListsChange: %s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getPropertyListNames(ntp);

    return EN_PROPLISTSCHANGE;
}

Event
CachedObjectChangeHandlers::handlePropertyListValueChange(NotifyableImpl *ntp,
                                                        int32_t etype,
                                                        const string &key)
{
    TRACE(CL_LOG, "handlePropertyListValueChange");
    
    unsetHandlerCallbackReady(PROPERTYLIST_VALUES_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handlePropertyListValueChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handlePropertyListValueChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_PROPLISTVALUESCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handlePropertyListValueChange: %s on notifyable \"%s\" "
              "for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*
     * Convert the NotifyableImpl into a PropertyList *.
     * If there's no PropertyListImpl * then punt.
     */
    PropertyListImpl *prop = dynamic_cast<PropertyListImpl *>(ntp);
    if (prop == NULL) {
        LOG_WARN(CL_LOG, 
                 "Conversion to PropertyListImpl * failed for %s",
                 key.c_str());
        return EN_PROPLISTVALUESCHANGE;
    }

    prop->updatePropertyListMap();

    return EN_PROPLISTVALUESCHANGE;
}

Event
CachedObjectChangeHandlers::handleDataDistributionShardsChange(
    NotifyableImpl *ntp,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleDataDistributionShardsChange");
    
    unsetHandlerCallbackReady(SHARDS_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleDataDistributionShardsChange: deleted");
        return EN_DELETED;
    }

    /*
     * If the given NotifyableImpl is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
        return EN_SHARDSCHANGE;
    }

    /*
     * Convert it into a DataDistribution *.
     */
    DataDistributionImpl *distp = dynamic_cast<DataDistributionImpl *>(ntp);

    /*
     * If there's no data distribution, punt.
     */
    if (distp == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to DataDistribution * failed for %s",
                 key.c_str());
        return EN_SHARDSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleShardsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());
    /*
     * If there was no change, return EN_NOEVENT.
     */
    if (!distp->update()) {
        return EN_NOEVENT;
    }

    /*
     * Propagate to client.
     */
    return EN_SHARDSCHANGE;
}

Event
CachedObjectChangeHandlers::handleClientStateChange(NotifyableImpl *ntp,
                                                    int32_t etype,
                                                    const string &key)
{
    TRACE(CL_LOG, "handleClientStateChange");
    
    unsetHandlerCallbackReady(NODE_CLIENT_STATE_CHANGE, key);

    /*                                                                       
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleClientStateChange: deleted");
        return EN_DELETED;
    }

    /*
     * If the given NotifyableImpl is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, 
                 "handleClientStateChange:"
                 " No NotifyableImpl provided -- punting");
        return EN_CLIENTSTATECHANGE;
    }

    /*
     * Convert it to a NodeImpl *.
     */
    NodeImpl *np = dynamic_cast<NodeImpl *>(ntp);

    /*
     * If there's no node, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to NodeImpl * failed for %s",
                 key.c_str());
        return EN_CLIENTSTATECHANGE;
    }

    /*
     * Get the current client state and re-establish watch.
     */
    string ns = getOps()->getNodeClientState(np->getKey());

    LOG_DEBUG(CL_LOG,
              "handleClientStateChange: %s on notifyable: (%s) with key (%s)"
              " with new client state (%s)",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str(),
              ns.c_str());

    /*
     * Update the cache and cause a user-level event if
     * the new value is different from the currently
     * cached value.
     */
    if (ns == np->getClientState()) {
        return EN_NOEVENT;
    }

    np->setClientStateAndTime(ns, TimerService::getCurrentTimeMsecs());

    return EN_CLIENTSTATECHANGE;
}

Event
CachedObjectChangeHandlers::handleMasterSetStateChange(NotifyableImpl *ntp,
                                                       int32_t etype,
                                                       const string &key)
{
    TRACE(CL_LOG, "handleMasterSetStateChange");
    
    unsetHandlerCallbackReady(NODE_MASTER_SET_STATE_CHANGE, key);

    /*
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleMasterSetStateChange: deleted");
        return EN_DELETED;
    }

    /*
     * If the given NotifyableImpl is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
        return EN_MASTERSTATECHANGE;
    }

    /*
     * Try to convert to NodeImpl *
     */
    NodeImpl *np = dynamic_cast<NodeImpl *>(ntp);

    /*
     * If there's no node, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to NodeImpl * failed for %s",
                 key.c_str());
        return EN_MASTERSTATECHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleMasterSetStateChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * Get the new value and re-establish watch.
     */
    int32_t nv = getOps()->getNodeMasterSetState(np->getKey());

    /*
     * Update the cache and cause a user-level event
     * if the new value is different than what is in
     * the cache now.
     */
    if (nv == np->getMasterSetState()) {
        return EN_NOEVENT;
    }

    np->setMasterSetStateAndTime(nv, TimerService::getCurrentTimeMsecs());

    return EN_MASTERSTATECHANGE;
}

Event
CachedObjectChangeHandlers::handleNodeConnectionChange(NotifyableImpl *ntp,
                                                       int32_t etype,
                                                       const string &key)
{
    TRACE(CL_LOG, "handleNodeConnectionChange");
    
    unsetHandlerCallbackReady(NODE_CONNECTION_CHANGE, key);

    /*
     * If there's no Notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
        return EN_CONNECTEDCHANGE;
    }

    /*
     * Convert it to a NodeImpl *
     */
    NodeImpl *np = dynamic_cast<NodeImpl *>(ntp);

    /*
     * If there's no Node, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to NodeImpl * failed for %s",
                 key.c_str());
        return EN_CONNECTEDCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleNodeConnectionChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * Get the current value and re-establish watch.
     */
    bool curconn = getOps()->isNodeConnected(np->getKey());

    /*
     * Cause a user level event and update the cache
     * if the returned value is different than the
     * currently cached value.
     */
    if (curconn == np->isConnected()) {
        return EN_NOEVENT;
    }

    np->setConnectedAndTime(curconn, TimerService::getCurrentTimeMsecs());

    return EN_CONNECTEDCHANGE;
}

/*
 * Special handler for synchronization that waits until the zk event
 * is propogated through the m_eventAdapter before returning.  This
 * ensures that all zk events prior to this point are visible by each
 * client.
 */
Event
CachedObjectChangeHandlers::handleSynchronizeChange(NotifyableImpl *ntp,
                                                    int32_t etype,
                                                    const string &key)
{
    TRACE(CL_LOG, "handleSynchronizeChange");

    /* No need to reset the callback i.e. unsetHandlerCallbackReady(). */

    /*
     * Notify the thread waiting to acquire the lock that this
     * lock node has finally been deleted.  Since the
     * PredMutexCond cannot be deleted, the thread calling this
     * function should be safe.
     */
    getOps()->getSyncEventSignalMap()->signalPredMutexCond(key);

    LOG_DEBUG(CL_LOG,
              "handleSynchronizeChange: sent conditional signal for key %s",
              key.c_str());

    return EN_NOEVENT;
}

/*
 * Handle existence change for preceding lock node. This is called
 * whenever a preceding lock node being watched by a thread in this
 * abdicates.  All it does is signal the lock waiting on it to wake up
 * and try again.
 */
Event
CachedObjectChangeHandlers::handlePrecLockNodeExistsChange(NotifyableImpl *ntp,
                                                           int32_t etype,
                                                           const string &key)
{
    TRACE(CL_LOG, "handlePrecLockNodeExistsChange");

    LOG_DEBUG(CL_LOG,
              "handlePrecLockNodeExistsChange: %s on key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              key.c_str());

    /*
     * This is the only expected event.
     */
    if (etype == ZOO_DELETED_EVENT) {
        /*
         * Notify the thread waiting to acquire the lock that this
         * lock node has finally been deleted.  Since the
         * PredMutexCond cannot be deleted, the thread calling this
         * function should be safe.
         */
        getOps()->getLockEventSignalMap()->signalPredMutexCond(key);

        return EN_LOCKNODECHANGE;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "handlePrecLockNodeExistsChange: non-ZOO_DELETED_EVENT "
                  "event %d called", etype);
        throw InconsistentInternalStateException(
            "handlePrecLockNodeExistsChange: "
            "non-ZOO_DELETED_EVENT called");
    }
    
    return EN_NOEVENT;
}

Event
CachedObjectChangeHandlers::handleQueueChildChange(NotifyableImpl *ntp,
                                                   int32_t etype,
                                                   const string &key)
{
    TRACE(CL_LOG, "handleQueueChildChange");

    LOG_DEBUG(CL_LOG,
              "handleQueueChildChange: %s on key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              key.c_str());

    /*
     * Try to check the queue again if a child event.
     */
    if (etype == ZOO_CHILD_EVENT) {
        /*
         * Notify the thread waiting to get the first element since
         * the number of children has changed.  Since the
         * PredMutexCond cannot be deleted, the thread calling this
         * function should be safe.
         */
        LOG_DEBUG(CL_LOG, 
                  "handleQueueChildChange: Got child event for key %s",
                  key.c_str());
        getOps()->getQueueEventSignalMap()->signalPredMutexCond(key);
        return EN_QUEUECHILDCHANGE;
    }
    else if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleQueueChildChange: deleted");
        return EN_DELETED;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "handleQueueChildChange: non-ZOO_CHILD_EVENT "
                  "event %s called", 
                  zk::ZooKeeperAdapter::getEventString(etype).c_str());
        throw InconsistentInternalStateException(
            "handleQueueChildChange: "
            "non-ZOO_CHILD_EVENT called");
    }
    
    return EN_NOEVENT;
}

bool
CachedObjectChangeHandlers::isHandlerCallbackReady(CachedObjectChange change,
                                                   const string &key)
{
    TRACE(CL_LOG, "isHandlerCallbackReady");

    Locker l1(getLock());

    map<CachedObjectChange, map<string, bool> >::const_iterator changeIt = 
        m_handlerKeyCallbackCount.find(change);
    if (changeIt != m_handlerKeyCallbackCount.end()) {
        map<string, bool>::const_iterator keyIt = changeIt->second.find(key);
        if (keyIt != changeIt->second.end()) {
            return keyIt->second;
        }
    }

    return false;
}

void
CachedObjectChangeHandlers::setHandlerCallbackReady(
    CachedObjectChange change,
    const string &key)
{
    TRACE(CL_LOG, "setHandlerCallbackReady");

    Locker l1(getLock());

    map<CachedObjectChange, map<string, bool> >::iterator changeIt = 
        m_handlerKeyCallbackCount.find(change);
    if (changeIt != m_handlerKeyCallbackCount.end()) {
        map<string, bool>::iterator keyIt = changeIt->second.find(key);
        if (keyIt != changeIt->second.end()) {
            if (keyIt->second == true) {
                LOG_FATAL(CL_LOG,
                          "setHandlerCallbackReady: "
                          "change %d, key %s, True!",
                          change,
                          key.c_str());
                throw InconsistentInternalStateException(
                    "setHandlerCallbackReady: True.");
            }
        }
    }

    LOG_DEBUG(CL_LOG,
              "setHandlerCallbackReady: "
              "change %s, key %s",
              getCachedObjectChangeString(change).c_str(),
              key.c_str());

    m_handlerKeyCallbackCount[change][key] = true;
}

void
CachedObjectChangeHandlers::unsetHandlerCallbackReady(
    CachedObjectChange change,
    const string &key)
{
    TRACE(CL_LOG, "unsetHandlerCallbackReady");

    Locker l1(getLock());
    map<CachedObjectChange, map<string, bool> >::iterator changeIt = 
        m_handlerKeyCallbackCount.find(change);
    if (changeIt == m_handlerKeyCallbackCount.end()) {
        LOG_FATAL(CL_LOG,
                  "unsetHandlerCallbackReady: change %s, key %s, "
                  "change not defined.",
                  getCachedObjectChangeString(change).c_str(),
                  key.c_str());
        throw InconsistentInternalStateException("unsetHandlerCallbackReady: "
                                                 "Change not initialized.");
    }
    map<string, bool>::iterator keyIt = 
        changeIt->second.find(key);
    if (keyIt == changeIt->second.end()) {
        LOG_FATAL(CL_LOG,
                  "unsetHandlerCallbackReady: change %s, key %s, "
                  "key not defined.",
                  getCachedObjectChangeString(change).c_str(),
                  key.c_str());
        throw InconsistentInternalStateException("unsetHandlerCallbackReady: "
                                                 "Key not initialized.");
    }

    if (keyIt->second != true) {
        LOG_FATAL(CL_LOG,
                  "unsetHandlerCallbackReady: change %s, key %s, Not true!",
                  getCachedObjectChangeString(change).c_str(),
                  key.c_str());
        throw InconsistentInternalStateException("unsetHandlerCallbackReady: "
                                                 "Not true.");
    }

    LOG_DEBUG(CL_LOG,
              "unsetHandlerCallbackReady: "
              "change %s, key %s",
              getCachedObjectChangeString(change).c_str(),
              key.c_str());
    
    keyIt->second = false;
}

CachedObjectEventHandler *
CachedObjectChangeHandlers::getChangeHandler(CachedObjectChange change) {
    switch (change) {
        case NOTIFYABLE_STATE_CHANGE:
            return &m_notifyableStateChangeHandler;
        case APPLICATIONS_CHANGE:
            return &m_applicationsChangeHandler;
        case GROUPS_CHANGE:
            return &m_groupsChangeHandler;
        case DATADISTRIBUTIONS_CHANGE:
            return &m_dataDistributionsChangeHandler;
        case NODES_CHANGE:
            return &m_nodesChangeHandler;
        case PROCESSSLOTS_CHANGE:
            return &m_processSlotsChangeHandler;
        case PROCESSSLOTS_USAGE_CHANGE:
            return &m_processSlotsUsageChangeHandler;
        case PROCESSSLOT_PORTVEC_CHANGE:
            return &m_processSlotPortVecChangeHandler;
        case PROCESSSLOT_EXECARGS_CHANGE:
            return &m_processSlotExecArgsChangeHandler;
        case PROCESSSLOT_RUNNING_EXECARGS_CHANGE:
            return &m_processSlotRunningExecArgsChangeHandler;
        case PROCESSSLOT_PID_CHANGE:
            return &m_processSlotPIDChangeHandler;
        case PROCESSSLOT_DESIRED_STATE_CHANGE:
            return &m_processSlotDesiredStateChangeHandler;
        case PROCESSSLOT_CURRENT_STATE_CHANGE:
            return &m_processSlotCurrentStateChangeHandler;
        case PROCESSSLOT_RESERVATION_CHANGE:
            return &m_processSlotReservationChangeHandler;
        case PROPERTYLISTS_CHANGE:
            return &m_propertyListsChangeHandler;
        case PROPERTYLIST_VALUES_CHANGE:
            return &m_propertyListValueChangeHandler;
        case SHARDS_CHANGE:
            return &m_dataDistributionShardsChangeHandler;
        case NODE_CLIENT_STATE_CHANGE:
            return &m_nodeClientStateChangeHandler;
        case NODE_MASTER_SET_STATE_CHANGE:
            return &m_nodeMasterSetStateChangeHandler;
        case NODE_CONNECTION_CHANGE:
            return &m_nodeConnectionChangeHandler;
        case SYNCHRONIZE_CHANGE:
            return &m_synchronizeChangeHandler;
        case PREC_LOCK_NODE_EXISTS_CHANGE:
            return &m_precLockNodeExistsChangeHandler;
        case QUEUE_CHILD_CHANGE:
            return &m_queueChildChangeHandler;
        default:
            LOG_FATAL(CL_LOG, 
                      "getChangeHandler: Change %d is not defined\n",
                      change);
            throw InvalidArgumentsException("getChangeHandler: "
                                            "change is not defined");
    }
}

};	/* End of 'namespace clusterlib' */
