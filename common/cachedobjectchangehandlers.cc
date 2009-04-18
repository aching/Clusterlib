/*
 * cachedobjectchangehandlers.cc --
 *
 * Implementation of the cache change handlers.
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
              "handleNotifyableStateChange: %s on notifyable \"%s\""
              " for key %s",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              (!ntp) ? "" : ntp->getKey().c_str(),
              key.c_str());
    
    if (etype == ZOO_DELETED_EVENT) {
        /*
         * If this NotifyableImpl hasn't been removed (removed by a
         * thread on another FactoryOps), the actual change in state is
         * propagated through this handler.
         */
        if ((ntp) && (key.compare(ntp->getKey()) == 0)) {
            getOps()->removeNotifyableFromCacheByKey(key);
        }

        return EN_NOEVENT;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleNotifyableStateChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_NOEVENT;
    }

    getOps()->establishNotifyableStateChange(ntp);
    return EN_READY;
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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
    }

    /*
     * Convert to group object.
     */
    GroupImpl *group = dynamic_cast<GroupImpl *>(ntp);
    if (group == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
    }

    /*
     * Data not required, only using this function to set the watch again.
     */
    getOps()->getNodeNames(group);

    return EN_MEMBERSHIPCHANGE;
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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
    }

    prop->updatePropertiesMap();

    return EN_PROPCHANGE;
}

/*
 * Handle change in shards of a data distribution.
 */
Event
CachedObjectChangeHandlers::handleShardsChange(NotifyableImpl *ntp,
                                               int32_t etype,
                                               const string &key)
{
    TRACE(CL_LOG, "handleShardsChange");

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleShardsChange: deleted");
        return EN_DELETED;
    }

    /*
     * If the given NotifyableImpl is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
        return EN_NOEVENT;
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
        return EN_NOEVENT;
    }

    LOG_DEBUG(CL_LOG,
              "handleShardsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());
    /*
     * If there was no change, return EN_NOEVENT.
     */
    if (!distp->updateShards()) {
        return EN_NOEVENT;
    }

    /*
     * Propagate to client.
     */
    return EN_SHARDSCHANGE;
}

/*
 * Handle change in manual overrides of a data distribution.
 */
Event
CachedObjectChangeHandlers::handleManualOverridesChange(NotifyableImpl *ntp,
                                                        int32_t etype,
                                                        const string &key)
{
    TRACE(CL_LOG, "handleManualOverridesChange");

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleManualOverridesChange: deleted");
        return EN_DELETED;
    }

    /*
     * If the given NotifyableImpl is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, 
                 "handleManualOverridesChange: "
                 "No NotifyableImpl provided -- punting");
        return EN_NOEVENT;
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
                 "Conversion to DataDistributionImpl * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    LOG_DEBUG(CL_LOG,
              "handleManualOverridesChange: %s on notifyable with key: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * If there was no change, return EN_NOEVENT.
     */
    if (!distp->updateManualOverrides()) {
        return EN_NOEVENT;
    }

    /*
     * Propagate to client.
     */
    return EN_MANUALOVERRIDESCHANGE;
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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
    }

    LOG_DEBUG(CL_LOG,
              "handleClientStateChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * Get the current client state and re-establish watch.
     */
    string ns = getOps()->getNodeClientState(np->getKey());

    /*
     * Update the cache and cause a user-level event if
     * the new value is different from the currently
     * cached value.
     */
    if (ns == np->getClientState()) {
        return EN_NOEVENT;
    }
    np->setClientState(ns);
    np->setClientStateTime(FactoryOps::getCurrentTimeMillis());

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
        return EN_NOEVENT;
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
        return EN_NOEVENT;
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
    np->setMasterSetState(nv);
    np->setMasterSetStateTime(FactoryOps::getCurrentTimeMillis());

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

    /*
     * If there's no Notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
        return EN_NOEVENT;
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
        return EN_NOEVENT;
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

    np->setConnected(curconn);
    np->setConnectionTime(FactoryOps::getCurrentTimeMillis());
    return (curconn == true) ? EN_CONNECTED : EN_DISCONNECTED;
}

/*
 * Handle change in the leadership of a group.
 */
Event
CachedObjectChangeHandlers::handleLeadershipChange(NotifyableImpl *ntp,
                                                   int32_t etype,
                                                   const string &key)
{
    TRACE(CL_LOG, "handleLeadershipChange");

    /*                                                                        
     * If NotifyableImpl was deleted, do not re-establish watch, pass
     * event to clients.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_DEBUG(CL_LOG, "handleLeadershipChange: deleted");
        return EN_DELETED;
    }

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handleLeadershipChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_NOEVENT;
    }

    GroupImpl *group = dynamic_cast<GroupImpl *>(ntp);
    if (group == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    LOG_DEBUG(CL_LOG,
              "handleLeadershipChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    group->setLeadershipChangeTime(FactoryOps::getCurrentTimeMillis());

    bool exists = 
        getOps()->establishNotifyableInterest(key,
                                              &m_leadershipChangeHandler);
    if (!exists) {
        group->updateLeader(NULL);
    }
    else {
        string lname = 
            getOps()->getNotifyableValue(key,
                                         &m_leadershipChangeHandler);
        if (lname == "") {
            group->updateLeader(NULL);
        }
        else {
            group->updateLeader(getOps()->getNode(lname, group, true, false));
        }
    }

    return EN_LEADERSHIPCHANGE;
}

/*
 * Handle existence change for preceding leader of
 * a group. This is called whenever a preceding leader
 * being watched by a server in this process abdicates.
 */
Event
CachedObjectChangeHandlers::handlePrecLeaderExistsChange(NotifyableImpl *ntp,
                                                         int32_t etype,
                                                         const string &key)
{
    TRACE(CL_LOG, "handlePrecLeaderExistsChange");

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "handlePrecLeaderExistsChange: Punting on event: %s on %s",
                 zk::ZooKeeperAdapter::getEventString(etype).c_str(),
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * Try to convert the passed NotifyableImpl * to
     * a Group *
     */
    Group *group = dynamic_cast<Group *>(ntp);
    if (group == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    LOG_DEBUG(CL_LOG,
              "handlePrecLeaderExistsChange: %s on notifyable: \"%s\"",
              zk::ZooKeeperAdapter::getEventString(etype).c_str(),
              ntp->getKey().c_str());

    /*
     * Make all interested Servers participate in the
     * election again.
     */
    ServerImpl *sp;
    Group *group1;
    NodeImpl *np1;
    LeadershipElectionMultimap copy;
    LeadershipIterator leIt;
    LeadershipElectionMultimapRange range;

    {
        Locker l1(getOps()->getLeadershipWatchesLock());

        /*
         * Make our own copy of the watches map.
         */
        copy = getOps()->getLeadershipWatches();

        /*
         * And remove the key from the watches map.
         */
        range = getOps()->getLeadershipWatches().equal_range(key);
        getOps()->getLeadershipWatches().erase(range.first, range.second);
    }

    /*
     * Find the range containing the watches for the
     * node we got notified about (there may be several
     * because there can be several Server instances in
     * this process).
     */
    range = copy.equal_range(key);

    for (leIt = range.first; 
         leIt != range.second;
         leIt++) {
        /*
         * Sanity check -- the key must be the same.
         */
        if ((*leIt).first != key) {
            LOG_FATAL(CL_LOG,
                      "Internal error: bad leadership watch (bid) %s vs %s",
                      key.c_str(), (*leIt).first.c_str());
            ::abort();
        }

        sp = (*leIt).second;

        /*
         * Sanity check -- the Server must not be NULL.
         */
        if (sp == NULL) {
            LOG_FATAL(
		CL_LOG,
                "Internal error: leadership watch (bid) with NULL server");
            ::abort();
        }

        /*
         * Sanity check -- the Server must be in this group.
         */

        np1 = dynamic_cast<NodeImpl *>(sp->getMyNode());
        if (np1 == NULL) {
            LOG_FATAL(CL_LOG,
                      "Internal error: NULL node for server 0x%x",
                      (uint32_t) sp);
            ::abort();
        }

        group1 = np1->getMyGroup();
        if (group1 == NULL) {
            throw InconsistentInternalStateException(
                string("handlePrecLeaderChange: ") +
                "NULL group containing " +
                np1->getKey());
        }

        if (group != group1) {
            LOG_FATAL(CL_LOG,
                      "Internal error: bad leadership watch (group) %s vs %s",
                      group->getKey().c_str(), group1->getKey().c_str());
            ::abort();
        }
        
        /*
         * Try to make this Server the leader. Ignore the result, we
         * must give everyone a chance.
         */
        (void) sp->tryToBecomeLeader();
    }

    return EN_LEADERSHIPCHANGE;
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

    {
        Locker l1(getOps()->getSyncLock());
        getOps()->incrSyncIdCompleted();
        getOps()->getSyncCond()->signal();
    }

    LOG_DEBUG(CL_LOG,
              "handleSynchronizeChange: sent conditional signal");

    return EN_NOEVENT;
}

};	/* End of 'namespace clusterlib' */
