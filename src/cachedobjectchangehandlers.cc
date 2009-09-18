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
        case PROPERTIES_CHANGE:
            return "PROPERTIES_CHANGE";
        case PROPERTIES_VALUES_CHANGE:
            return "PROPERTIES_VALUES_CHANGE";
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
        default:
            return "unknown change";
    }
}

/*
 * Handle changes to the state of a Notifyable.
 */
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

/*
 * Handle a change in the set of applications
 */
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

/*
 * Handle a change in the set of groups for an
 * application.
 */
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

/*
 * Handle a change in the set of distributions
 * for an application.
 */
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

/*
 * Handle a change in the set of nodes in a group.
 */
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

/*
 * Handle a change in the set of properties in a notifyable.
 */
Event
CachedObjectChangeHandlers::handlePropertiesChange(NotifyableImpl *ntp,
                                                   int32_t etype,
                                                   const string &key)
{
    TRACE(CL_LOG, "handlePropertiesChange");

    unsetHandlerCallbackReady(PROPERTIES_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handlePropertiesChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handlePropertiesChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_PROPSCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handlePropertiesChange: %s on notifyable \"%s\" for key %s",
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
        return EN_PROPSCHANGE;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getPropertiesNames(ntp);

    return EN_PROPSCHANGE;
}

/*
 * Handle a change in the value of a property list.
 */
Event
CachedObjectChangeHandlers::handlePropertiesValueChange(NotifyableImpl *ntp,
                                                        int32_t etype,
                                                        const string &key)
{
    TRACE(CL_LOG, "handlePropertiesValueChange");

    unsetHandlerCallbackReady(PROPERTIES_VALUES_CHANGE, key);

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handlePropertiesValueChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handlePropertiesValueChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_PROPSVALCHANGE;
    }

    LOG_DEBUG(CL_LOG,
              "handlePropertiesValueChange: %s on notifyable \"%s\" "
              "for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str(),
              key.c_str());

    /*
     * Convert the NotifyableImpl into a Properties *.
     * If there's no PropertiesImpl * then punt.
     */
    PropertiesImpl *prop = dynamic_cast<PropertiesImpl *>(ntp);
    if (prop == NULL) {
        LOG_WARN(CL_LOG, 
                 "Conversion to PropertiesImpl * failed for %s",
                 key.c_str());
        return EN_PROPSVALCHANGE;
    }

    prop->updatePropertiesMap();

    return EN_PROPSVALCHANGE;
}

/*
 * Handle change in the shards of a data distribution.
 */
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

/*
 * Handle change in client-reported state for a node.
 */
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

    np->setClientStateAndTime(ns, TimerService::getCurrentTimeMillis());

    return EN_CLIENTSTATECHANGE;
}

/*
 * Handle change in master-set desired state for
 * a node.
 */
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

    np->setMasterSetStateAndTime(nv, TimerService::getCurrentTimeMillis());

    return EN_MASTERSTATECHANGE;
}

/*
 * Handle change in the connection state of a node.
 */
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

    np->setConnectedAndTime(curconn, TimerService::getCurrentTimeMillis());

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
        getOps()->getDistributedLocks()->getSignalMap()->
            signalPredMutexCond(key);

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
        case PROPERTIES_CHANGE:
            return &m_propertiesChangeHandler;
        case PROPERTIES_VALUES_CHANGE:
            return &m_propertiesValueChangeHandler;
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
        default:
            LOG_FATAL(CL_LOG, 
                      "getChangeHandler: Change %d is not defined\n",
                      change);
            throw InvalidArgumentsException("getChangeHandler: "
                                            "change is not defined");
    }
}

};	/* End of 'namespace clusterlib' */
