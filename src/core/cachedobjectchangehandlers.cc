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

using namespace std;
using namespace boost;

namespace clusterlib {

string 
CachedObjectChangeHandlers::getCachedObjectChangeString(
    CachedObjectChange change)
{
    switch (change) {
        case NOTIFYABLE_REMOVED_CHANGE:
            return "NOTIFYABLE_REMOVED_CHANGE";
        case CURRENT_STATE_CHANGE:
            return "CURRENT_STATE_CHANGE";
        case DESIRED_STATE_CHANGE:
            return "DESIRED_STATE_CHANGE";
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
        case PROCESSSLOT_PROCESSINFO_CHANGE:
            return "PROCESSSLOT_PROCESSINFO_CHANGE";
        case PROPERTYLISTS_CHANGE:
            return "PROPERTYLISTS_CHANGE";
        case PROPERTYLIST_VALUES_CHANGE:
            return "PROPERTYLIST_VALUES_CHANGE";
        case SHARDS_CHANGE:
            return "SHARDS_CHANGE";
        case NODE_PROCESS_SLOT_INFO_CHANGE:
            return "NODE_PROCESS_SLOT_INFO_CHANGE";
        case SYNCHRONIZE_CHANGE:
            return "SYNCHRONIZE_CHANGE";
        case PREC_LOCK_NODE_EXISTS_CHANGE:
            return "PREC_LOCK_NODE_EXISTS_CHANGE";
        case QUEUES_CHANGE:
            return "QUEUES_CHANGE";
        case QUEUE_CHILD_CHANGE:
            return "QUEUE_CHILD_CHANGE";
        default:
            return "unknown change";
    }
}

Event
CachedObjectChangeHandlers::handleNotifyableRemovedChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleNotifyableRemovedChange");
    LOG_DEBUG(CL_LOG,
              "handleNotifyableRemovedChange: %s on notifyable (%s)"
              " for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              (!notifyableSP) ? "" : notifyableSP->getKey().c_str(),
              key.c_str());

    unsetHandlerCallbackReady(NOTIFYABLE_REMOVED_CHANGE, key);
    
    if (etype == ZOO_DELETED_EVENT) {
        /*
         * If this NotifyableImpl hasn't been removed (removed by a
         * thread on another FactoryOps), the actual change in state
         * is propagated through this handler.  Note that if any part
         * of the NotifyableImpl is removed, it gets removed.
         */
        if (notifyableSP != NULL) {
            getOps()->removeCachedNotifyable(
                shared_ptr<NotifyableImpl>(notifyableSP));
        }

        return EN_DELETED;
    }

    /* Re-establish the watch since it wasn't deleted */
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->nodeExists(
            key,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::NOTIFYABLE_REMOVED_CHANGE)),
        ,
        CachedObjectChangeHandlers::NOTIFYABLE_REMOVED_CHANGE,
        key,
        "Establishing the watch on value %s failed: %s",
        key.c_str(),
        false,
        true);

    return EN_NOEVENT;
}

Event
CachedObjectChangeHandlers::handleCurrentStateChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleCurrentStateChange");
    LOG_DEBUG(CL_LOG,
              "handleCurrentStateChange: %s on notifyable (%s)"
              " for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              (!notifyableSP) ? "" : notifyableSP->getKey().c_str(),
              key.c_str());

    unsetHandlerCallbackReady(CURRENT_STATE_CHANGE, key);
    
    /*
     * If there's no notifyable or is deleted, punt.
     */
    if ((notifyableSP == NULL) || (etype == ZOO_DELETED_EVENT)) {
        LOG_WARN(CL_LOG,
                 "handleCurrentStateChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_CURRENT_STATE_CHANGE;
    }

    /*
     * Update the cached data and re-establish watches.
     */
    dynamic_cast<CachedStateImpl &>(
        notifyableSP->cachedCurrentState()).loadDataFromRepository(false);
    return EN_CURRENT_STATE_CHANGE;
}

Event
CachedObjectChangeHandlers::handleDesiredStateChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleDesiredStateChange");
    LOG_DEBUG(CL_LOG,
              "handleDesiredStateChange: %s on notifyable (%s)"
              " for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              (!notifyableSP) ? "" : notifyableSP->getKey().c_str(),
              key.c_str());

    unsetHandlerCallbackReady(DESIRED_STATE_CHANGE, key);
    
    /*
     * If there's no notifyable or is deleted, punt.
     */
    if ((notifyableSP == NULL) || (etype == ZOO_DELETED_EVENT)) {
        LOG_WARN(CL_LOG,
                 "handleDesiredStateChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_DESIRED_STATE_CHANGE;
    }

    /*
     * Update the cached data and re-establish watches.
     */
    dynamic_cast<CachedStateImpl &>(
        notifyableSP->cachedDesiredState()).loadDataFromRepository(false);
    return EN_DESIRED_STATE_CHANGE;
}

Event
CachedObjectChangeHandlers::handleApplicationsChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handleApplicationsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_APPSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleApplicationsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str());

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getChildrenNames(
        NotifyableKeyManipulator::createApplicationChildrenKey(notifyableSP->getKey()),
        CachedObjectChangeHandlers::APPLICATIONS_CHANGE);
    return EN_APPSCHANGE;
}

Event
CachedObjectChangeHandlers::handleGroupsChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handleGroupsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_GROUPSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleGroupsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str());

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getChildrenNames(
        NotifyableKeyManipulator::createGroupChildrenKey(notifyableSP->getKey()),
        CachedObjectChangeHandlers::GROUPS_CHANGE);
    return EN_GROUPSCHANGE;
}

Event
CachedObjectChangeHandlers::handleDataDistributionsChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handleDataDistributionsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_DISTSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleDataDistributionsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str());

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getChildrenNames(
        NotifyableKeyManipulator::createDataDistributionChildrenKey(
            notifyableSP->getKey()),
        CachedObjectChangeHandlers::DATADISTRIBUTIONS_CHANGE);
    return EN_DISTSCHANGE;
}

Event
CachedObjectChangeHandlers::handleNodesChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handleNodesChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_NODESCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleNodesChange: %s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str(),
              key.c_str());

    /*
     * Convert to a group object.
     */
    shared_ptr<GroupImpl> groupSP = 
        dynamic_pointer_cast<GroupImpl>(notifyableSP);
    if (groupSP == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NODESCHANGE;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getChildrenNames(
        NotifyableKeyManipulator::createNodeChildrenKey(
            notifyableSP->getKey()),
        CachedObjectChangeHandlers::NODES_CHANGE);
    return EN_NODESCHANGE;
}

Event
CachedObjectChangeHandlers::handleProcessSlotsChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_PROCESSSLOTSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotsChange: %s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str(),
              key.c_str());
    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getChildrenNames(
        NotifyableKeyManipulator::createProcessSlotChildrenKey(notifyableSP->getKey()),
        CachedObjectChangeHandlers::PROCESSSLOTS_CHANGE);
    return EN_PROCESSSLOTSCHANGE;
}

Event
CachedObjectChangeHandlers::handleProcessSlotsUsageChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotsUsageChange");

    Event event = EN_PROCESSSLOTSUSAGECHANGE;
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
    if (notifyableSP == NULL) {
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
              notifyableSP->getKey().c_str(),
              key.c_str());

    /*
     * Convert to a node object.
     */
    shared_ptr<NodeImpl> nodeSP = dynamic_pointer_cast<NodeImpl>(notifyableSP);
    if (nodeSP == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Node * failed for %s",
                 key.c_str());
        return event;
    }

    dynamic_cast<CachedProcessSlotInfoImpl &>(
        nodeSP->cachedProcessSlotInfo()).loadDataFromRepository(false);
    
    return event;
}

Event
CachedObjectChangeHandlers::handleProcessSlotProcessInfoChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleProcessSlotProcessInfoChange");

    Event event = EN_PROCESSSLOTPROCESSINFOCHANGE;
    unsetHandlerCallbackReady(PROCESSSLOT_PROCESSINFO_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleProcessSlotProcessInfoChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handleProcessSlotProcessInfoChange: "
                 "Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return event;
    }

    LOG_DEBUG(CL_LOG,
              "handleProcessSlotProcessInfoChange: "
              "%s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str(),
              key.c_str());

    /*
     * Convert to a process slot object.
     */
    shared_ptr<ProcessSlotImpl> processSlotSP = 
        dynamic_pointer_cast<ProcessSlotImpl>(notifyableSP);
    if (processSlotSP == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to ProcessSlotImpl * failed for %s",
                 key.c_str());
        return event;
    }

    dynamic_cast<CachedProcessInfoImpl &>(
        processSlotSP->cachedProcessInfo()).loadDataFromRepository(false);

    return event;
}

Event
CachedObjectChangeHandlers::handlePropertyListsChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handlePropertyListsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_PROPLISTSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handlePropertyListsChange: %s on notifyable \"%s\" for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str(),
              key.c_str());

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getChildrenNames(
        NotifyableKeyManipulator::createPropertyListChildrenKey(
            notifyableSP->getKey()),
        CachedObjectChangeHandlers::PROPERTYLISTS_CHANGE);
    return EN_PROPLISTSCHANGE;
}

Event
CachedObjectChangeHandlers::handlePropertyListValueChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
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
              notifyableSP->getKey().c_str(),
              key.c_str());

    /*
     * Convert the NotifyableImpl into a PropertyList *.
     * If there's no PropertyListImpl * then punt.
     */
    shared_ptr<PropertyListImpl> propertyListSP = 
        dynamic_pointer_cast<PropertyListImpl>(notifyableSP);
    if (propertyListSP == NULL) {
        LOG_WARN(CL_LOG, 
                 "Conversion to PropertyListImpl * failed for %s",
                 key.c_str());
        return EN_PROPLISTVALUESCHANGE;
    }

    dynamic_cast<CachedKeyValuesImpl &>(
        propertyListSP->cachedKeyValues()).loadDataFromRepository(false);

    return EN_PROPLISTVALUESCHANGE;
}

Event
CachedObjectChangeHandlers::handleDataDistributionShardsChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
        return EN_SHARDSCHANGE;
    }

    /*
     * Convert it into a DataDistribution *.
     */
    shared_ptr<DataDistributionImpl> dataDistributionSP = 
        dynamic_pointer_cast<DataDistributionImpl>(notifyableSP);

    /*
     * If there's no data distribution, punt.
     */
    if (dataDistributionSP == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to DataDistribution * failed for %s",
                 key.c_str());
        return EN_SHARDSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleShardsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str());

    /*
     * Update the cached data and re-establish watches.
     */
    dynamic_cast<CachedShardsImpl &>(
        dataDistributionSP->cachedShards()).loadDataFromRepository(false);
    return EN_SHARDSCHANGE;
}

Event
CachedObjectChangeHandlers::handleNodeProcessSlotInfoChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleNodeProcessSlotInfoChange");
    
    unsetHandlerCallbackReady(NODE_PROCESS_SLOT_INFO_CHANGE, key);

    /*
     * Convert it to a NodeImpl *
     */
    shared_ptr<NodeImpl> nodeSP = 
        dynamic_pointer_cast<NodeImpl>(notifyableSP);

    /*
     * If there's no Node, punt.
     */
    if (nodeSP == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to NodeImpl * failed for %s",
                 key.c_str());
        return EN_PROCESS_SLOT_INFO_CHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleNodeProcessSlotInfoChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str());

    /*
     * Update the cached data and re-establish watches.
     */
    dynamic_cast<CachedProcessSlotInfoImpl &>(
        nodeSP->cachedProcessSlotInfo()).loadDataFromRepository(false);

    return EN_PROCESS_SLOT_INFO_CHANGE;
}

/*
 * Special handler for synchronization that waits until the zk event
 * is propogated through the m_eventAdapter before returning.  This
 * ensures that all zk events prior to this point are visible by each
 * client.
 */
Event
CachedObjectChangeHandlers::handleSynchronizeChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
CachedObjectChangeHandlers::handlePrecLockNodeExistsChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
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
CachedObjectChangeHandlers::handleQueuesChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleQueuesChange");
    
    unsetHandlerCallbackReady(QUEUES_CHANGE, key);

    /*
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleQueuesChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG,
                 "handleQueuesChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_APPSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handleQueuesChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              notifyableSP->getKey().c_str());

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getChildrenNames(
        NotifyableKeyManipulator::createQueueChildrenKey(notifyableSP->getKey()),
        CachedObjectChangeHandlers::QUEUES_CHANGE);
    return EN_QUEUESCHANGE;
}

Event
CachedObjectChangeHandlers::handleQueueChildChange(
    const shared_ptr<NotifyableImpl> &notifyableSP,
    int32_t etype,
    const string &key)
{
    TRACE(CL_LOG, "handleQueueChildChange");

    unsetHandlerCallbackReady(QUEUE_CHILD_CHANGE, key);

    /*
     * If there's no Notifyable, punt.
     */
    if (notifyableSP == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
        return EN_QUEUECHILDCHANGE;
    }

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
        shared_ptr<QueueImpl> queueSP = 
            dynamic_pointer_cast<QueueImpl>(notifyableSP);
        /*
         * Reestablish the watch if the queue still exists
         */
        if (queueSP != NULL) {
            queueSP->establishQueueWatch();
        }
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
                ostringstream oss;
                oss << "setHandlerCallbackReady: change=" 
                    << getCachedObjectChangeString(change)
                    << " with key=" << key;
                LOG_FATAL(CL_LOG, "%s", oss.str().c_str());
                throw InconsistentInternalStateException(oss.str());
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

    ostringstream oss;
    Locker l1(getLock());
    map<CachedObjectChange, map<string, bool> >::iterator changeIt = 
        m_handlerKeyCallbackCount.find(change);
    if (changeIt == m_handlerKeyCallbackCount.end()) {
        oss << "unsetHandlerCallbackReady: Key " << key << " with change "
            << getCachedObjectChangeString(change) << " is not found";
        LOG_FATAL(CL_LOG, "%s", oss.str().c_str());
        throw InconsistentInternalStateException(oss.str());
    }

    map<string, bool>::iterator keyIt = 
        changeIt->second.find(key);
    if (keyIt == changeIt->second.end()) {
        oss << "unsetHandlerCallbackReady: Key " << key << "not initialized"
            << " for event " << getCachedObjectChangeString(change);
        LOG_FATAL(CL_LOG, "%s", oss.str().c_str());
        throw InconsistentInternalStateException(oss.str());
    }

    if (keyIt->second != true) {
        oss << "unsetHandlerCallbackReady: Key " << key 
            << " didn't have the handler callback set for event " 
            << getCachedObjectChangeString(change);
        LOG_FATAL(CL_LOG, "%s", oss.str().c_str());
        throw InconsistentInternalStateException(oss.str());
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
        case NOTIFYABLE_REMOVED_CHANGE:
            return &m_notifyableRemovedChangeHandler;
        case CURRENT_STATE_CHANGE:
            return &m_currentStateChangeHandler;
        case DESIRED_STATE_CHANGE:
            return &m_desiredStateChangeHandler;
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
        case PROCESSSLOT_PROCESSINFO_CHANGE:
            return &m_processSlotProcessInfoChangeHandler;
        case PROPERTYLISTS_CHANGE:
            return &m_propertyListsChangeHandler;
        case PROPERTYLIST_VALUES_CHANGE:
            return &m_propertyListValueChangeHandler;
        case SHARDS_CHANGE:
            return &m_dataDistributionShardsChangeHandler;
        case NODE_PROCESS_SLOT_INFO_CHANGE:
            return &m_nodeProcessSlotInfoChangeHandler;
        case SYNCHRONIZE_CHANGE:
            return &m_synchronizeChangeHandler;
        case PREC_LOCK_NODE_EXISTS_CHANGE:
            return &m_precLockNodeExistsChangeHandler;
        case QUEUES_CHANGE:
            return &m_queuesChangeHandler;
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

}	/* End of 'namespace clusterlib' */
