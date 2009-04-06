/*
 * clusterlib.cc --
 *
 * Implementation of the Factory class.
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
 * Implement 'ready' protocol for notifyable objects.
 */
Event
CachedObjectChangeHandlers::handleNotifyableReady(NotifyableImpl *ntp,
                                                  int32_t etype,
                                                  const string &key)
{
    TRACE(CL_LOG, "handleNotifyableReady");

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 key.c_str());
        return EN_NOEVENT;
    }

    LOG_WARN(CL_LOG, 
              "handleNotifyableReady(%s)",
              ntp->getKey().c_str());

    getOps()->establishNotifyableReady(ntp);
    return EN_READY;
}

/*
 * Note the existence of a new notifyable, or the destruction of
 * an existing notifyable.
 */
Event
CachedObjectChangeHandlers::handleNotifyableExists(NotifyableImpl *ntp,
                                                   int32_t etype,
                                                   const string &key)
{
    TRACE(CL_LOG, "handleNotifyableExists");

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 key.c_str());
        return EN_NOEVENT;
    }

    LOG_WARN(CL_LOG,
             "Got exists event: %d on notifyable: \"%s\"",
             etype,
             ntp->getKey().c_str());

    /*
     * Re-establish interest in the existence of this notifyable.
     */
    getOps()->establishNotifyableInterest(ntp->getKey(),
                                          &m_notifyableExistsHandler);

    /*
     * Now decide what to do with the event.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_WARN(CL_LOG,
                 "Deleted event for key: %s",
                 key.c_str());
        ntp->setState(Notifyable::REMOVED);
        return EN_DELETED;
    }
    if (etype == ZOO_CREATED_EVENT) {
        LOG_WARN(CL_LOG,
                 "Created event for key: %s",
                 key.c_str());
        getOps()->establishNotifyableReady(ntp);
        return EN_CREATED;
    }
    if (etype == ZOO_CHANGED_EVENT) {
        LOG_WARN(CL_LOG,
                 "Changed event for key: %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * SHOULD NOT HAPPEN!
     */
    LOG_ERROR(CL_LOG,
              "Illegal event %d for key %s",
              etype,
              key.c_str());
    ::abort();
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
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 key.c_str());
        return EN_NOEVENT;
    }

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
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
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
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
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
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 key.c_str());
        return EN_NOEVENT;
    }

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
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * Convert the NotifyableImpl into a Properties *
     */
    PropertiesImpl *prop = dynamic_cast<PropertiesImpl *>(ntp);
    string nv = "";

    /*
     * If there's no PropertiesImpl * then punt.
     */
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
                 "Conversion to DataDistributionImpl * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

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
     * If the given NotifyableImpl is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No NotifyableImpl provided -- punting");
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
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
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
                 "Punting on event: %d on %s",
                 etype,
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
            throw ClusterException(string("handlePrecLeaderChange: ") +
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

    /*
     * If there's no notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 key.c_str());
        return EN_NOEVENT;
    }

    if (etype == ZOO_DELETED_EVENT) {
        /*
         * This is the only expected event.
         */
        LOG_DEBUG(CL_LOG, 
                  "handlePrecLockNodeExistsChange: ZOO_DELETED_EVENT called");

        /*
         * Notify the thread waiting to acquire the lock that this
         * lock node has finally been deleted.  Since this object
         * cannot be deleted it should be safe 
         */
        WaitMap::iterator waitMapIt = 
            getOps()->getDistrbutedLocks()->getWaitMap()->find(key);
        if (waitMapIt != getOps()->getDistrbutedLocks()->getWaitMap()->end()) {
            throw ClusterException("handlePrecLockNodeExistsChange: Signalling "
                                   "the thread waiting on acquire failed");
        }

        waitMapIt->second->predSignal();

        return EN_NOEVENT;
    }
    else {
        LOG_ERROR(CL_LOG, 
                  "handlePrecLockNodeExistsChange: non-ZOO_DELETED_EVENT "
                  "event %d called", etype);
        throw ClusterException("handlePrecLockNodeExistsChange: "
                               "non-ZOO_DELETED_EVENT called");
    }
    
    return EN_LOCKNODECHANGE;
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
