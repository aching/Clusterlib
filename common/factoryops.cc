/*
 * factoryops.cc --
 *
 * Implementation of the FactoryOps class.
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
 * Constructor of FactoryOps.
 *
 * Connect to cluster via the registry specification.
 * Initialize all the cache data structures.
 * Create the associated FactoryOps delegate object.
 */
FactoryOps::FactoryOps(const string &registry)
    : m_root(NULL),
      m_syncId(0),
      m_syncIdCompleted(0),
      m_endEventDispatched(false),
      m_config(registry, 3000),
      m_zk(m_config, NULL, false),
      m_timerEventAdapter(m_timerEventSrc),
      m_zkEventAdapter(m_zk),
      m_shutdown(false),
      m_connected(false),
      m_changeHandlers(this),
      m_distributedLocks(this)
{
    TRACE(CL_LOG, "FactoryOps");

    /*
     * Clear all the collections (lists, maps)
     * so that they start out empty.
     */
    m_dists.clear();
    m_apps.clear();
    m_groups.clear();
    m_nodes.clear();
    m_clients.clear();

    /*
     * Clear the multi-map of leadership election watches.
     */
    m_leadershipWatches.clear();

    /*
     * Link up the event sources.
     */
    m_timerEventAdapter.addListener(&m_eventAdapter);
    m_zkEventAdapter.addListener(&m_eventAdapter);

    /*
     * Create the event dispatch thread.
     */
    m_eventThread.Create(*this, &FactoryOps::dispatchEvents);

    /*
     * Create the timer handler thread.
     */
    m_timerHandlerThread.Create(*this, &FactoryOps::consumeTimerEvents);

    /*
     * Connect to ZK.
     */
    try {
        m_zk.reconnect();
        LOG_WARN(CL_LOG, 
                 "Waiting for connect event from ZooKeeper");
        if (m_eventSyncLock.lockedWait(3000) == false) {
	    throw ClusterException(
		"Did not receive connect event in time, aborting");
        }
        LOG_WARN(CL_LOG, 
                 "After wait, m_connected == %d",
                 static_cast<int>(m_connected));
    } catch (zk::ZooKeeperException &e) {
        m_zk.disconnect();
        throw ClusterException(e.what());
    }

    /*
     * Load up the clusterlib root of all objects.
     */
    getRoot();
}

/*
 * Destructor of FactoryOps
 */
FactoryOps::~FactoryOps()
{
    TRACE(CL_LOG, "~FactoryOps");

    injectEndEvent();
    waitForThreads();

    removeAllClients();
    removeAllDataDistributions();
    removeAllProperties();
    removeAllApplications();
    removeAllGroups();
    removeAllNodes();
    delete m_root;

    try {
        m_zk.disconnect();
    } catch (zk::ZooKeeperException &e) {
        LOG_WARN(CL_LOG,
                 "Got exception during disconnect: %s",
                 e.what());
    }
}

/*
 * Inject an END event so that all threads and clients
 * event loops will finish up in an orderly fashion.
 */
void
FactoryOps::injectEndEvent()
{
    TRACE(CL_LOG, "injectEndEvent");

    Locker l(getEndEventLock());

    if (m_endEventDispatched) {
        return;
    }
    m_zk.injectEndEvent();
}

/*
 * Wait for all my threads.
 */
void
FactoryOps::waitForThreads()
{
    TRACE(CL_LOG, "waitForThreads");

    m_eventThread.Join();
    m_timerHandlerThread.Join();
}

/*
 * Create a client.
 */
Client *
FactoryOps::createClient()
{
    TRACE(CL_LOG, "createClient");

    /*
     * Ensure we're connected.
     */
    if (!m_connected) {
        LOG_ERROR(CL_LOG, "Cannot create client when disconnected.");
        return NULL;
    }

    /*
     * Create the new client and add it to the registry.
     */
    try {
        ClientImpl *cp = new ClientImpl(this);
        addClient(cp);
        return cp;
    } catch (ClusterException &e) {
	LOG_WARN(CL_LOG, 
                 "Couldn't create client because: %s", 
                 e.what());
        return NULL;
    }
}

/*
 * Create a server.
 */
Server *
FactoryOps::createServer(Group *group,
                         const string &nodeName,
                         HealthChecker *checker,
                         ServerFlags flags)
{
    TRACE(CL_LOG, "createServer");

    /*
     * Ensure we're connected.
     */
    if (!m_connected) {
        LOG_ERROR(CL_LOG, "Cannot create server when disconnected.");
        return NULL;
    }

    /*
     * Create the new server and add it to the registry.
     */
    try {
        ServerImpl *sp = new ServerImpl(this,
                                        group,
                                        nodeName,
                                        checker,
                                        flags);
        addClient(sp);
        return sp;
    } catch (ClusterException &e) {
	LOG_WARN(CL_LOG, 
                 "Couldn't create server with "
                 "group %s, node %s because: %s", 
                 group->getName().c_str(), 
                 nodeName.c_str(), 
                 e.what());
        return NULL;
    }
}

int64_t
FactoryOps::getCurrentTimeMillis()
{
    return Timer<int>::getCurrentTimeMillis();
}

bool
FactoryOps::isConnected() const
{
    return m_connected;
}

/*
 * Try to synchronize with the underlying data store.
 */
void
FactoryOps::synchronize()
{
    TRACE(CL_LOG, "synchronize");
    int32_t syncId = 0;

    /* 
     * Simple algorithm to ensure that each synchronize() called by
     * various clients gets a unique ID.  As long as there are not
     * 2^64 sync operations in progress, this will not be a problem.
     */
    {
        Locker l1(getSyncLock());
        if ((m_syncId < m_syncIdCompleted) ||
            (m_syncId == numeric_limits<int64_t>::max())) {
            throw ClusterException("synchronize: sync invariant not "
                                   "maintained");
        }
        
        /*
         * Reset m_syncId and m_syncIdCompleted if there are no
         * outstanding syncs and are not at the initial state. 
         */
        if ((m_syncId == m_syncIdCompleted) && (m_syncId != 0)) {
            m_syncId = 0;
            m_syncIdCompleted = 0;
        }
        ++m_syncId;
        syncId = m_syncId;
    }

    string key(ClusterlibStrings::ROOTNODE);
    key.append(ClusterlibStrings::CLUSTERLIB);
    SAFE_CALL_ZK(m_zk.sync(key, 
                           &m_zkEventAdapter, 
                           getChangeHandlers()->getSynchronizeChangeHandler()),
                 "Could not synchronize with the underlying store %s: %s",
                 "/",
                 true,
                 true);

    /* 
     * Wait for notification of the event to be received by *
     * m_eventAdapter.
     */
    {
        Locker l1(getSyncLock());
        while (syncId > m_syncIdCompleted) {
            m_syncCond.wait(m_syncLock);
        }
    }

    LOG_DEBUG(CL_LOG, "synchronize: Complete");
}


/**********************************************************************/
/* Below this line are the methods of class FactoryOps that provide
 * functionality beyond what Factory needs.  */
/**********************************************************************/

/*
 * Add and remove clients.
 */
void
FactoryOps::addClient(ClientImpl *clp)
{
    TRACE(CL_LOG, "addClient");

    Locker l(getClientsLock());
    
    m_clients.push_back(clp);
}

void
FactoryOps::removeClient(ClientImpl *clp)
{
    TRACE(CL_LOG, "removeClient");

    Locker l(getClientsLock());
    ClientImplList::iterator clIt = find(m_clients.begin(),
                                         m_clients.end(),
                                         clp);

    if (clIt == m_clients.end()) {
        return;
    }
    m_clients.erase(clIt);
}

void
FactoryOps::removeAllClients()
{
    TRACE(CL_LOG, "removeAllClients");

    Locker l(getClientsLock());
    ClientImplList::iterator clIt;
    for (clIt  = m_clients.begin();
         clIt != m_clients.end();
         clIt++)
    {
	delete *clIt;
    }
    m_clients.clear();
}

/*
 * Methods to clean up storage used by a FactoryOps.
 */
void
FactoryOps::removeAllDataDistributions()
{
    TRACE(CL_LOG, "removeAllDataDistributions");

    Locker l(getDataDistributionsLock());
    DataDistributionImplMap::iterator distIt;

    for (distIt = m_dists.begin();
         distIt != m_dists.end();
         distIt++)
    {
	delete distIt->second;
    }
    m_dists.clear();
}
void
FactoryOps::removeAllProperties()
{
    TRACE(CL_LOG, "removeAllProperties");

    Locker l(getPropertiesLock());
    PropertiesImplMap::iterator pIt;

    for (pIt = m_properties.begin();
         pIt != m_properties.end();
         pIt++)
    {
	delete pIt->second;
    }
    m_properties.clear();
}
void
FactoryOps::removeAllApplications()
{
    TRACE(CL_LOG, "removeAllApplications");

    Locker l(getApplicationsLock());
    ApplicationImplMap::iterator aIt;

    for (aIt = m_apps.begin();
         aIt != m_apps.end();
         aIt++)
    {
	delete aIt->second;
    }
    m_apps.clear();
}
void
FactoryOps::removeAllGroups()
{
    TRACE(CL_LOG, "removeAllGroups");

    Locker l(getGroupsLock());
    GroupImplMap::iterator gIt;

    for (gIt = m_groups.begin();
         gIt != m_groups.end();
         gIt++)
    {
	delete gIt->second;
    }
    m_groups.clear();
}
void
FactoryOps::removeAllNodes()
{
    TRACE(CL_LOG, "removeAllNodes");

    Locker l(getNodesLock());
    NodeImplMap::iterator nIt;

    for (nIt = m_nodes.begin();
         nIt != m_nodes.end();
         nIt++)
    {
	delete nIt->second;
    }
    m_nodes.clear();
}

/*
 * Dispatch events to all registered clients.
 */
void
FactoryOps::dispatchEvents()
{
    TRACE(CL_LOG, "dispatchEvents");

    uint32_t eventSeqId = 0;

    LOG_DEBUG(CL_LOG,
              "Hello from dispatchEvents(), this: 0x%x, thread: %d",
              (unsigned int) this,
              (unsigned int) pthread_self());

    try {
        for (m_shutdown = false;
             m_shutdown == false;
             ) 
        {
            LOG_INFO(CL_LOG,
                     "[%d]: Asking for next event",
                     eventSeqId);
            eventSeqId++;

            /*
             * Get the next event and send it off to the
             * correct handler.
             */
            GenericEvent ge(m_eventAdapter.getNextEvent());

            LOG_DEBUG(CL_LOG,
                      "[%d, 0x%x] dispatchEvents() received event of "
                      "type %d",
                      eventSeqId,
                      (unsigned int) this,
                      (unsigned int) ge.getType());
            
            switch (ge.getType()) {
                default:
                    LOG_FATAL(CL_LOG,
                              "Illegal event with type %d",
                              ge.getType());
                    ::abort();
                    
                case ILLEGALEVENT:
                    LOG_FATAL(CL_LOG, "Illegal event");
                    ::abort();
                    
                case TIMEREVENT:
                {
                    ClusterlibTimerEvent *tp =
                        (ClusterlibTimerEvent *) ge.getEvent();
                    
                    LOG_DEBUG(CL_LOG,
                              "Dispatching timer event: 0x%x, id: "
                              "%d, alarm time: %lld",
                              (unsigned int) tp,
                              tp->getID(),
                              tp->getAlarmTime());
                    
                    dispatchTimerEvent(tp);
                    
                    break;
                }

                case ZKEVENT:
                {
                    zk::ZKWatcherEvent *zp =
                        (zk::ZKWatcherEvent *) ge.getEvent();
                    
                    LOG_DEBUG(CL_LOG,
                              "Processing ZK event "
                              "(type: %d, state: %d, context: 0x%x, path %s)",
                              zp->getType(),
                              zp->getState(),
                              (unsigned int) zp->getContext(),
                              zp->getPath().c_str());
                    
                    if ((zp->getType() == ZOO_SESSION_EVENT) &&
                        (zp->getPath().compare(ClusterlibStrings::SYNC) 
                         != 0)) {
                        dispatchSessionEvent(zp);
                    } 
                    else {
                        dispatchZKEvent(zp);
                    }
                    
                    break;
                }
            }
        }

        /*
         * After the event loop, we inform all
         * registered clients that there will
         * be no more events coming.
         */
        dispatchEndEvent();
    } catch (zk::ZooKeeperException &zke) {
        LOG_ERROR(CL_LOG, "ZooKeeperException: %s", zke.what());
        dispatchEndEvent();
        throw ClusterException(zke.what());
    } catch (ClusterException &ce) {
        dispatchEndEvent();
        throw ClusterException(ce.what());
    } catch (std::exception &stde) {
        LOG_ERROR(CL_LOG, "Unknown exception: %s", stde.what());
        dispatchEndEvent();
        throw ClusterException(stde.what());
    }
}

/*
 * Dispatch a timer event.
 */
void
FactoryOps::dispatchTimerEvent(ClusterlibTimerEvent *tep)
{
    TRACE(CL_LOG, "dispatchTimerEvent");

    if (tep == NULL) {
        m_timerEventQueue.put(NULL);
    } 
    else {
        TimerEventPayload *tp = (TimerEventPayload *) tep->getUserData();
        m_timerEventQueue.put(tp);
    }
}

/*
 * Dispatch a ZK event.
 */
void
FactoryOps::dispatchZKEvent(zk::ZKWatcherEvent *zp)
{
    TRACE(CL_LOG, "dispatchZKEvent");
    
    if (!zp) {
	throw ClusterException("Unexpected NULL ZKWatcherEvent");
    }

    CachedObjectEventHandler *fehp =
        (CachedObjectEventHandler *) zp->getContext();
    ClusterEventPayload *cep, *cepp;
    ClientImplList::iterator clIt;
    char buf[1024];

    /*
     * Protect against NULL context.
     */
    if (fehp == NULL) {
        snprintf(buf,
                 1024,
                 "type: %d, state: %d, key: %s",
                 zp->getType(),
                 zp->getState(),
                 zp->getPath().c_str());
        throw ClusterException(string("Unexpected NULL event context: ") +
                               buf);
    }

    LOG_DEBUG(CL_LOG, "Dispatching ZK event!");

    /*
     * Update the cache representation of the clusterlib
     * repository object and get back a prototypical
     * cluster event payload to send to clients.
     *
     * If NULL is returned, the event is not propagated
     * to clients.
     */
    cep = updateCachedObject(fehp, zp);
    if (cep == NULL) {
        LOG_DEBUG(CL_LOG, 
                  "dispatchZKEvent: NULL cluster event payload "
                  "will not propogate to clients");
        return;
    }

    /*
     * Now dispatch the event to all registered clients in
     * case they have a registered handler for the event on
     * the affected clusterlib repository object.
     */
    {
        Locker l(getClientsLock());

        for (clIt = m_clients.begin();
             clIt != m_clients.end(); 
             clIt++) {
            cepp = new ClusterEventPayload(*cep);
            (*clIt)->sendEvent(cepp);
        }
    }
    delete cep;
}

/*
 * Dispatch a session event. These events
 * are in fact handled directly, here.
 */
void
FactoryOps::dispatchSessionEvent(zk::ZKWatcherEvent *zep)
{
    TRACE(CL_LOG, "dispatchSessionEvent");

    LOG_DEBUG(CL_LOG,
              "dispatchSessionEvent: (type: %d, state: %d, key: %s)",
              zep->getType(), 
              zep->getState(),
              zep->getPath().c_str());

    if ((zep->getState() == ZOO_ASSOCIATING_STATE) ||
        (zep->getState() == ZOO_CONNECTING_STATE)) {
        /*
         * Not really clear what to do here.
         * For now do nothing.
         */
#ifdef	VERY_VERY_VERBOSE
        LOG_TRACE(CL_LOG, "Do nothing.");
#endif
    } 
    else if (zep->getState() == ZOO_CONNECTED_STATE) {
        /*
         * Mark as connected.
         */
#ifdef	VERY_VERY_VERBOSE
        LOG_TRACE(CL_LOG, "Marked connected.");
#endif
        m_connected = true;

        /*
         * Notify anyone waiting that this factory is
         * now connected.
         */
        m_eventSyncLock.lockedNotify();
    }
    else if (zep->getState() == ZOO_EXPIRED_SESSION_STATE) {
        /*
         * We give up on SESSION_EXPIRED.
         */
#ifdef	VERY_VERY_VERBOSE
        LOG_WARN(CL_LOG, "Giving up.");
#endif
        m_shutdown = true;
        m_connected = false;

        /*
         * Notify anyone waiting that this factory is
         * now disconnected.
         */
        m_eventSyncLock.lockedNotify();
    }
    else {
        LOG_WARN(CL_LOG,
                 "Session event with unknown state "
                 "(type: %d, state: %d)",
                 zep->getType(), 
                 zep->getState());
    }
}

/*
 * Dispatch a final event to all registered clients
 * to indicate that no more events will be sent
 * following this one.
 */
bool
FactoryOps::dispatchEndEvent()
{
    TRACE(CL_LOG, "dispatchEndEvent");

    ClientImplList::iterator clIt;

    /*
     * If the end event was already dispatched, don't do
     * it again.
     */
    {
        Locker l(getEndEventLock());

        if (m_endEventDispatched) {
            LOG_WARN(CL_LOG,
                     "Attempt to dispatch END event more than once!");
            return false;
        }
        m_endEventDispatched = true;
    }

    /*
     * Send a terminate signal to the timer
     * event handler thread.
     */
    dispatchTimerEvent(NULL);

    /*
     * Send a terminate signal to all registered
     * client-specific cluster event handler threads.
     */
    for (clIt = m_clients.begin();
         clIt != m_clients.end();
         clIt++) {
        (*clIt)->sendEvent(NULL);
    }

    return true;
}

/*
 * Consume timer events. Run the timer event handlers.
 * This runs in a special thread owned by the factory.
 */
void
FactoryOps::consumeTimerEvents()
{
    TRACE(CL_LOG, "consumeTimerEvents");

    TimerEventPayload *tepp;

#ifdef	VERY_VERY_VERBOSE
    LOG_WARN(CL_LOG,
             "Hello from consumeTimerEvents, this: 0x%x, thread: %d",
             this,
             (uint32_t) pthread_self());
#endif

    try {
        for (;;) {
            tepp = m_timerEventQueue.take();

            /*
             * If we received the terminate signal,
             * then exit from the loop.
             */
            if (tepp == NULL) {
                LOG_INFO(CL_LOG,
                         "Received terminate signal, finishing loop");
                return;
            }

            /*
             * Dispatch the event to its handler, if the
             * event hadn't been cancelled.
             */
            if (!tepp->cancelled()) {
                tepp->getHandler()->handleTimerEvent(tepp->getId(),
                                                     tepp->getData());
            }

            LOG_INFO(CL_LOG,
                     "Serviced timer %d, handler 0x%x, client data 0x%x",
                     tepp->getId(), 
                     (int) tepp->getHandler(),
                     (int) tepp->getData());

            /*
             * Deallocate the payload object.
             */

            {
                Locker l(getTimersLock());

                m_timerRegistry.erase(tepp->getId());
                delete tepp;
            }
        }
    } catch (zk::ZooKeeperException &zke) {
        LOG_ERROR(CL_LOG, "ZooKeeperException: %s", zke.what());
        throw ClusterException(zke.what());
    } catch (ClusterException &ce) {
        throw ClusterException(ce.what());
    } catch (std::exception &stde) {
        LOG_ERROR(CL_LOG, "Unknown exception: %s", stde.what());
        throw ClusterException(stde.what());
    }        
}

/*
 * Methods to retrieve entities (or create them) from the
 * FactoryOps's cache.
 */
RootImpl *
FactoryOps::getRoot()
{
    TRACE(CL_LOG, "getRoot");

    if (m_root) {
        return m_root;
    }

    string key = NotifyableKeyManipulator::createRootKey();
    bool exists = false;
    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      key, 
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNotifyableExistsHandler())),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        string applications = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::APPLICATIONS;

        SAFE_CALL_ZK(m_zk.createNode(applications, "", 0, true),
                     "Could not create key %s: %s",
                     applications.c_str(),
                     true,
                     true);
        
        SAFE_CALL_ZK(m_zk.setNodeData(key, "ready", 0),
                     "Could not complete ready protocol for %s: %s",
                     key.c_str(),
                     true,
                     true);
    }
    
    m_root = new RootImpl("root", key, this);
    return m_root;
}

ApplicationImpl *
FactoryOps::getApplication(const string &appName, bool create)
{
    TRACE(CL_LOG, "getApplication");

    /*
     * Do not allow empty names, and names containing '/'.
     */
    if (appName.empty() || (appName.find('/') != string::npos)) {
        LOG_WARN(CL_LOG,
                 "getApplication: illegal application name %s",
                 appName.c_str());
        return NULL;
    }

    string key = NotifyableKeyManipulator::createAppKey(appName);

    {
        Locker l(getApplicationsLock());
        ApplicationImplMap::const_iterator appIt = m_apps.find(key);

        if (appIt != m_apps.end()) {
            return appIt->second;
        }
    }

    ApplicationImpl *app = loadApplication(appName, key);
    if (app != NULL) {
        return app;
    }
    if (create == true) {
        return createApplication(appName, key);
    }

    LOG_WARN(CL_LOG,
             "getApplication: application %s not found nor created",
             appName.c_str());

    return NULL;
}

DataDistributionImpl *
FactoryOps::getDataDistribution(const string &distName,
                                GroupImpl *parentGroup,
                                bool create)
{
    TRACE(CL_LOG, "getDataDistribution");

    /*
     * Do not allow empty names, and names containing '/'.
     */
    if (distName.empty() || (distName.find('/') != string::npos)) {
        LOG_WARN(CL_LOG,
                 "getDataDistribution: Illegal data distribution name %s",
                 distName.c_str());
        return NULL;
    }

    if (parentGroup == NULL) {
        LOG_WARN(CL_LOG, "NULL parent group");
        return NULL;
    }

    string key = NotifyableKeyManipulator::createDistKey(parentGroup->getKey(),
                                                         distName);

    {
        Locker l(getDataDistributionsLock());
        DataDistributionImplMap::const_iterator distIt = 
            m_dists.find(key);

        if (distIt != m_dists.end()) {
            return distIt->second;
        }
    }
    DataDistributionImpl *distp = loadDataDistribution(distName, 
                                                       key, 
                                                       parentGroup);
    if (distp != NULL) {
        return distp;
    }
    if (create == true) {
        return createDataDistribution(distName, key, "", "", parentGroup);
    }

    LOG_WARN(CL_LOG,
             "getDataDistribution: data distribution %s not found "
             "nor created",
             distName.c_str());

    return NULL;
}

PropertiesImpl *
FactoryOps::getProperties(Notifyable *parent,
                          bool create)
{
    TRACE(CL_LOG, "getProperties");

    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getProperties: NULL parent");                 
        return NULL;
    }

    string key = 
        NotifyableKeyManipulator::createPropertiesKey(parent->getKey());

    {
        Locker l(getPropertiesLock());
        PropertiesImplMap::const_iterator propIt = m_properties.find(key);

        if (propIt != m_properties.end()) {
            return propIt->second;
        }
    }

    PropertiesImpl *prop = loadProperties(key, parent);
    if (prop != NULL) {
        return prop;
    }
    if (create == true) {
        return createProperties(key, parent);
    }

    LOG_WARN(CL_LOG,
             "getProperties: could not find nor create "
             "properties for %s",
             parent->getKey().c_str());

    return NULL;
}

GroupImpl *
FactoryOps::getGroup(const string &groupName,
                     GroupImpl *parentGroup,
                     bool create)
{
    TRACE(CL_LOG, "getGroup");

    /*
     * Do not allow empty names, and names containing '/'.
     */
    if (groupName.empty() || (groupName.find('/') != string::npos)) {
        LOG_WARN(CL_LOG,
                 "getGroup: Illegal group name %s",
                 groupName.c_str());
        return NULL;
    }

    if (parentGroup == NULL) {
        LOG_WARN(CL_LOG, "getGroup: NULL parent group");
        return NULL;
    }
    string key = 
        NotifyableKeyManipulator::createGroupKey(parentGroup->getKey(),
                                                 groupName);

    {
        Locker l(getGroupsLock());
        GroupImplMap::const_iterator groupIt = m_groups.find(key);

        if (groupIt != m_groups.end()) {
            return groupIt->second;
        }
    }

    GroupImpl *group = loadGroup(groupName, key, parentGroup);
    if (group != NULL) {
        return group;
    }
    if (create == true) {
        return createGroup(groupName, key, parentGroup);
    }

    LOG_WARN(CL_LOG,
             "getGroup: group %s not found nor created",
             groupName.c_str());

    return NULL;
}

NodeImpl *
FactoryOps::getNode(const string &nodeName,
                    GroupImpl *parentGroup,
                    bool managed,
                    bool create)
{
    TRACE(CL_LOG, "getNode");

    /*
     * Do not allow empty names, and names containing '/'.
     */
    if (nodeName.empty() || (nodeName.find('/') != string::npos)) {
        LOG_WARN(CL_LOG,
                 "getNode: Illegal node name %s",
                 nodeName.c_str());
        return NULL;
    }

    if (parentGroup == NULL) {
        LOG_WARN(CL_LOG, "NULL parent group");
        return NULL;
    }

    string key = NotifyableKeyManipulator::createNodeKey(parentGroup->getKey(),
                                                         nodeName,
                                                         managed);

    {
        Locker l(getNodesLock());
        NodeImplMap::const_iterator nodeIt = m_nodes.find(key);

        if (nodeIt != m_nodes.end()) {
            return nodeIt->second;
        }
    }

    NodeImpl *np = loadNode(nodeName, key, parentGroup);
    if (np != NULL) {
        return np;
    }
    if (create == true) {
        return createNode(nodeName, key, parentGroup);
    }

    LOG_WARN(CL_LOG,
             "getNode: node %s not found nor created",
             nodeName.c_str());

    return NULL;
}

/*
 * Update the fields of a distribution in the clusterlib repository.
 */
void
FactoryOps::updateDataDistribution(const string &distKey,
                                   const string &shards,
                                   const string &manualOverrides,
                                   int32_t shardsVersion,
                                   int32_t manualOverridesVersion)
{
    TRACE(CL_LOG, "updateDataDistribution");
    
    string snode = 
	distKey +
        ClusterlibStrings::KEYSEPARATOR +
	ClusterlibStrings::SHARDS;
    string monode =
        distKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::MANUALOVERRIDES;
    bool exists = false;

    /*
     * Update the shards.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(snode)),
                 "Could not determine whether key %s exists: %s",
                 snode.c_str(),
                 false,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(snode, shards, 0),
                     "Creation of %s failed: %s",
                     snode.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(snode, shards, shardsVersion),
                 "Setting of %s failed: %s",
                 snode.c_str(),
                 false,
                 true);
    SAFE_CALL_ZK(m_zk.getNodeData(
                     snode,
                     &m_zkEventAdapter, 
                     getChangeHandlers()->getShardsChangeHandler()),
                 "Reestablishing watch on value of %s failed: %s",
                 snode.c_str(),
                 true,
                 true);

    /*
     * Update the manual overrides.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(monode)),
                 "Could not determine whether key %s exists: %s",
                 monode.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(monode,
                                     manualOverrides,
                                     0),
                     "Creation of %s failed: %s",
                     monode.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(monode,
                                  manualOverrides,
                                  manualOverridesVersion),
                 "Setting of %s failed: %s",
                 monode.c_str(),
                 false,
                 true);
    SAFE_CALL_ZK(m_zk.getNodeData(
                     monode,
                     &m_zkEventAdapter,
                     getChangeHandlers()->getManualOverridesChangeHandler()),
                 "Reestablishing watch on value of %s failed: %s",
                 monode.c_str(),
                 false,
                 true);
}

/*
 * Update the properties of a notifyable object in the clusterlib repository
 */
void
FactoryOps::updateProperties(const string &propsKey,
                             const string &propsValue,
                             int32_t versionNumber,
                             int32_t &finalVersionNumber)
{
    TRACE(CL_LOG, "updateProperties");

    Stat stat;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(propsKey)),
                 "Could not determine whether key %s exists: %s",
                 propsKey.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(propsKey, propsValue, 0),
                     "Creation of %s failed: %s",
                     propsKey.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(propsKey, 
                                  propsValue, 
                                  versionNumber, 
                                  &stat),
                 "Setting of %s failed: %s",
                 propsKey.c_str(),
                 false,
                 true);
    SAFE_CALL_ZK(m_zk.getNodeData(
                     propsKey,
                     &m_zkEventAdapter,
                     getChangeHandlers()->getPropertiesValueChangeHandler()),
                 "Reestablishing watch on value of %s failed: %s",
                 propsKey.c_str(),
                 false,
                 true);

    finalVersionNumber = stat.version;
}

/*
 * Update the client state field of a node.
 */
void
FactoryOps::updateNodeClientState(const string &nodeKey,
                                  const string &cs)
{
    TRACE(CL_LOG, "updateNodeClientState");

    string csKey = 
        nodeKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(csKey)),
                 "Could not determine whether key %s exists: %s",
                 csKey.c_str(),
                 true,
                 true);

    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(csKey, cs, 0),
                     "Creation of %s failed: %s",
                     csKey.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(csKey, cs),
                 "Setting of %s failed: %s",
                 csKey.c_str(),
                 false,
                 true);
    SAFE_CALL_ZK(m_zk.getNodeData(
                     csKey,
                     &m_zkEventAdapter,
                     getChangeHandlers()->getNodeClientStateChangeHandler()),
                 "Reestablishing watch on value of %s failed: %s",
                 csKey.c_str(),
                 false,
                 true);
}

/*
 * Update the client state description field for
 * a node.
 */
void
FactoryOps::updateNodeClientStateDesc(const string &nodeKey,
                                      const string &desc)
{
    TRACE(CL_LOG, "updateNodeClientStateDesc");

    string csKey = 
        nodeKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTSTATEDESC;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(csKey)),
                 "Could not determine whether key %s exists: %s",
                 csKey.c_str(),
                 true,
                 true);

    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(csKey, desc, 0),
                     "Creation of %s failed: %s",
                     csKey.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(csKey, desc),
                 "Setting of %s failed: %s",
                 csKey.c_str(),
                 false,
                 true);

    /*
     * NO WATCHER -- do we need one?
     */
}

/*
 * Update the master state field of a node.
 */
void
FactoryOps::updateNodeMasterSetState(const string &nodeKey,
                                  const string &ms)
{
    TRACE(CL_LOG, "updateNodeMasterSetState");

    string msKey = 
        nodeKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::MASTERSETSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(msKey)),
                 "Could not determine whether key %s exists: %s",
                 msKey.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(msKey, ms, 0),
                     "Creation of %s failed: %s",
                     msKey.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(msKey, ms),
                 "Setting of %s failed: %s",
                 msKey.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.getNodeData(msKey,
                                  &m_zkEventAdapter,
                                  getChangeHandlers()->getNodeMasterSetStateChangeHandler()),
                 "Reestablishing watch on value of %s failed: %s",
                 msKey.c_str(),
                 false,
                 true);
}

/*
 * Get (& potentially load) cache objects given a key.
 */

NotifyableImpl *
FactoryOps::getNotifyableFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getNotifyableFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getNotifyableFromComponents(components, -1, create);
}
NotifyableImpl *
FactoryOps::getNotifyableFromComponents(const vector<string> &components,
                                        int32_t elements, 
                                        bool create)
{
    TRACE(CL_LOG, "getNotifyableFromComponents");

    if (elements == -1) {
        elements = components.size();
    }

    NotifyableImpl *ntp = NULL;    
    int32_t clusterObjectElements = elements;

    LOG_DEBUG(CL_LOG, "getNotifyableFromComponents: elements %d", elements);

    while (clusterObjectElements >= ClusterlibInts::ROOT_COMPONENTS_COUNT) {
        ntp = getRootFromComponents(components,
                                    clusterObjectElements);
        if (ntp) {
            LOG_DEBUG(CL_LOG, 
                      "getNotifyableFromComponents: found Root");
            return ntp;
        }
        ntp = getApplicationFromComponents(components,
                                           clusterObjectElements, 
                                           create);
        if (ntp) {
            LOG_DEBUG(CL_LOG, 
                      "getNotifyableFromComponents: found Application");
            return ntp;
        }
        ntp = getPropertiesFromComponents(components,
                                          clusterObjectElements, 
                                          create);
        if (ntp) {
            LOG_DEBUG(CL_LOG, 
                      "getNotifyableFromComponents: found Properties");
            return ntp;
        }
        ntp = getDataDistributionFromComponents(components,
                                                clusterObjectElements, 
                                                create);
        if (ntp) {
            LOG_DEBUG(CL_LOG, 
                      "getNotifyableFromComponents: found DataDistribution");
            return ntp;
        }
        ntp = getGroupFromComponents(components,
                                     clusterObjectElements, 
                                     create);
        if (ntp) {
            LOG_DEBUG(CL_LOG, 
                      "getNotifyableFromComponents: found Group");
            return ntp;
        }
        ntp = getNodeFromComponents(components, 
                                    clusterObjectElements, 
                                    create);
        if (ntp) {
            LOG_DEBUG(CL_LOG, 
                      "getNotifyableFromComponents: found Node");
            return ntp;
        }

        clusterObjectElements = 
            NotifyableKeyManipulator::removeObjectFromComponents(
                components, 
                clusterObjectElements);
    }

    return NULL;
}

RootImpl *
FactoryOps::getRootFromKey(const string &key)
{
    TRACE(CL_LOG, "getRootFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getRootFromComponents(components, -1);
}
RootImpl *
FactoryOps::getRootFromComponents(const vector<string> &components, 
                                  int32_t elements)
{
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isRootKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getRootFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getRootFromComponents: root name = %s", 
              components.at(elements - 1).c_str());

    return getRoot();
}

ApplicationImpl *
FactoryOps::getApplicationFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getApplicationFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getApplicationFromComponents(components, -1, create);
}
ApplicationImpl *
FactoryOps::getApplicationFromComponents(const vector<string> &components, 
                                         int32_t elements,
                                         bool create)
{
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isApplicationKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getApplicationFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getApplicationFromComponents: application name = %s", 
              components.at(elements - 1).c_str());

    return getApplication(components.at(elements - 1), create);    
}

DataDistributionImpl *
FactoryOps::getDataDistributionFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getDataDistributionFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getDataDistributionFromComponents(components, -1, create);
}
DataDistributionImpl *
FactoryOps::getDataDistributionFromComponents(
    const vector<string> &components, 
    int32_t elements,
    bool create)
{
    TRACE(CL_LOG, "getDataDistributionFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isDataDistributionKey(components, 
                                                         elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getDataDistributionFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    GroupImpl *parent = getGroupFromComponents(components,
                                               parentGroupCount,
                                               create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getDataDistributionFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getDataDistributionFromComponents: parent key = %s, "
              "distribution name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    return getDataDistribution(components.at(elements - 1),
                               parent,
                               create);
}

PropertiesImpl *
FactoryOps::getPropertiesFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getPropertiesFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getPropertiesFromComponents(components, -1, create);
}
PropertiesImpl *
FactoryOps::getPropertiesFromComponents(const vector<string> &components, 
                                        int32_t elements,
                                        bool create)
{
    TRACE(CL_LOG, "getPropertiesFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

   if (!NotifyableKeyManipulator::isPropertiesKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getPropertiesFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    Notifyable *parent = getNotifyableFromComponents(components,
                                                     parentGroupCount,
                                                     create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getPropertiesFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getPropertiesFromComponents: parent key = %s, "
              "properties name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());
    
    return getProperties(parent,
                         create);
}

GroupImpl *
FactoryOps::getGroupFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getGroupFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getGroupFromComponents(components, -1, create);
}
GroupImpl *
FactoryOps::getGroupFromComponents(const vector<string> &components, 
                                   int32_t elements,
                                   bool create)
{
    TRACE(CL_LOG, "getGroupFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /* 
     * Could be either an Application or a Group, need to check both.
     * If is a Group and not an Application, then it must have a
     * parent that is either an Application or Group.  If the parent
     * is a Group, this function will call itself recursively.
     */
    if (NotifyableKeyManipulator::isApplicationKey(components, elements)) {
        return getApplicationFromComponents(components, elements, create);
    }
    
    if (!NotifyableKeyManipulator::isGroupKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getGroupFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    GroupImpl *parent = getGroupFromComponents(components,
                                           parentGroupCount,
                                           create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getGroupFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }
    
    LOG_DEBUG(CL_LOG, 
              "getGroupFromComponents: parent key = %s, group name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    return getGroup(components.at(elements - 1),
                   parent,
                   create);
}

NodeImpl *
FactoryOps::getNodeFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getNodeFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getNodeFromComponents(components, -1, create);
}
NodeImpl *
FactoryOps::getNodeFromComponents(const vector<string> &components, 
                                  int32_t elements,
                                  bool create)
{
    TRACE(CL_LOG, "getNodeFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isNodeKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getNodeFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    GroupImpl *parent = getGroupFromComponents(components,
                                           parentGroupCount,
                                           create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getNodeFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getNodeFromComponents: parent key = %s, node name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());
    
    return getNode(components.at(elements - 1),
                   parent,
                   ((components.at(elements - 2)) == ClusterlibStrings::NODES),
                   create);
}

/*
 * Entity loading from ZooKeeper. Also add it to the global cache.
 */
ApplicationImpl *
FactoryOps::loadApplication(const string &name,
                            const string &key)
{
    TRACE(CL_LOG, "loadApplication");

    vector<string> zkNodes;
    ApplicationImpl *app;
    bool exists = false;
    Locker l(getApplicationsLock());

    ApplicationImplMap::const_iterator appIt = m_apps.find(key);
    if (appIt != m_apps.end()) {
        return appIt->second;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      key, 
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNotifyableExistsHandler())),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    app = new ApplicationImpl(name, key, this, getRoot());

    /*
     * Set up ready protocol.
     */
    establishNotifyableReady(app);

    m_apps[key] = app;

    return app;
}
    
DataDistributionImpl *
FactoryOps::loadDataDistribution(const string &distName,
                                 const string &distKey,
                                 GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "loadDataDistribution");

    DataDistributionImpl *distp;
    bool exists = false;

    /*
     * Ensure that we have a cached object for this data
     * distribution in the cache.
     */
    Locker l(getDataDistributionsLock());

    DataDistributionImplMap::const_iterator distIt = 
        m_dists.find(distKey);
    if (distIt != m_dists.end()) {
        return distIt->second;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(distKey)),
                 "Could not determine whether key %s exists: %s",
                 distKey.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    distp = new DataDistributionImpl(parentGroup,
                                     distName,
                                     distKey,
                                     this);
    m_dists[distKey] = distp;

    /*
     * Set up the 'ready' protocol.
     */
    establishNotifyableReady(distp);

    return distp;
}

string
FactoryOps::loadShards(const string &key, int32_t &version)
{
    Stat stat;
    string snode =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::SHARDS;
    string res = "";

    version = 0;
    SAFE_CALL_ZK((res = m_zk.getNodeData(
                      snode,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getShardsChangeHandler(),
                      &stat)),
                 "Loading shards from %s failed: %s",
                 snode.c_str(),
                 false,
                 true);
    version = stat.version;
    return res;
}

string
FactoryOps::loadManualOverrides(const string &key, int32_t &version)
{
    Stat stat;
    string monode =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::MANUALOVERRIDES;
    string res = "";

    version = 0;
    SAFE_CALL_ZK((res = m_zk.getNodeData(
                      monode,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getManualOverridesChangeHandler(),
                      &stat)),
                 "Loading manual overrides from %s failed: %s",
                 monode.c_str(),
                 false,
                 true);
    version = stat.version;
    return res;
}

PropertiesImpl *
FactoryOps::loadProperties(const string &propsKey,
                           Notifyable *parent)
{
    TRACE(CL_LOG, "Properties");

    PropertiesImpl *prop;
    bool exists = false;
    Locker l(getPropertiesLock());

    PropertiesImplMap::const_iterator propIt =
        m_properties.find(propsKey);
    if (propIt != m_properties.end()) {
        return propIt->second;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(propsKey)),
                 "Could not determine whether key %s exists: %s",
                 propsKey.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    SAFE_CALL_ZK(m_zk.getNodeData(
                     propsKey,
                     &m_zkEventAdapter,
                     getChangeHandlers()->getPropertiesValueChangeHandler()),
                 "Reading the value of %s failed: %s",
                 propsKey.c_str(),
                 false,
                 true);
    prop = new PropertiesImpl(dynamic_cast<NotifyableImpl *>(parent),
                              propsKey, 
                              this);
    m_properties[propsKey] = prop;

    /*
     * Initialize the data out of the repository into the cache. Needs
     * to be called directly here since properties do not participate
     * in the 'ready' protocol.
     */
    prop->initializeCachedRepresentation();

    return prop;
}

string
FactoryOps::loadKeyValMap(const string &key, int32_t &version)
{
    Stat stat;
    string kvnode = "";

    SAFE_CALL_ZK((kvnode = m_zk.getNodeData(
                      key,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getPropertiesValueChangeHandler(),
                      &stat)),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 false,
                 true);

    version = stat.version;

    return kvnode;
}

GroupImpl *
FactoryOps::loadGroup(const string &groupName,
                      const string &groupKey,
                      GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "loadGroup");

    GroupImpl *group;
    bool exists = false;
    Locker l(getGroupsLock());

    GroupImplMap::const_iterator groupIt = m_groups.find(groupKey);
    if (groupIt != m_groups.end()) {
        return groupIt->second;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(groupKey)),
                 "Could not determine whether key %s exists: %s",
                 groupKey.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    group = new GroupImpl(groupName,
                    groupKey,
                    this,
                    parentGroup);
    m_groups[groupKey] = group;

    /*
     * Set up ready protocol.
     */
    establishNotifyableReady(group);

    return group;
}

NodeImpl *
FactoryOps::loadNode(const string &name,
                     const string &key,
                     GroupImpl *group)
{
    TRACE(CL_LOG, "loadNode");

    NodeImpl *np;
    bool exists = false;
    Locker l(getNodesLock());

    NodeImplMap::const_iterator nodeIt = m_nodes.find(key);
    if (nodeIt != m_nodes.end()) {
        return nodeIt->second;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    np = new NodeImpl(group, name, key, this);
    m_nodes[key] = np;

    /*
     * Set up ready protocol.
     */
    establishNotifyableReady(np);

    return np;
}

/*
 * Entity creation in ZooKeeper.
 */
ApplicationImpl *
FactoryOps::createApplication(const string &name, const string &key)
{
    TRACE(CL_LOG, "createApplication");

    vector<string> zkNodes;
    ApplicationImpl *app = NULL;
    bool created = false;
    bool exists = false;
    
    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      key,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNotifyableExistsHandler())),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        string groups = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::GROUPS;
        string dists = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::DISTRIBUTIONS;
        string props = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::PROPERTIES;

        /*
         * Create the application data structure if needed.
         */
        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(groups, "", 0),
                     "Could not create key %s: %s",
                     groups.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(dists, "", 0),
                     "Could not create key %s: %s",
                     dists.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0),
                     "Could not create key %s: %s",
                     props.c_str(),
                     true,
                     true);
    }

    /*
     * Load the application object, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    app = loadApplication(name, key);

    /*
     * Trigger the 'ready' protocol if we created this
     * application. We need to do this for any *OTHER*
     * processes waiting for the application to be
     * ready.
     */
    if (created) {
        SAFE_CALL_ZK(m_zk.setNodeData(key, "ready", 0),
                     "Could not complete ready protocol for %s: %s",
                     key.c_str(),
                     true,
                     true);
    }

    return app;
}

DataDistributionImpl *
FactoryOps::createDataDistribution(const string &name,
                                   const string &key,
                                   const string &marshalledShards,
                                   const string &marshalledManualOverrides,
                                   GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "createDataDistribution");

    DataDistributionImpl *distp;
    bool created = false;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      key,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNotifyableExistsHandler())),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        string shards = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::SHARDS;
        string mos = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::MANUALOVERRIDES;
        string props = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(shards, "", 0),
                     "Could not create key %s: %s",
                     shards.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(mos, "", 0),
                     "Could not create key %s: %s",
                     mos.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0),
                     "Could not create key %s: %s",
                     props.c_str(),
                     true,
                     true);
    }

    /*
     * Load the distribution, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    distp = loadDataDistribution(name, key, parentGroup);

    /*
     * If we created the data distribution in the
     * repository, trigger the ready protocol. This is
     * needed for *OTHER* processes waiting till this
     * object is ready.
     */
    if (created) {
        SAFE_CALL_ZK(m_zk.setNodeData(key, "ready", 0),
                     "Could not complete ready protocol for %s: %s",
                     key.c_str(),
                     true,
                     true);
    }

    return distp;
}

PropertiesImpl *
FactoryOps::createProperties(const string &key,
                          Notifyable *parent) 
{
    TRACE(CL_LOG, "createProperties");
    bool exists = false;

    /*
     * Preliminaries: Ensure the node exists and
     * has the correct watchers set up.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      key,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNotifyableExistsHandler())),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.getNodeData(
                     key, 
                     &m_zkEventAdapter,
                     getChangeHandlers()->getPropertiesValueChangeHandler()),
                 "Could not read value of %s: %s",
                 key.c_str(),
                 false,
                 true);

    /*
     * Load the properties, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    PropertiesImpl *prop = loadProperties(key, parent);
    
    /*
     * No ready protocol necessary for Properties.
     */
    
    return prop;

}

GroupImpl *
FactoryOps::createGroup(const string &groupName, 
                        const string &groupKey,
                        GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "createGroup");

    GroupImpl *group = NULL;
    bool created = false;
    bool exists = false;
    
    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      groupKey,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNotifyableExistsHandler())),
                 "Could not determine whether key %s exists: %s",
                 groupKey.c_str(),
                 false,
                 true);
    if (!exists) {
        string nodes = 
            groupKey + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::NODES;
        string leadership = 
            groupKey + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::LEADERSHIP;
        string bids = 
            leadership + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::BIDS;
        string props = 
            groupKey + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(groupKey, "", 0),
                     "Could not create key %s: %s",
                     groupKey.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(nodes, "", 0),
                     "Could not create key %s: %s",
                     nodes.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(leadership, "", 0),
                     "Could not create key %s: %s",
                     leadership.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(bids, "", 0),
                     "Could not create key %s: %s",
                     bids.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0),
                     "Could not create key %s: %s",
                     props.c_str(),
                     true,
                     true);
    }

    /*
     * Load the group, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    group = loadGroup(groupName, groupKey, parentGroup);
    
    /*
     * Trigger the 'ready' protocol if we created the
     * group. This is for *OTHER* processes that are
     * waiting for the group to be ready.
     */
    if (created) {
        SAFE_CALL_ZK(m_zk.setNodeData(groupKey, "ready", 0),
                     "Could not complete ready protocol for %s: %s",
                     groupKey.c_str(),
                     true,
                     true);
    }
    
    return group;
}

NodeImpl *
FactoryOps::createNode(const string &name,
                       const string &key, 
                       GroupImpl *group)
{
    TRACE(CL_LOG, "createNode");

    NodeImpl *np = NULL;
    bool created = false;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      key,
                      &m_zkEventAdapter, 
                      getChangeHandlers()->getNotifyableExistsHandler())),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        string cs = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::CLIENTSTATE;
        string ms = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::MASTERSETSTATE;
        string cv = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::CLIENTVERSION;
        string props = 
            key + 
            ClusterlibStrings::KEYSEPARATOR + 
            ClusterlibStrings::PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(cs, "", 0),
                     "Could not create key %s: %s",
                     cs.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(ms, "", 0),
                     "Could not create key %s: %s",
                     ms.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(cv, "1.0", 0),
                     "Could not create key %s: %s",
                     cv.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0),
                     "Could not create key %s: %s",
                     props.c_str(),
                     true,
                     true);
    }

    /*
     * Load the node, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    np = loadNode(name, key, group);

    /*
     * Trigger the 'ready' protocol if we created the
     * group. This is for *OTHER* processes that are
     * waiting for the group to be ready.
     */
    if (created) {
        SAFE_CALL_ZK(m_zk.setNodeData(key, "ready", 0),
                     "Could not complete ready protocol for %s: %s",
                     key.c_str(),
                     true,
                     true);
    }
    
    return np;
}

/*
 * Retrieve bits of Node state.
 */
bool
FactoryOps::isNodeConnected(const string &nodeKey)
{
    TRACE(CL_LOG, "isNodeConnected");

    string ckey =
        nodeKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CONNECTED;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(
                      ckey,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNodeConnectionChangeHandler())),
                 "Could not determine whether key %s is connected: %s",
                 nodeKey.c_str(),
                 false,
                 true);
    return exists;
}
string
FactoryOps::getNodeClientState(const string &nodeKey)
{
    TRACE(CL_LOG, "getNodeClientState");

    string ckey =
        nodeKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CLIENTSTATE;
    string res = "";

    SAFE_CALL_ZK((res = m_zk.getNodeData(
                      ckey,
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNodeClientStateChangeHandler())),
                 "Could not read node %s client state: %s",
                 nodeKey.c_str(),
                 false,
                 true);
    return res;
}
int32_t
FactoryOps::getNodeMasterSetState(const string &nodeKey)
{
    TRACE(CL_LOG, "getNodeMasterSetState");

    string ckey =
        nodeKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::MASTERSETSTATE;
    string res = "";

    SAFE_CALL_ZK(
        (res = m_zk.getNodeData(
            ckey,
            &m_zkEventAdapter,
            getChangeHandlers()->getNodeMasterSetStateChangeHandler())),
        "Could not read node %s master set state: %s",
        nodeKey.c_str(),
        false,
        true);
    return ::atoi(res.c_str());
}

/*
 * Get all entity names within a collection: all applications,
 * all groups or distributions within an application, or all
 * nodes within a group.
 */
NameList
FactoryOps::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    NameList list;
    string key =
        ClusterlibStrings::ROOTNODE +
        ClusterlibStrings::CLUSTERLIB +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CLUSTERLIBVERSION +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::ROOT +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::APPLICATIONS;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(
                     list,
                     key, 
                     &m_zkEventAdapter,
                     getChangeHandlers()->getApplicationsChangeHandler()),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);

    for (NameList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + 
                           ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getGroupNames(GroupImpl *group)
{
    TRACE(CL_LOG, "getGroupNames");

    if (group == NULL) {
        throw ClusterException("NULL group in getGroupNames");
    }

    NameList list;
    string key =
        group->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::GROUPS;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(
                     list,
                     key,
                     &m_zkEventAdapter,
                     getChangeHandlers()->getGroupsChangeHandler()),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);

    for (NameList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + 
                           ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getDataDistributionNames(GroupImpl *group)
{
    TRACE(CL_LOG, "getDistributionNames");

    if (group == NULL) {
        throw ClusterException("NULL group in getDataDistributionNames");
    }

    NameList list;
    string key=
        group->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::DISTRIBUTIONS;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(
                     list,
                     key,
                     &m_zkEventAdapter,
                     getChangeHandlers()->getDataDistributionsChangeHandler()),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);

    for (NameList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + 
                           ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getNodeNames(GroupImpl *group)
{
    TRACE(CL_LOG, "getNodeNames");

    if (group == NULL) {
        throw ClusterException("NULL group in getNodeNames");
    }

    NameList list;
    string key =
        group->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::NODES;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(
                     list,
                     key,
                     &m_zkEventAdapter,
                     getChangeHandlers()->getNodesChangeHandler()),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);

    for (NameList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + 
                           ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}

/**********************************************************************/
/* Leadership protocol.                                               */
/**********************************************************************/

/*
 * Get the node for the current leader of a given group.
 */
NodeImpl *
FactoryOps::getLeader(GroupImpl *group)
{
    TRACE( CL_LOG, "getLeader" );

    if (group == NULL) {
        throw ClusterException("getLeader: NULL group");
    }

    NodeImpl *lp = NULL;
    string lnn = group->getCurrentLeaderNodeName();
    string ln = "";

    SAFE_CALL_ZK((ln = m_zk.getNodeData(
                      lnn, 
                      &m_zkEventAdapter,
                      getChangeHandlers()->getLeadershipChangeHandler())),
                 "Getting current leader of group %s failed: %s",
                 group->getKey().c_str(),
                 true,
                 true);
    if (ln != "") {
        lp = getNode(ln, group, true, false);
    }
    return lp;
}

int64_t
FactoryOps::placeBid(NodeImpl *np, ServerImpl *sp)
{
    TRACE(CL_LOG, "placeBid");

    if (np == NULL) {
        throw ClusterException("placeBid: NULL node");
    }
    if (sp == NULL) {
        throw ClusterException("placeBid: NULL server");
    }

    char tmp[100];

    snprintf(tmp, 100, "%d", getpid());

    GroupImpl *group = dynamic_cast<GroupImpl *>(np->getMyGroup());

    if (group == NULL) {
        throw ClusterException(string("placeBid: NULL group containing node ")
                               + np->getKey());
    }

    string pfx = group->getLeadershipBidPrefix();
    string value =
        np->getKey() +
        ";" +
        VERSION +
        ";" +
        tmp;
    int64_t bid = 0;

    string createdPath;
    SAFE_CALL_ZK((bid = m_zk.createSequence(pfx, 
                                            value, 
                                            ZOO_EPHEMERAL, 
                                            false, 
                                            createdPath)),
                 "Bidding with prefix %s to become leader failed: %s",
                 pfx.c_str(),
                 true,
                 true);

    Locker l1(getLeadershipWatchesLock());
    m_leadershipWatches.insert(pair<const string, ServerImpl *>(np->getKey(),
                                                                sp));

    return bid;
}

/*
 * Make the given node a leader of its group if the given
 * bid is the lowest (honors system!).
 */
bool
FactoryOps::tryToBecomeLeader(NodeImpl *np, int64_t bid)
{
    TRACE(CL_LOG, "tryToBecomeLeader");

    if (np == NULL) {
        throw ClusterException("tryToBecomeLeader: NULL node");
    }

    NameList list;
    NameList::iterator iIt;
    GroupImpl *group = dynamic_cast<GroupImpl *>(np->getMyGroup());

    if (group == NULL) {
        throw ClusterException(
		string("tryToBecomeLeader: NULL group containing ") +
                np->getKey());
    }

    string lnn = group->getCurrentLeaderNodeName();
    string bnn = group->getLeadershipBidsNodeName();
    string pfx = group->getLeadershipBidPrefix();
    string ln = "";
    string val = "";
    string suffix = "";
    string toCheck = "";
    int32_t len = pfx.length();
    const char *cppfx = pfx.c_str();
    char *ptr;
    int64_t checkID;
    bool exists = false;

    /*
     * If there's already a different leader, then I'm not the leader.
     */

    SAFE_CALL_ZK((m_zk.getNodeChildren(list, bnn)),
                 "Getting bids for group %s failed: %s",
                 group->getKey().c_str(),
                 true,
                 true);
    for (iIt = list.begin(); iIt != list.end(); iIt++) {
        toCheck = *iIt;

        /*
         * Skip any random strings that are not
         * sequence members.
         */
        if (toCheck.compare(0, len, cppfx, len) != 0) {
            continue;
        }

        /*
         * Ensure that this is a legal sequence number.
         */
        suffix = toCheck.substr(len, toCheck.length() - len);
        ptr = NULL;
        checkID = strtol(suffix.c_str(), &ptr, 10);
        if ((ptr != NULL) && (*ptr != '\0')) {
            LOG_WARN(CL_LOG, "Expecting a number but got %s", suffix.c_str());
            throw ClusterException( "Expecting a number but got " + suffix );
        }

        /*
         * Compare to my bid -- if smaller, then a preceding leader exists.
         */
        if (checkID < bid) {
            SAFE_CALL_ZK(
                (exists = m_zk.nodeExists(
                    toCheck,
                    &m_zkEventAdapter,
                    getChangeHandlers()->getPrecLeaderExistsHandler())),
                "Checking for preceding leader %s failed: %s",
                toCheck.c_str(),
                false,
                true);
            if (exists) {
                /*
                 * This is informational so not fatal if it fails.
                 */
                SAFE_CALL_ZK((val = m_zk.getNodeData(toCheck)),
                             "Getting name of preceding leader in %s "
                             "failed: %s",
                             toCheck.c_str(),
                             true,
                             true);
                LOG_INFO(CL_LOG,
                         "Found preceding leader %s value %s, "
                         "%s is not the leader",
                         toCheck.c_str(), 
                         val.c_str(),
                         np->getKey().c_str() );

                /*
                 * This node did not become the leader.
                 */
                return false;
            }
        }
    }

    LOG_WARN(CL_LOG,
             "Found no preceding leader, %s is the leader!",
             np->getKey().c_str());

    return true;
}

/*
 * Is the leader known within the group containing this node.
 */
bool
FactoryOps::isLeaderKnown(NodeImpl *np)
{
    TRACE(CL_LOG, "isLeaderKnown");

    if (np == NULL) {
        return false;
    }

    GroupImpl *group = dynamic_cast<GroupImpl *>(np->getMyGroup());
    if (group == NULL) {
        throw ClusterException(string("NULL group containing ") +
                               np->getKey());
    }

    return group->isLeaderKnown();
}

/*
 * Denote that the leader of the group in which this node
 * is a member is unknown.
 */
void
FactoryOps::leaderIsUnknown(NodeImpl *np)
{
    TRACE(CL_LOG, "leaderIsUnknown");

    if (np == NULL) {
        return;
    }

    GroupImpl *group = dynamic_cast<GroupImpl *>(np->getMyGroup());
    if (group == NULL) {
        throw ClusterException(string("NULL group containing ") +
                               np->getKey());
    }

    group->updateLeader(NULL);
}

/*
 * The server represented by the given node, and that owns the
 * given bid, is no longer the leader of its group (if it ever
 * was).
 */
void
FactoryOps::giveUpLeadership(NodeImpl *np, int64_t bid)
{
    TRACE(CL_LOG, "giveUpLeadership");

    if (np == NULL) {
        return;
    }

    GroupImpl *group = dynamic_cast<GroupImpl *>(np->getMyGroup());
    if (group == NULL) {
        throw ClusterException(string("NULL group containing ") +
                               np->getKey());
    }

    /*
     * Ensure that our view of the leader is not updated by
     * any other thread.
     */
    {
        Locker l1(group->getLeadershipLock());
        if (group->getLeader() == np) {
            /*
             * Delete the current leader node.
             */
            SAFE_CALL_ZK(m_zk.deleteNode(group->getCurrentLeaderNodeName()),
                         "Could not delete current leader for %s: %s",
                         group->getKey().c_str(),
                         true,
                         true);

            /*
             * We no longer know who the leader of this group is.
             */
            group->updateLeader(NULL);
        }
    }

    /*
     * Delete the leadership bid for this node.
     */
    char buf[100];

    snprintf(buf, 100, "%lld", bid);
    string sbid = group->getLeadershipBidPrefix() + buf;

    SAFE_CALL_ZK(m_zk.deleteNode(sbid),
                 "Could not delete bid for current leader %s: %s",
                 np->getKey().c_str(),
                 false,
                 true);
}

/*
 * Methods to prepare strings for leadership protocol.
 */
string
FactoryOps::getCurrentLeaderNodeName(const string &gkey)
{
    return
        gkey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::LEADERSHIP +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CURRENTLEADER;
}
string
FactoryOps::getLeadershipBidsNodeName(const string &gkey)
{
    return
        gkey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::LEADERSHIP +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::BIDS;
}
string
FactoryOps::getLeadershipBidPrefix(const string &gkey)
{
    return
        getLeadershipBidsNodeName(gkey) +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::BIDPREFIX;
}

/*
 * Register a timer handler.
 */
TimerId
FactoryOps::registerTimer(TimerEventHandler *handler,
                       uint64_t afterTime,
                       ClientData data)
{
    Locker l(getTimersLock());
    TimerEventPayload *tepp =
        new TimerEventPayload(afterTime, handler, data);
    TimerId id = m_timerEventSrc.scheduleAfter(afterTime, tepp);

    tepp->updateTimerId(id);
    m_timerRegistry[id] = tepp;

    return id;
}

/*
 * Cancel a timer.
 */
bool
FactoryOps::cancelTimer(TimerId id)
{
    TRACE(CL_LOG, "cancelTimer");

    Locker l(getTimersLock());
    TimerRegistry::const_iterator timerIt = m_timerRegistry.find(id);

    if (timerIt == m_timerRegistry.end()) {
        return false;
    }

    timerIt->second->cancel();
    if (m_timerEventSrc.cancelAlarm(id)) {
        return true;
    }
    return false;
}

/*
 * Update the cached representation of a clusterlib repository object and
 * generate the prototypical cluster event payload to send to registered
 * clients.
 */
ClusterEventPayload *
FactoryOps::updateCachedObject(CachedObjectEventHandler *fehp,
                               zk::ZKWatcherEvent *ep)
{
    TRACE(CL_LOG, "updateCachedObject");

    if (fehp == NULL) {
        throw ClusterException("NULL CachedObjectEventHandler!");
    }
    if (ep == NULL) {
        throw ClusterException("NULL watcher event!");
    }

    const string path = ep->getPath();
    int32_t etype = ep->getType();


    LOG_INFO(CL_LOG,
              "updateCachedObject: (0x%x, 0x%x, %s)",
              (int) fehp,
              (int) ep,
	      path.c_str());

    NotifyableImpl *ntp;

    /*
     * SYNC doesn't get a Notifyable.
     */
    if (path.compare(ClusterlibStrings::SYNC) == 0) {
        ntp = NULL;
    }
    else {
        ntp = getNotifyableFromKey(path); 
        if (ntp == NULL) {
            throw ClusterException(string("Unknown event : ") +
                                   path);
        }
    }

    /*
     * Invoke the object handler. It will update the cache. It
     * will also return the kind of user-level event that this
     * repository event represents.
     */
    Event e = fehp->deliver(ntp, etype, path);

    if (e == EN_NOEVENT) {
        return NULL;
    }
    return new ClusterEventPayload(ntp, e);
}

/*
 * Re-establish interest in the existence of a notifyable by its key.
 */
bool
FactoryOps::establishNotifyableInterest(
    const string &key,
    CachedObjectEventHandler *eventHandlerP)
{
    if (key.empty()) {
        return false;
    }

    bool exists = false;
    SAFE_CALL_ZK(exists = m_zk.nodeExists(key,
                                          &m_zkEventAdapter,
                                          eventHandlerP),
                 "Could not reestablish exists watch on %s: %s",
                 key.c_str(),
                 false,
                 true);

    return exists;
}

string
FactoryOps::getNotifyableValue(const string &key,
                               CachedObjectEventHandler *eventHandlerP)
{
    if (key.empty()) {
        return false;
    }

    string lname;
    SAFE_CALL_ZK((lname = m_zk.getNodeData(key,
                                           &m_zkEventAdapter,
                                           eventHandlerP)),
                 "Could not read current leader for group %s: %s",
                 key.c_str(),
                 false,
                 true);

    return lname;
}
                               
/*
 * Establish whether the given notifyable object
 * is 'ready' according to the 'ready' protocol.
 */
bool
FactoryOps::establishNotifyableReady(NotifyableImpl *ntp)
{
    string ready = "";

    if (ntp == NULL) {
        return false;
    }

    SAFE_CALL_ZK((ready = m_zk.getNodeData(
                      ntp->getKey(),
                      &m_zkEventAdapter,
                      getChangeHandlers()->getNotifyableReadyHandler())),
                 "Reading the value of %s failed: %s",
                 ntp->getKey().c_str(),
                 true,
                 true);

    if (ready == "ready") {
        ntp->setState(Notifyable::READY);
        ntp->initializeCachedRepresentation();
    }

    return (ntp->getState() == Notifyable::READY);
}

};	/* End of 'namespace clusterlib' */
