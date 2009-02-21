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

#include "clusterlib.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

/*
 * Macro for safely executing calls to ZooKeeper.
 *
 * NOTE: If your call needs to return a value, please use
 * SAFE_CALL_ZK((var = call()), "some message %s", false, true)
 */
#define SAFE_CALL_ZK(_action, _message, _node, _warning, _once) \
{ \
    bool done = false; \
    while (!done) { \
	try { \
	    _action ; done = true; \
	} catch (zk::ZooKeeperException &ze) { \
	    if (!ze.isConnected()) { \
		reestablishConnectionAndState(ze.what()); \
	    } else if (_warning) { \
		LOG_WARN(CL_LOG, _message, _node, ze.what()); \
		if (_once) { \
		    /* \
		     * Only warn once. \
		     */ \
		    done = true; \
                } \
	    } else { \
		throw ClusterException(ze.what()); \
	    } \
	} \
    } \
}

namespace clusterlib
{

/*
 * All the string constants needed to construct and deconstruct
 * ZK keys.
 */
const string ClusterlibStrings::ROOTNODE = "/";
const string ClusterlibStrings::PATHSEPARATOR = "/";

const string ClusterlibStrings::CLUSTERLIB = "clusterlib";
const string ClusterlibStrings::CLUSTERLIBVERSION = "1.0";

const string ClusterlibStrings::PROPERTIES = "properties";
const string ClusterlibStrings::CONFIGURATION = "configuration";
const string ClusterlibStrings::ALERTS = "alerts";
const string ClusterlibStrings::SYNC = "sync";

const string ClusterlibStrings::APPLICATIONS = "applications";
const string ClusterlibStrings::GROUPS = "groups";
const string ClusterlibStrings::NODES = "nodes";
const string ClusterlibStrings::UNMANAGEDNODES = "unmanagedNodes";
const string ClusterlibStrings::DISTRIBUTIONS = "distributions";

const string ClusterlibStrings::CLIENTSTATE = "clientState";
const string ClusterlibStrings::CLIENTSTATEDESC = "clientStateDesc";
const string ClusterlibStrings::ADDRESS = "address";
const string ClusterlibStrings::LASTCONNECTED = "lastConnected";
const string ClusterlibStrings::CLIENTVERSION = "clientVersion";
const string ClusterlibStrings::CONNECTED = "connected";
const string ClusterlibStrings::BOUNCY = "bouncy";
const string ClusterlibStrings::READY = "ready";
const string ClusterlibStrings::ALIVE = "alive";
const string ClusterlibStrings::MASTERSETSTATE = "masterSetState";
const string ClusterlibStrings::SUPPORTEDVERSIONS = "supportedVersions";

const string ClusterlibStrings::LEADERSHIP = "leadership";
const string ClusterlibStrings::BIDS = "bids";
const string ClusterlibStrings::CURRENTLEADER = "currentLeader";

const string ClusterlibStrings::SHARDS = "shards";
const string ClusterlibStrings::GOLDENSHARDS = "goldenShards";
const string ClusterlibStrings::MANUALOVERRIDES = "manualOverrides";

const string ClusterlibStrings::LOCKS = "locks";
const string ClusterlibStrings::QUEUES = "queues";
const string ClusterlibStrings::BARRIERS = "barriers";
const string ClusterlibStrings::TRANSACTIONS = "transactions";

/*
 * All strings that are used as ZK values or part of values.
 */
const string ClusterlibStrings::BIDPREFIX = "L_";
const string ClusterlibStrings::INFLUX = "influx";
const string ClusterlibStrings::HEALTHY = "healthy";
const string ClusterlibStrings::UNHEALTHY = "unhealthy";

/*
 * All strings that are used as propreties.
 */

const string ClusterlibStrings::HEARTBEATMULTIPLE = "heartBeat.multiple";
const string ClusterlibStrings::HEARTBEATCHECKPERIOD= "heartBeat.checkPeriod";
const string ClusterlibStrings::HEARTBEATHEALTHY = "heartBeat.healthy";
const string ClusterlibStrings::HEARTBEATUNHEALTHY = "heatBeat.unhealthy";
const string ClusterlibStrings::TIMEOUTUNHEALTHYYTOR = "timeOut.unhealthyYToR";
const string ClusterlibStrings::TIMEOUTUNHEALTHYRTOD = "timeOut.unhealthyRToR";
const string ClusterlibStrings::TIMEOUTDISCONNECTYTOR = "timeOut.disconnectYToR";
const string ClusterlibStrings::TIMEOUTDISCONNECTRTOD = "timeOut.disconnectRToR";
const string ClusterlibStrings::NODESTATEGREEN = "node.state.green";
const string ClusterlibStrings::NODEBOUNCYPERIOD = "nodeBouncy.period";
const string ClusterlibStrings::NODEBOUNCYNEVENTS = "nodeBouncy.nEvents";
const string ClusterlibStrings::NODEMOVEBACKPERIOD = "nodeMoveBack.period";
const string ClusterlibStrings::CLUSTERUNMANAGED = "cluster.unmanaged";
const string ClusterlibStrings::CLUSTERDOWN = "cluster.down";
const string ClusterlibStrings::CLUSTERFLUXPERIOD = "cluster.fluxPeriod";
const string ClusterlibStrings::CLUSTERFLUXNEVENTS = "cluster.fluxNEvents";
const string ClusterlibStrings::HISTORYSIZE = "history.size";
const string ClusterlibStrings::LEADERFAILLIMIT = "leader.failLimit";
const string ClusterlibStrings::SERVERBIN = "server.bin";

/*
 * Names associated with the special clusterlib master application.
 */
const string ClusterlibStrings::MASTER = "master";

/* 
 * All indices use for parsing ZK node names
 */
const int ClusterlibInts::CLUSTERLIB_INDEX = 1;
const int ClusterlibInts::VERSION_NAME_INDEX = 2;
const int ClusterlibInts::APP_INDEX = 3;
const int ClusterlibInts::APP_NAME_INDEX = 4;
const int ClusterlibInts::GROUP_INDEX = 5;
const int ClusterlibInts::GROUP_NAME_INDEX = 6;
const int ClusterlibInts::DIST_INDEX = 5;
const int ClusterlibInts::DIST_NAME_INDEX = 6;
const int ClusterlibInts::NODE_TYPE_INDEX = 7;
const int ClusterlibInts::NODE_NAME_INDEX = 8;

/*
 * Constructor of Factory.
 *
 * Connect to cluster via the registry specification.
 * Initialize all the cache data structures.
 * Create the associated FactoryOps delegate object.
 */
Factory::Factory(const string &registry)
    : m_syncId(0),
      m_syncIdCompleted(0),
      m_config(registry, 3000),
      m_zk(m_config, NULL, false),
      m_timerEventAdapter(m_timerEventSrc),
      m_zkEventAdapter(m_zk),
      m_shutdown(false),
      m_connected(false),
      m_notifyableReadyHandler(
	  this,
          &Factory::handleNotifyableReady),
      m_notifyableExistsHandler(
	  this,
          &Factory::handleNotifyableExists),
      m_propertiesChangeHandler(
	  this,
          &Factory::handlePropertiesChange),
      m_applicationsChangeHandler(
          this,
          &Factory::handleApplicationsChange),
      m_groupsChangeHandler(
	  this,
          &Factory::handleGroupsChange),
      m_distributionsChangeHandler(
	  this,
          &Factory::handleDistributionsChange),
      m_shardsChangeHandler(
	  this,
          &Factory::handleShardsChange),
      m_manualOverridesChangeHandler(
	  this,
          &Factory::handleManualOverridesChange),
      m_nodesChangeHandler(
	  this,
          &Factory::handleNodesChange),
      m_nodeClientStateChangeHandler(
	  this,
          &Factory::handleClientStateChange),
      m_nodeMasterSetStateChangeHandler(
	  this,
          &Factory::handleMasterSetStateChange),
      m_leadershipChangeHandler(
	  this,
          &Factory::handleLeadershipChange),
      m_precLeaderExistsHandler(
	  this,
          &Factory::handlePrecLeaderExistsChange),
      m_synchronizeChangeHandler(
	  this,
          &Factory::handleSynchronizeChange)
{
    TRACE(CL_LOG, "Factory");

    /*
     * Clear all the collections (lists, maps)
     * so that they start out empty.
     */
    m_dataDistributions.clear();
    m_applications.clear();
    m_groups.clear();
    m_nodes.clear();
    m_clients.clear();

    /*
     * Clear the multi-map of leadership election watches.
     */
    m_leadershipWatches.clear();

    /*
     * Create the delegate.
     */
    mp_ops = new FactoryOps(this);

    /*
     * Link up the event sources.
     */
    m_timerEventAdapter.addListener(&m_eventAdapter);
    m_zkEventAdapter.addListener(&m_eventAdapter);

    /*
     * Create the event dispatch thread.
     */
    m_eventThread.Create(*this, &Factory::dispatchEvents);

    /*
     * Create the timer handler thread.
     */
    m_timerHandlerThread.Create(*this, &Factory::consumeTimerEvents);

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
};

/*
 * Destructor of Factory
 */
Factory::~Factory()
{
    TRACE(CL_LOG, "~Factory");

    removeAllClients();
    removeAllDataDistributions();
    removeAllProperties();
    removeAllApplications();
    removeAllGroups();
    removeAllNodes();

    delete mp_ops;

    try {
        m_zk.disconnect();
    } catch (zk::ZooKeeperException &e) {
        LOG_WARN(CL_LOG,
                 "Got exception during disconnect: %s",
                 e.what());
    }
};


/*
 * Create a client.
 */
Client *
Factory::createClient()
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
        Client *cp = new Client(mp_ops);
        addClient(cp);
        return cp;
    } catch (ClusterException &e) {
	LOG_WARN(CL_LOG, 
                 "Couldn't create client because: %s", 
                 e.what());
        return NULL;
    }
};

/*
 * Create a server.
 */
Server *
Factory::createServer(const string &app,
                      const string &group,
                      const string &node,
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
        Server *sp = new Server(mp_ops,
                                app,
                                group,
                                node,
                                checker,
                                flags);
        addClient(sp);
        return sp;
    } catch (ClusterException &e) {
	LOG_WARN(CL_LOG, 
                 "Couldn't create server with app %s, "
                 "group %s, node %s because: %s", 
                 app.c_str(), 
                 group.c_str(), 
                 node.c_str(), 
                 e.what());
        return NULL;
    }
};



/*
 * Try to synchronize with the underlying data store.
 */
void
Factory::synchronize()
{
    TRACE(CL_LOG, "synchronize");
    int syncId = 0;

    /* 
     * Simple algorithm to ensure that each synchronize() called by
     * various clients gets a unique ID.  As long as there are not
     * 2^64 sync operations in progress, this will not be a problem.
     */
    {
        AutoLock l1(m_syncLock);
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

    string path(ROOTNODE);
    path.append(CLUSTERLIB);
    SAFE_CALL_ZK(m_zk.sync(path, 
                           &m_zkEventAdapter, 
                           &m_synchronizeChangeHandler),
                 "Could not synchronize with the underlying store %s: %s",
                 "/",
                 true,
                 true);

    /* Wait for notification of the event to be received by *
     * m_eventAdapter. */
    {
        AutoLock l1(m_syncLock);
        while (syncId > m_syncIdCompleted) {
            m_syncCond.Wait(m_syncLock);
        }
    }

    LOG_DEBUG(CL_LOG, "synchronize: Complete");
}


/**********************************************************************/
/* Below this line are the private methods of class Factory.          */
/**********************************************************************/

/*
 * Add and remove clients.
 */
void
Factory::addClient(Client *clp)
{
    TRACE(CL_LOG, "addClient");

    Locker l(&m_clLock);
    
    m_clients.push_back(clp);
}

void
Factory::removeClient(Client *clp)
{
    TRACE(CL_LOG, "removeClient");

    Locker l(&m_clLock);
    ClientList::iterator i = find(m_clients.begin(),
                                  m_clients.end(),
                                  clp);

    if (i == m_clients.end()) {
        return;
    }
    m_clients.erase(i);
}

void
Factory::removeAllClients()
{
    TRACE(CL_LOG, "removeAllClients");

    Locker l(&m_clLock);
    ClientList::iterator it = m_clients.begin();
    for (; it != m_clients.end(); it++) {
	delete *it;
    }
    m_clients.clear();
}

/*
 * Methods to clean up storage used by a Factory.
 */
void
Factory::removeAllDataDistributions()
{
    TRACE(CL_LOG, "removeAllDataDistributions");

    Locker l(&m_ddLock);
    DataDistributionMap::iterator it = m_dataDistributions.begin();
    for (; it != m_dataDistributions.end(); it++) {
	delete it->second;
    }
    m_dataDistributions.clear();
}
void
Factory::removeAllProperties()
{
    TRACE(CL_LOG, "removeAllProperties");

    Locker l(&m_propLock);
    PropertiesMap::iterator it = m_properties.begin();
    for (; it != m_properties.end(); it++) {
	delete it->second;
    }
    m_properties.clear();
}
void
Factory::removeAllApplications()
{
    TRACE(CL_LOG, "removeAllApplications");

    Locker l(&m_appLock);
    ApplicationMap::iterator it = m_applications.begin();
    for (; it != m_applications.end(); it++) {
	delete it->second;
    }
    m_applications.clear();
}
void
Factory::removeAllGroups()
{
    TRACE(CL_LOG, "removeAllGroups");

    Locker l(&m_grpLock);
    GroupMap::iterator it = m_groups.begin();
    for (; it != m_groups.end(); it++) {
	delete it->second;
    }
    m_groups.clear();
}
void
Factory::removeAllNodes()
{
    TRACE(CL_LOG, "removeAllNodes");

    Locker l(&m_nodeLock);
    NodeMap::iterator it = m_nodes.begin();
    for (; it != m_nodes.end(); it++) {
	delete it->second;
    }
    m_nodes.clear();
}

/*
 * Dispatch events to all registered clients.
 */
void
Factory::dispatchEvents()
{
    TRACE(CL_LOG, "dispatchEvents");

    unsigned int eventSeqId = 0;
    bool sentEndEvent = false;
    GenericEvent ge;

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
            ge = m_eventAdapter.getNextEvent();

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
                              "(type: %d, state: %d, context: 0x%x)",
                              zp->getType(),
                              zp->getState(),
                              (unsigned int) zp->getContext());
                    
                    if ((zp->getType() == SESSION_EVENT) &&
                        (zp->getPath().compare(SYNC) != 0)) {
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
        sentEndEvent = dispatchEndEvent();
    } catch (zk::ZooKeeperException &zke) {
        LOG_ERROR(CL_LOG, "ZooKeeperException: %s", zke.what());
        if (!sentEndEvent) {
            dispatchEndEvent();
        }
        throw ClusterException(zke.what());
    } catch (ClusterException &ce) {
        if (!sentEndEvent) {
            dispatchEndEvent();
        }
        throw ClusterException(ce.what());
    } catch (std::exception &stde) {
        LOG_ERROR(CL_LOG, "Unknown exception: %s", stde.what());
        if (!sentEndEvent) {
            dispatchEndEvent();
        }
        throw ClusterException(stde.what());
    }
}

/*
 * Dispatch a timer event.
 */
void
Factory::dispatchTimerEvent(ClusterlibTimerEvent *te)
{
    TRACE(CL_LOG, "dispatchTimerEvent");

    if (te == NULL) {
        m_timerEventQueue.put(NULL);
    } else {
        TimerEventPayload *tp = (TimerEventPayload *) te->getUserData();
        m_timerEventQueue.put(tp);
    }
}

/*
 * Dispatch a ZK event.
 */
void
Factory::dispatchZKEvent(zk::ZKWatcherEvent *zp)
{
    TRACE(CL_LOG, "dispatchZKEvent");
    
    if (!zp) {
	throw ClusterException("Unexpected NULL ZKWatcherEvent");
    }

    FactoryEventHandler *cp =
        (FactoryEventHandler *) zp->getContext();
    ClusterEventPayload *cep, *cepp;
    ClientList::iterator i;
    char buf[1024];

    /*
     * Protect against NULL context.
     */
    if (cp == NULL) {
        snprintf(buf,
                 1024,
                 "type: %d, state: %d, path: %s",
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
    cep = updateCachedObject(cp, zp);
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
        Locker l(&m_clLock);

        for (i = m_clients.begin(); i != m_clients.end(); i++) {
            cepp = new ClusterEventPayload(*cep);
            (*i)->sendEvent(cepp);
        }
    }
    delete cep;
}

/*
 * Dispatch a session event. These events
 * are in fact handled directly, here.
 */
void
Factory::dispatchSessionEvent(zk::ZKWatcherEvent *ze)
{
    TRACE(CL_LOG, "dispatchSessionEvent");

    LOG_DEBUG(CL_LOG,
              "dispatchSessionEvent: (type: %d, state: %d, path: %s)",
              ze->getType(), 
              ze->getState(),
              ze->getPath().c_str());

    if ((ze->getState() == ASSOCIATING_STATE) ||
        (ze->getState() == CONNECTING_STATE)) {
        /*
         * Not really clear what to do here.
         * For now do nothing.
         */
#ifdef	VERY_VERY_VERBOSE
        LOG_TRACE(CL_LOG, "Do nothing.");
#endif
    } else if (ze->getState() == CONNECTED_STATE) {
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
    } else if (ze->getState() == EXPIRED_SESSION_STATE) {
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
    } else {
        LOG_WARN(CL_LOG,
                 "Session event with unknown state "
                 "(type: %d, state: %d)",
                 ze->getType(), 
                 ze->getState());
    }
}

/*
 * Dispatch a final event to all registered clients
 * to indicate that no more events will be sent
 * following this one.
 */
bool
Factory::dispatchEndEvent()
{
    TRACE(CL_LOG, "dispatchEndEvent");

    ClientList::iterator i;

    /*
     * Send a terminate signal to the timer
     * event handler thread.
     */
    dispatchTimerEvent(NULL);

    /*
     * Send a terminate signal to all registered
     * client-specific cluster event handler threads.
     */
    for (i = m_clients.begin(); i != m_clients.end(); i++) {
        (*i)->sendEvent(NULL);
    }

    return true;
}

/*
 * Consume timer events. Run the timer event handlers.
 * This runs in a special thread owned by the factory.
 */
void
Factory::consumeTimerEvents()
{
    TRACE(CL_LOG, "consumeTimerEvents");

    TimerEventPayload *pp;

#ifdef	VERY_VERY_VERBOSE
    LOG_WARN(CL_LOG,
             "Hello from consumeTimerEvents, this: 0x%x, thread: %d",
             this,
             (uint32_t) pthread_self());
#endif

    try {
        for (;;) {
            pp = m_timerEventQueue.take();

            /*
             * If we received the terminate signal,
             * then exit from the loop.
             */
            if (pp == NULL) {
                LOG_INFO(CL_LOG,
                         "Received terminate signal, finishing loop");
                return;
            }

            /*
             * Dispatch the event to its handler, if the
             * event hadn't been cancelled.
             */
            if (!pp->cancelled()) {
                pp->getHandler()->handleTimerEvent(pp->getId(),
                                                   pp->getData());
            }

            LOG_INFO(CL_LOG,
                     "Serviced timer %d, handler 0x%x, client data 0x%x",
                     pp->getId(), 
                     (int) pp->getHandler(),
                     (int) pp->getData());

            /*
             * Deallocate the payload object.
             */
            delete pp;
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
 * Factory's cache.
 */
Application *
Factory::getApplication(const string &name, bool create)
{
    string key = createAppKey(name);
    Application *app;

    {
        Locker l(&m_appLock);
        app = m_applications[key];
        if (app != NULL) {
            return app;
        }
    }
    app = loadApplication(name, key);
    if (app != NULL) {
        return app;
    }
    if (create == true) {
        return createApplication(name, key);
    }
    return NULL;
}
DataDistribution *
Factory::getDistribution(const string &distName,
                         Application *app,
                         bool create)
{
    if (app == NULL) {
        return NULL;
    }
    string key = createDistKey(app->getName(), distName);

    {
        Locker l(&m_ddLock);
        DataDistribution *dist = m_dataDistributions[key];
        if (dist != NULL) {
            return dist;
        }
    }
    DataDistribution *dist = loadDistribution(distName, key, app);
    if (dist != NULL) {
        return dist;
    }
    if (create == true) {
        return createDistribution(distName, key, "", "", app);
    }
    return NULL;
}
DataDistribution *
Factory::getDistribution(const string &distName,
                         const string &appName,
                         bool create)
{
    return getDistribution(distName,
                           getApplication(appName, false),
                           create);
}
Properties *
Factory::getProperties(const string &key,
		       bool create)
{
    if (key.empty()) {
        return NULL;
    }

    {
        Locker l(&m_propLock);
        Properties *prop = m_properties[key];
        if (prop != NULL) {
            return prop;
        }
    }
    Properties *prop = loadProperties(key);
    if (prop != NULL) {
        return prop;
    }
    if (create == true) {
        return createProperties(key);
    }
    return NULL;
}
Group *
Factory::getGroup(const string &groupName,
                  Application *app,
                  bool create)
{
    if (app == NULL) {
        return NULL;
    }
    string key = createGroupKey(app->getName(), groupName);
    Group *grp;

    {
        Locker l(&m_grpLock);
        grp = m_groups[key];
        if (grp != NULL) {
            return grp;
        }
    }
    grp = loadGroup(groupName, key, app);
    if (grp != NULL) {
        return grp;
    }
    if (create == true) {
        return createGroup(groupName, key, app);
    }
    return NULL;
}
Group *
Factory::getGroup(const string &appName,
                  const string &groupName,
                  bool create)
{
    return getGroup(groupName,
                    getApplication(appName, create),
                    create);
}
Node *
Factory::getNode(const string &nodeName,
                 Group *grp,
                 bool managed,
                 bool create)
{
    if (grp == NULL) {
        return NULL;
    }
    Application *app = grp->getApplication();
    if (app == NULL) {
        return NULL;
    }
    string key = createNodeKey(app->getName(),
                               grp->getName(),
                               nodeName,
                               managed);
    Node *np;

    {
        Locker l(&m_nodeLock);
        np = m_nodes[key];
        if (np != NULL) {
            return np;
        }
    }
    np = loadNode(nodeName, key, grp);
    if (np != NULL) {
        return np;
    }
    if (create == true) {
        return createNode(nodeName, key, grp);
    }
    return NULL;
}
Node *
Factory::getNode(const string &appName,
                 const string &groupName,
                 const string &nodeName,
                 bool managed,
                 bool create)
{
    return getNode(nodeName,
                   getGroup(appName, groupName, false),
                   managed,
                   create);
}

/*
 * Update the fields of a distribution in the clusterlib repository.
 */
void
Factory::updateDistribution(const string &key,
                            const string &shards,
                            const string &manualOverrides,
                            int32_t shardsVersion,
			    int32_t manualOverridesVersion)
{
    TRACE(CL_LOG, "updateDistribution");
    
    string snode = 
	key +
        PATHSEPARATOR +
	SHARDS;
    string monode =
        key +
        PATHSEPARATOR +
        MANUALOVERRIDES;
    bool exists = false;

    /*
     * Update the shards.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(snode)),
                 "Could not determine whether node %s exists: %s",
                 snode.c_str(),
                 false,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(snode,shards,0,true),
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
    SAFE_CALL_ZK(m_zk.getNodeData(snode, &m_zkEventAdapter, 
                                  &m_shardsChangeHandler),
                 "Reestablishing watch on value of %s failed: %s",
                 snode.c_str(),
                 true,
                 true);

    /*
     * Update the manual overrides.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(monode)),
                 "Could not determine whether node %s exists: %s",
                 monode.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(monode,
                                     manualOverrides,
                                     0,
                                     true),
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
    SAFE_CALL_ZK(m_zk.getNodeData(monode,
                                  &m_zkEventAdapter,
                                  &m_manualOverridesChangeHandler),
                 "Reestablishing watch on value of %s failed: %s",
                 monode.c_str(),
                 false,
                 true);
}

/*
 * Update the properties of a notifyable object in the clusterlib repository
 */
void
Factory::updateProperties(const string &key,
			  const string &properties,
			  int32_t versionNumber)
{
    TRACE(CL_LOG, "updateProperties");
    
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(key, properties, 0, true),
                     "Creation of %s failed: %s",
                     key.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(key, properties, versionNumber),
                 "Setting of %s failed: %s",
                 key.c_str(),
                 false,
                 true);
    SAFE_CALL_ZK(m_zk.getNodeData(key,
                                  &m_zkEventAdapter,
                                  &m_propertiesChangeHandler),
                 "Reestablishing watch on value of %s failed: %s",
                 key.c_str(),
                 false,
                 true);
}

/*
 * Update the client state field of a node.
 */
void
Factory::updateNodeClientState(const string &key,
                               const string &cs)
{
    TRACE(CL_LOG, "updateNodeClientState");

    string csKey = key + PATHSEPARATOR + CLIENTSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(csKey, cs, 0, true),
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
    SAFE_CALL_ZK(m_zk.getNodeData(csKey,
                                  &m_zkEventAdapter,
                                  &m_nodeClientStateChangeHandler),
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
Factory::updateNodeClientStateDesc(const string &key,
                                   const string &desc)
{
    TRACE(CL_LOG, "updateNodeClientStateDesc");

    string csKey = key + PATHSEPARATOR + CLIENTSTATEDESC;
    bool exists = false;


    SAFE_CALL_ZK((exists = m_zk.nodeExists(csKey)),
                 "Could not determine whether node %s exists: %s",
                 csKey.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(csKey, desc, 0, true),
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
Factory::updateNodeMasterSetState(const string &key,
                                  const string &ms)
{
    TRACE(CL_LOG, "updateNodeMasterSetState");

    string msKey = key + PATHSEPARATOR + MASTERSETSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(msKey)),
                 "Could not determine whether node %s exists: %s",
                 msKey.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(msKey, ms, 0, true),
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
                                  &m_nodeMasterSetStateChangeHandler),
                 "Reestablishing watch on value of %s failed: %s",
                 msKey.c_str(),
                 false,
                 true);
}

/*
 * Get (& potentially load) cache objects given a key.
 */
Application *
Factory::getApplicationFromKey(const string &key, bool create)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasAppKeyPrefix(components)) {
        return getApplication(components[APP_NAME_INDEX], create);
    }
    return NULL;
}
DataDistribution *
Factory::getDistributionFromKey(const string &key, bool create)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasDistKeyPrefix(components)) {
        return getDistribution(components[DIST_NAME_INDEX],
                               getApplication(components[APP_NAME_INDEX], 
                                              false),
                               create);
    }
    return NULL;
}
Group *
Factory::getGroupFromKey(const string &key, bool create)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasGroupKeyPrefix(components)) {
        return getGroup(components[GROUP_NAME_INDEX],
                        getApplication(components[APP_NAME_INDEX], false),
                        create);
    }
    return NULL;
}
Node *
Factory::getNodeFromKey(const string &key, bool create)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasNodeKeyPrefix(components)) {
        return getNode(components[NODE_NAME_INDEX],
                       getGroup(components[APP_NAME_INDEX],
                                components[GROUP_NAME_INDEX],
                                false),
                       (components[NODE_TYPE_INDEX] == "nodes") ? true : false,
                       create);
    }
    return NULL;
}


/*
 * Key creation and recognition.
 */
string
Factory::createNodeKey(const string &appName,
                       const string &groupName,
                       const string &nodeName,
                       bool managed)
{
    string res =
        createGroupKey(appName, groupName) +
        PATHSEPARATOR +
        (managed ? NODES : UNMANAGEDNODES) +
        PATHSEPARATOR +
        nodeName
        ;

    return res;
}

string
Factory::createGroupKey(const string &appName,
                        const string &groupName)
{
    string res = 
        createAppKey(appName) +
        PATHSEPARATOR +
        GROUPS +
        PATHSEPARATOR +
        groupName
        ;

    return res;
}

string
Factory::createAppKey(const string &appName)
{
    string res =
        ROOTNODE +
        CLUSTERLIB +
        PATHSEPARATOR +
        CLUSTERLIBVERSION +
        PATHSEPARATOR +
        APPLICATIONS +
        PATHSEPARATOR +
        appName
        ;

    return res;
}

string
Factory::createDistKey(const string &appName,
                       const string &distName)
{
    string res =
        createAppKey(appName) +
        PATHSEPARATOR +
        DISTRIBUTIONS +
        PATHSEPARATOR +
        distName
        ;

    return res;
}

string
Factory::createPropertiesKey(const string &notifyableKey)
{
    string res =
	notifyableKey +
        PATHSEPARATOR +
        PROPERTIES
	;

    return res;
}

bool
Factory::isNodeKey(const string &key, bool *managedP)
{
    return true;
}
bool
Factory::hasNodeKeyPrefix(const string &key, bool *managedP)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    return hasNodeKeyPrefix(components, managedP);
}
bool
Factory::hasNodeKeyPrefix(vector<string> &components, bool *managedP)
{
    if ((components.size() < 8) ||
        (hasGroupKeyPrefix(components) == false) ||
        ((components[NODE_TYPE_INDEX] != NODES) &&
         (components[NODE_TYPE_INDEX] != UNMANAGEDNODES))) {
        return false;
    }
    if (managedP != NULL) {
        *managedP = ((components[NODE_TYPE_INDEX] == NODES) ? true : false);
    }
    return true;
}
string
Factory::getNodeKeyPrefix(const string &key, bool *managedP)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasNodeKeyPrefix(components, managedP) == false) {
        return string("");
    }
    return getNodeKeyPrefix(components);
}
string
Factory::getNodeKeyPrefix(vector<string> &components)
{
    return
        getGroupKeyPrefix(components) +
        PATHSEPARATOR +
        NODES +
        PATHSEPARATOR +
        components[NODE_NAME_INDEX];
}

bool
Factory::hasPropertiesKeyPrefix(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    return hasPropertiesKeyPrefix(components);
}
bool
Factory::hasPropertiesKeyPrefix(vector<string> &components)
{
    if ((components.size() < 5) ||
        (hasAppKeyPrefix(components) == false) ||
        (components.back().compare(PROPERTIES))) {
        return false;
    }
    return true;
}
bool
Factory::isGroupKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 7) ||
        (hasGroupKeyPrefix(components) == false)) {
        return false;
    }
    return true;
}
bool
Factory::hasGroupKeyPrefix(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    return hasGroupKeyPrefix(components);
}
bool
Factory::hasGroupKeyPrefix(vector<string> &components)
{
    if ((components.size() < 7) ||
        (hasAppKeyPrefix(components) == false) ||
        (components[GROUP_NAME_INDEX] != GROUPS)) {
        return false;
    }
    return true;
}
string
Factory::getGroupKeyPrefix(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasGroupKeyPrefix(components) == false) {
        return string("");
    }
    return getGroupKeyPrefix(components);
}
string
Factory::getGroupKeyPrefix(vector<string> &components)
{
    return
        getAppKeyPrefix(components) +
        PATHSEPARATOR +
        GROUPS +
        PATHSEPARATOR +
        components[GROUP_NAME_INDEX];
}

bool
Factory::isAppKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 5) ||
        (hasAppKeyPrefix(components) == false)) {
        return false;
    }

    return true;
}
bool
Factory::hasAppKeyPrefix(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    return hasAppKeyPrefix(components);
}
bool
Factory::hasAppKeyPrefix(vector<string> &components)
{
    if ((components.size() < 4) ||
        (components[CLUSTERLIB_INDEX] != CLUSTERLIB) ||
        (components[VERSION_NAME_INDEX] != CLUSTERLIBVERSION) ||
        (components[APP_INDEX] != APPLICATIONS)) {
        return false;
    }
    return true;
}
string
Factory::getAppKeyPrefix(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasAppKeyPrefix(components) == false) {
        return string("");
    }
    return getAppKeyPrefix(components);
}
string
Factory::getAppKeyPrefix(vector<string> &components)
{
    return
        ROOTNODE +
        CLUSTERLIB +
        PATHSEPARATOR +
        CLUSTERLIBVERSION +
        PATHSEPARATOR +
        APPLICATIONS +
        PATHSEPARATOR +
        components[APP_NAME_INDEX];
}

bool
Factory::isDistKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 7) ||
        (hasDistKeyPrefix(components) == false)) {
        return false;
    }
    return true;
}
bool
Factory::hasDistKeyPrefix(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    return hasDistKeyPrefix(components);
}
bool
Factory::hasDistKeyPrefix(vector<string> &components)
{
    if ((components.size() < 6) ||
        (hasAppKeyPrefix(components) == false) ||
        (components[DIST_INDEX] != DISTRIBUTIONS)) {
        return false;
    }
    return true;
}
string
Factory::getDistKeyPrefix(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasDistKeyPrefix(components) == false) {
        return string("");
    }
    return getDistKeyPrefix(components);
}
string
Factory::getDistKeyPrefix(vector<string> &components)
{
    return
        getAppKeyPrefix(components) +
        PATHSEPARATOR +
        DISTRIBUTIONS +
        PATHSEPARATOR +
        components[DIST_NAME_INDEX];
}

string
Factory::appNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() < 4) ||
        (hasAppKeyPrefix(components) == false)) {
        return "";
    }
    return components[APP_NAME_INDEX];
}
string
Factory::distNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 7) ||
        (hasDistKeyPrefix(components) == false)) {
        return "";
    }
    return components[DIST_NAME_INDEX];
}
string
Factory::groupNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 7) ||
        (hasGroupKeyPrefix(components) == false)) {
        return "";
    }
    return components[GROUP_NAME_INDEX];
}
string
Factory::nodeNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 9) ||
        (hasDistKeyPrefix(components) == false)) {
        return "";
    }
    return components[NODE_NAME_INDEX];
}

/*
 * Remove the name from the key such that it is removed at most once
 * and deepest in the namespace.  Does not remove the CLUSTERLIB,
 * CLUSTERLIBVERSION, APPLICATIONS, PROPERTIES, or the application name.
 */
string 
Factory::removeObjectFromKey(const string &key)
{
    string res;
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    /* Remove the object name and its base name that is not PROPERTIES */
    if (static_cast<int32_t>(components.size()) > 4) {
	components.erase(components.begin() + 
			 static_cast<int32_t>(components.size()) - 2);
	components.erase(components.begin() + 
			 static_cast<int32_t>(components.size()) - 2);
    }
	 
    for (vector<string>::const_iterator it = components.begin();
	 it != components.end(); it++) {
	res.append(*it);
	if (it + 1 != components.end()) {
	    res.append(PATHSEPARATOR);
	}
    }

    LOG_WARN(CL_LOG, 
              "Changed key %s to %s",
              key.c_str(), 
             res.c_str());

    return res;
}

/*
 * Entity loading from ZooKeeper. Also add it to the
 * global cache.
 */
Application *
Factory::loadApplication(const string &name,
                         const string &key)
{
    TRACE(CL_LOG, "loadApplication");

    vector<string> zkNodes;
    Application *app;
    bool exists = false;
    Locker l(&m_appLock);

    app = m_applications[key];
    if (app != NULL) {
        return app;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key, 
                                           &m_zkEventAdapter,
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    app = new Application(name, key, mp_ops);
    m_applications[key] = app;
    app->updateCachedRepresentation();

#ifdef	TO_BE_MOVED
    /*
     * Set up event notifications.
     */
    m_zk.getNodeChildren(zkNodes,
                         key + PATHSEPARATOR + GROUPS,
                         &m_zkEventAdapter,
                         &m_groupsChangeHandler);
    m_zk.getNodeChildren(zkNodes,
                         key + PATHSEPARATOR + DISTRIBUTIONS,
                         &m_zkEventAdapter,
                         &m_distributionsChangeHandler);
    m_zk.getNodeData(key + PATHSEPARATOR + PROPERTIES,
                     &m_zkEventAdapter,
                     &m_propertiesChangeHandler);
#endif
    /*
     * Set up ready protocol.
     */
    establishNotifyableReady(app);

    return app;
}
    
DataDistribution *
Factory::loadDistribution(const string &name,
                          const string &key,
                          Application *app)
{
    TRACE(CL_LOG, "loadDataDistribution");

    DataDistribution *dp;
    bool exists = false;

    /*
     * Ensure that we have a cached object for this data
     * distribution in the cache.
     */
    Locker l(&m_ddLock);

    dp = m_dataDistributions[key];
    if (dp != NULL) {
        return dp;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    dp = new DataDistribution(app, name, key, mp_ops);
    m_dataDistributions[key] = dp;

    /*
     * Set up event notifications and load the data
     * from the repository.
     */
    dp->updateCachedRepresentation();

    /*
     * Set up the 'ready' protocol.
     */
    establishNotifyableReady(dp);

    return dp;
}

string
Factory::loadShards(const string &key, int32_t &version)
{
    Stat stat;
    string snode =
        key +
        PATHSEPARATOR +
        SHARDS;
    string res = "";

    version = 0;
    SAFE_CALL_ZK((res = m_zk.getNodeData(snode,
                                         &m_zkEventAdapter,
                                         &m_shardsChangeHandler,
                                         &stat)),
                 "Loading shards from %s failed: %s",
                 snode.c_str(),
                 false,
                 true);
    version = stat.version;
    return res;
}

string
Factory::loadManualOverrides(const string &key, int32_t &version)
{
    Stat stat;
    string monode =
        key +
        PATHSEPARATOR +
        MANUALOVERRIDES;
    string res = "";

    version = 0;
    SAFE_CALL_ZK((res = m_zk.getNodeData(monode,
                                         &m_zkEventAdapter,
                                         &m_manualOverridesChangeHandler,
                                         &stat)),
                 "Loading manual overrides from %s failed: %s",
                 monode.c_str(),
                 false,
                 true);
    version = stat.version;
    return res;
}

Properties *
Factory::loadProperties(const string &key)
{
    TRACE(CL_LOG, "Properties");

    Properties *prop;
    bool exists = false;
    Locker l(&m_propLock);

    prop = m_properties[key];
    if (prop != NULL) {
        return prop;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        return NULL;
    }
    SAFE_CALL_ZK(m_zk.getNodeData(key,
                                  &m_zkEventAdapter,
                                  &m_propertiesChangeHandler),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 false,
                 true);
    prop = new Properties(key, mp_ops);
    m_properties[key] = prop;

    return prop;
}

string
Factory::loadKeyValMap(const string &key, int32_t &version)
{
    Stat stat;
    string kvnode = "";

    version = 0;
    SAFE_CALL_ZK((kvnode = m_zk.getNodeData(key,
                                            &m_zkEventAdapter,
                                            &m_propertiesChangeHandler,
                                            &stat)),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 false,
                 true);
    if (kvnode != "") {
        version = stat.version;
    }

    return kvnode;
}

Group *
Factory::loadGroup(const string &name,
                   const string &key,
                   Application *app)
{
    TRACE(CL_LOG, "loadGroup");

    Group *grp;
    bool exists = false;
    Locker l(&m_grpLock);

    grp = m_groups[key];
    if (grp != NULL) {
        return grp;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        return NULL;
    }
    grp = new Group(app, name, key, mp_ops);
    m_groups[key] = grp;

    /*
     * Update the cached representation and establish
     * all event notifications.
     */
    grp->updateCachedRepresentation();

#ifdef	TO_BE_MOVED
    /*
     * Set up event notifications.
     */
    m_zk.getNodeChildren(zkNodes,
                         key + PATHSEPARATOR + NODES,
                         &m_groupMembershipChangeHandler);
    m_zk.nodeExists(key +
                    PATHSEPARATOR + 
                    LEADERSHIP +
                    PATHSEPARATOR +
                    CURRENTLEADER,
                    &m_zkEventAdapter,
                    &m_groupLeadershipChangeHandler);
    m_zk.getNodeData(key + PATHSEPARATOR + PROPERTIES,
                     &m_zkEventAdapter,
                     &m_propertiesChangeHandler);
#endif
    /*
     * Set up ready protocol.
     */
    establishNotifyableReady(grp);

    return grp;
}

Node *
Factory::loadNode(const string &name,
                  const string &key,
                  Group *grp)
{
    TRACE(CL_LOG, "loadNode");

    Node *np;
    bool exists = false;
    Locker l(&m_nodeLock);

    np = m_nodes[key];
    if (np != NULL) {
        return np;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        return NULL;
    }
    np = new Node(grp, name, key, mp_ops);
    m_nodes[key] = np;

    /*
     * Update the cached representation and
     * establish all event notifications.
     */
    np->updateCachedRepresentation();

    /*
     * Set up ready protocol.
     */
    establishNotifyableReady(np);

    return np;
}

/*
 * Entity creation in ZooKeeper.
 */
Application *
Factory::createApplication(const string &name, const string &key)
{
    TRACE(CL_LOG, "createApplication");

    vector<string> zkNodes;
    Application *app = NULL;
    bool created = false;
    bool exists = false;
    
    SAFE_CALL_ZK((exists = m_zk.nodeExists(key,
                                           &m_zkEventAdapter,
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        string groups = key + PATHSEPARATOR + GROUPS;
        string dists = key + PATHSEPARATOR + DISTRIBUTIONS;
        string props = key + PATHSEPARATOR + PROPERTIES;

        /*
         * Create the application data structure if needed.
         */
        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create node %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(groups, "", 0, true),
                     "Could not create node %s: %s",
                     groups.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(dists, "", 0, true),
                     "Could not create node %s: %s",
                     dists.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
                     "Could not create node %s: %s",
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

DataDistribution *
Factory::createDistribution(const string &name,
			    const string &key,
                            const string &marshalledShards,
                            const string &marshalledManualOverrides,
                            Application *app)
{
    TRACE(CL_LOG, "createDataDistribution");

    DataDistribution *dp;
    bool created = false;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key,
                                           &m_zkEventAdapter,
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        string shards = key + PATHSEPARATOR + SHARDS;
        string mos = key + PATHSEPARATOR + MANUALOVERRIDES;
        string props = key + PATHSEPARATOR + PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create node %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(shards, "", 0, true),
                     "Could not create node %s: %s",
                     shards.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(mos, "", 0, true),
                     "Could not create node %s: %s",
                     mos.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
                     "Could not create node %s: %s",
                     props.c_str(),
                     true,
                     true);
    }

    /*
     * Load the distribution, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    dp = loadDistribution(name, key, app);

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

    return dp;
}

Properties *
Factory::createProperties(const string &key) 
{
    TRACE(CL_LOG, "createProperties");
    bool exists = false;

    /*
     * Preliminaries: Ensure the node exists and
     * has the correct watchers set up.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(key,
                                           &m_zkEventAdapter,
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create node %s: %s",
                     key.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.getNodeData(key, 
                                  &m_zkEventAdapter,
                                  &m_propertiesChangeHandler),
                 "Could not read value of %s: %s",
                 key.c_str(),
                 false,
                 true);

    /*
     * Enter into cache.
     */
    Properties *prop;
    {
	/*                                                                
	 * Scope the lock to the shortest sequence possible.
	 */
	Locker l(&m_propLock);
	
	prop = m_properties[key];
	if (prop != NULL) {
	    LOG_WARN(CL_LOG,
		     "Tried to create properties that exists!");
	}
        SAFE_CALL_ZK(m_zk.nodeExists(key, &m_zkEventAdapter, &m_notifyableExistsHandler),
                     "Could not determine whether node %s exists: %s",
                     key.c_str(),
                     false,
                     true);
	prop = new Properties(key, mp_ops);
    }
    
    return prop;

}

Group *
Factory::createGroup(const string &name, 
		     const string &key, 
		     Application *app)
{
    TRACE(CL_LOG, "createGroup");

    Group *grp = NULL;
    bool created = false;
    bool exists = false;
    
    SAFE_CALL_ZK((exists = m_zk.nodeExists(key, &m_zkEventAdapter, &m_notifyableExistsHandler)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        string nodes = key + PATHSEPARATOR + NODES;
        string leadership = key + PATHSEPARATOR + LEADERSHIP;
        string bids = leadership + PATHSEPARATOR + BIDS;
        string props = key + PATHSEPARATOR + PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create node %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(nodes, "", 0, true),
                     "Could not create node %s: %s",
                     nodes.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(leadership, "", 0, true),
                     "Could not create node %s: %s",
                     leadership.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(bids, "", 0, true),
                     "Could not create node %s: %s",
                     bids.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
                     "Could not create node %s: %s",
                     props.c_str(),
                     true,
                     true);
    }

    /*
     * Load the group, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    grp = loadGroup(name, key, app);
    
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
    
    return grp;
}

Node *
Factory::createNode(const string &name,
		    const string &key, 
		    Group *grp)
{
    TRACE(CL_LOG, "createNode");

    Node *np = NULL;
    bool created = false;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key, &m_zkEventAdapter, 
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether node %s exists: %s",
                 key.c_str(),
                 true,
                 true);
    if (!exists) {
        string cs = key + PATHSEPARATOR + CLIENTSTATE;
        string ms = key + PATHSEPARATOR + MASTERSETSTATE;
        string cv = key + PATHSEPARATOR + CLIENTVERSION;
        string props = key + PATHSEPARATOR + PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create node %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(cs, "", 0, true),
                     "Could not create node %s: %s",
                     cs.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(ms, "", 0, true),
                     "Could not create node %s: %s",
                     ms.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(cv, "1.0", 0, true),
                     "Could not create node %s: %s",
                     cv.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
                     "Could not create node %s: %s",
                     props.c_str(),
                     true,
                     true);
    }

    /*
     * Load the node, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    np = loadNode(name, key, grp);

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
 * Get all entity names within a collection: all applications,
 * all groups or distributions within an application, or all
 * nodes within a group.
 */
IdList
Factory::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    IdList list;
    string key =
        ROOTNODE +
        CLUSTERLIB +
        PATHSEPARATOR +
        CLUSTERLIBVERSION;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(list,
                                      key, 
                                      &m_zkEventAdapter,
                                      &m_applicationsChangeHandler),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);
    return list;
}
IdList
Factory::getGroupNames(Application *app)
{
    TRACE(CL_LOG, "getGroupNames");

    IdList list;
    string key =
        app->getKey() +
        PATHSEPARATOR +
        GROUPS;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(list,
                                      key,
                                      &m_zkEventAdapter,
                                      &m_groupsChangeHandler),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);
    return list;
}
IdList
Factory::getDistributionNames(Application *app)
{
    TRACE(CL_LOG, "getDistributionNames");

    IdList list;
    string key=
        app->getKey() +
        PATHSEPARATOR +
        DISTRIBUTIONS;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(list,
                                      key,
                                      &m_zkEventAdapter,
                                      &m_distributionsChangeHandler),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);
    return list;
}
IdList
Factory::getNodeNames(Group *grp)
{
    TRACE(CL_LOG, "getNodeNames");

    IdList list;
    string key =
        grp->getKey() +
        PATHSEPARATOR +
        NODES;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(list,
                                      key,
                                      &m_zkEventAdapter,
                                      &m_nodesChangeHandler),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);
    return list;
}

/**********************************************************************/
/* Leadership protocol.                                               */
/**********************************************************************/

/*
 * Get the node for the current leader of a given group.
 */
Node *
Factory::getLeader(Group *grp)
{
    TRACE( CL_LOG, "getLeader" );

    Node *lp = NULL;
    string lnn = grp->getCurrentLeaderNodeName();
    string ln = "";

    SAFE_CALL_ZK((ln = m_zk.getNodeData(lnn, 
                                        &m_zkEventAdapter,
                                        &m_leadershipChangeHandler)),
                 "Getting current leader of group %s failed: %s",
                 grp->getKey().c_str(),
                 true,
                 true);
    if (ln != "") {
        lp = getNode(ln, grp, true, false);
    }
    return lp;
}

int64_t
Factory::placeBid(Node *np)
{
    TRACE(CL_LOG, "placeBid");

    char tmp[100];

    snprintf(tmp, 100, "%d", getpid());

    Group *grp = np->getGroup();
    string pfx = grp->getLeadershipBidPrefix();
    string value =
        np->getKey() +
        ";" +
        VERSION +
        ";" +
        tmp;
    int64_t bid = 0;

    SAFE_CALL_ZK((bid = m_zk.createSequence(pfx, value, EPHEMERAL, true)),
                 "Bidding with prefix %s to become leader failed: %s",
                 pfx.c_str(),
                 true,
                 true);
    return bid;
}

/*
 * Make the given node a leader of its group if the given
 * bid is the lowest (honors system!).
 */
bool
Factory::tryToBecomeLeader(Node *np, int64_t bid)
{
    TRACE(CL_LOG, "tryToBecomeLeader");

    IdList l;
    Group *grp = np->getGroup();
    string lnn = grp->getCurrentLeaderNodeName();
    string bnn = grp->getLeadershipBidsNodeName();
    string pfx = grp->getLeadershipBidPrefix();
    string ln = "";
    string val = "";
    string suffix = "";
    string toCheck = "";
    int len = pfx.length();
    const char *cppfx = pfx.c_str();
    char *ptr;
    int64_t checkID;
    bool exists = false;

    /*
     * If there's already a different leader, then I'm not the leader.
     */

    SAFE_CALL_ZK((m_zk.getNodeChildren(l, bnn)),
                 "Getting bids for group %s failed: %s",
                 grp->getKey().c_str(),
                 true,
                 true);
    for (IdList::iterator i = l.begin();
         i != l.end();
         i++) {
        toCheck = *i;

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
        if (ptr != NULL && *ptr != '\0') {
            LOG_WARN(CL_LOG, "Expecting a number but got %s", suffix.c_str());
            throw ClusterException( "Expecting a number but got " + suffix );
        }

        /*
         * Compare to my bid -- if smaller, then a preceding leader exists.
         */
        if (checkID < bid) {
            SAFE_CALL_ZK((exists = m_zk.nodeExists(toCheck,
                                                   &m_zkEventAdapter,
                                                   &m_precLeaderExistsHandler)),
                         "Checking for preceding leader %s failed: %s",
                         toCheck.c_str(),
                         false,
                         true);
            if (exists) {
                /*
                 * This is informational so not fatal if it fails.
                 */
                SAFE_CALL_ZK((val = m_zk.getNodeData(toCheck)),
                             "Getting name of preceding leader in %s failed: %s",
                             toCheck.c_str(),
                             true,
                             true);
                LOG_WARN(CL_LOG,
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
             np->getKey().c_str() );

    return true;
}

/*
 * Is the leader known within the group containing this node.
 */
bool
Factory::isLeaderKnown(Node *np)
{
    TRACE(CL_LOG, "isLeaderKnown");

    return np->getGroup()->isLeaderKnown();
}

/*
 * Denote that the leader of the group in which this node
 * is a member is unknown.
 */
void
Factory::leaderIsUnknown(Node *np)
{
    TRACE(CL_LOG, "leaderIsUnknown");

    np->getGroup()->updateLeader(NULL);
}

/*
 * The server represented by the given node, and that owns the
 * given bid, is no longer the leader of its group.
 */
void
Factory::giveUpLeadership(Node *np, int64_t bid)
{
    TRACE(CL_LOG, "giveUpLeadership");

    Group *grp = np->getGroup();

    /*
     * Delete the current leader node.
     */
    SAFE_CALL_ZK(m_zk.deleteNode(grp->getCurrentLeaderNodeName()),
                 "Could not delete current leader for %s: %s",
                 grp->getKey().c_str(),
                 true,
                 true);

    /*
     * Delete the leadership bid for this node.
     */
    char buf[100];

    snprintf(buf, 100, "%lld", bid);
    string sbid = grp->getLeadershipBidPrefix() + buf;

    SAFE_CALL_ZK(m_zk.deleteNode(sbid),
                 "Could not delete bid for current leader %s: %s",
                 np->getKey().c_str(),
                 true,
                 true);
}

/*
 * Methods to prepare strings for leadership protocol.
 */
string
Factory::getCurrentLeaderNodeName(const string &gkey)
{
    return
        gkey +
        PATHSEPARATOR +
        LEADERSHIP +
        PATHSEPARATOR +
        CURRENTLEADER;
}
string
Factory::getLeadershipBidsNodeName(const string &gkey)
{
    return
        gkey +
        PATHSEPARATOR +
        LEADERSHIP +
        PATHSEPARATOR +
        BIDS;
}
string
Factory::getLeadershipBidPrefix(const string &gkey)
{
    return
        getLeadershipBidsNodeName(gkey) +
        PATHSEPARATOR +
        BIDPREFIX;
}

/*
 * Register a timer handler.
 */
TimerId
Factory::registerTimer(TimerEventHandler *handler,
                       uint64_t afterTime,
                       ClientData data)
{
    TimerEventPayload *pp =
        new TimerEventPayload(afterTime, handler, data);
    TimerId id = m_timerEventSrc.scheduleAfter(afterTime, pp);
    pp->updateTimerId(id);

    Locker l(&m_timerRegistryLock);
    m_timerRegistry[id] = pp;

    return id;
}

/*
 * Cancel a timer.
 */
bool
Factory::cancelTimer(TimerId id)
{
    Locker l(&m_timerRegistryLock);
    TimerEventPayload *pp = m_timerRegistry[id];

    if (pp == NULL) {
        return false;
    }

    pp->cancel();
    if (m_timerEventSrc.cancelAlarm(id)) {
        return true;
    }
    return false;
}

/*
 * Forget a timer ID. Used when the event is finally
 * delivered (whether or not it is cancelled).
 */
void
Factory::forgetTimer(TimerId id)
{
    Locker l(&m_timerRegistryLock);

    delete m_timerRegistry[id];
    m_timerRegistry.erase(id);
}

/*
 * Update the cached representation of a clusterlib repository object and
 * generate the prototypical cluster event payload to send to registered
 * clients.
 */
ClusterEventPayload *
Factory::updateCachedObject(FactoryEventHandler *cp,
                            zk::ZKWatcherEvent *ep)
{
    TRACE(CL_LOG, "updateCachedObject");

    if (cp == NULL) {
        throw ClusterException("NULL FactoryEventHandler!");
    }
    if (ep == NULL) {
        throw ClusterException("NULL watcher event!");
    }

    const string path = ep->getPath();
    int etype = ep->getType();
    vector<string> components;
    Notifyable *np;

    LOG_INFO(CL_LOG,
              "updateCachedObject: (0x%x, 0x%x, %s)",
              (int) cp,
              (int) ep,
	      path.c_str());

    split(components, path, is_any_of(PATHSEPARATOR));
    /* Check for matches of the path in reverse order since these are
     * prefix matches. */
    if (hasNodeKeyPrefix(components)) {
        np = getNode(components[NODE_NAME_INDEX],
                     getGroup(components[APP_NAME_INDEX],
                              components[GROUP_NAME_INDEX],
                              false),
                     false,
                     false);
    } else if (hasPropertiesKeyPrefix(components)) {
	np = getProperties(path,
			   false);
    } else if (hasDistKeyPrefix(components)) {
        np = getDistribution(components[DIST_NAME_INDEX],
                             getApplication(components[APP_NAME_INDEX], false),
                             false);
    } else if (hasGroupKeyPrefix(components)) {
        np = getGroup(components[GROUP_NAME_INDEX],
                      getApplication(components[APP_NAME_INDEX], false),
                      false);
    } else if (hasAppKeyPrefix(components)) {
        np = getApplication(components[APP_NAME_INDEX], false);
    } else if (!path.compare(SYNC)) {
        /* There is no notifyable for sync */
        np = NULL;
    } else {
        throw ClusterException(string("Unknown event key: ") +
                               path);
    }

    /*
     * Invoke the object handler. It will update the cache. It
     * should also return the kind of user-level event that this
     * event represents.
     */
    Event e = cp->deliver(np, etype, path);
    
    if (e == EN_NO_EVENT) {
        return NULL;
    }
    return new ClusterEventPayload(np, e);
}

/*
 * Establish whether the given notifyable object
 * is 'ready' according to the 'ready' protocol.
 */
bool
Factory::establishNotifyableReady(Notifyable *np)
{
    string ready = "";

    SAFE_CALL_ZK((ready = m_zk.getNodeData(np->getKey(),
                                           &m_zkEventAdapter,
                                           &m_notifyableReadyHandler)),
                 "Reading the value of %s failed: %s",
                 np->getKey().c_str(),
                 true,
                 true);

    if (ready == "ready") {
        np->setReady(true);
    } else {
        np->setReady(false);
    }

    return np->isReady();
}

/*
 * Implement 'ready' protocol for notifyable objects.
 */
Event
Factory::handleNotifyableReady(Notifyable *np,
                               int etype,
                               const string &path)
{
    TRACE(CL_LOG, "handleNotifyableReady");

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    LOG_WARN(CL_LOG, 
              "handleNotifyableReady(%s)",
              np->getKey().c_str());

    (void) establishNotifyableReady(np);
    return EN_NOTIFYABLE_READY;
}

/*
 * Note the existence of a new notifyable, or the destruction of
 * an existing notifyable.
 */
Event
Factory::handleNotifyableExists(Notifyable *np,
                                int etype,
                                const string &path)
{
    TRACE(CL_LOG, "handleNotifyableExists");

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    LOG_WARN(CL_LOG,
             "Got exists event: %d on notifyable: \"%s\"",
             etype,
             np->getKey().c_str());

    /*
     * Re-establish interest in the existence of this notifyable.
     */
    (void) m_zk.nodeExists(path, &m_zkEventAdapter, 
                           &m_notifyableExistsHandler);

    /*
     * Now decide what to do with the event.
     */
    if (etype == DELETED_EVENT) {
        LOG_WARN(CL_LOG,
                 "Deleted event for path: %s",
                 path.c_str());
        np->setReady(false);
        return EN_NOTIFYABLE_DELETED;
    }
    if (etype == CREATED_EVENT) {
        LOG_WARN(CL_LOG,
                 "Created event for path: %s",
                 path.c_str());
        establishNotifyableReady(np);
        return EN_NOTIFYABLE_CREATED;
    }

    /*
     * SHOULD NOT HAPPEN!
     */
    return EN_NO_EVENT;
}

/*
 * Handle a change in the set of applications
 */
Event
Factory::handleApplicationsChange(Notifyable *np,
                                  int etype,
                                  const string &path)
{
    TRACE(CL_LOG, "handleApplicationsChange");

    /*
     * For now return EN_NO_EVENT.
     */
    return EN_NO_EVENT;
}

/*
 * Handle a change in the set of groups for an
 * application.
 */
Event
Factory::handleGroupsChange(Notifyable *np,
                            int etype,
                            const string &path)
{
    TRACE(CL_LOG, "handleGroupsChange");

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    LOG_WARN(CL_LOG,
              "Got groups change event for : \"%s\"",
              np->getKey().c_str());

    /*
     * Convert to application object.
     */
    Application *app = dynamic_cast<Application *>(np);
    if (app == NULL) {
        LOG_FATAL(CL_LOG,
                  "Expected application object for %s",
                  path.c_str());
        return EN_NO_EVENT;
    }

    if (app->cachingGroups()) {
        app->recacheGroups();
    }

    return EN_APP_GROUPSCHANGE;
}

/*
 * Handle a change in the set of distributions
 * for an application.
 */
Event
Factory::handleDistributionsChange(Notifyable *np,
                                   int etype,
                                   const string &path)
{
    TRACE(CL_LOG, "handleDistributionsChange");

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    LOG_WARN(CL_LOG,
             "Got dists change event for : \"%s\"",
             np->getKey().c_str());

    /*
     * Convert to application object.
     */
    Application *app = dynamic_cast<Application *>(np);
    if (app == NULL) {
        LOG_FATAL(CL_LOG,
                  "Expected application object for %s",
                  path.c_str());
        return EN_NO_EVENT;
    }

    /*
     * If we are caching the distribution objects,
     * then update the cache.
     */
    if (app->cachingDists()) {
        app->recacheDists();
    }

    return EN_APP_DISTSCHANGE;
}

/*
 * Handle a change in the set of nodes in a group.
 */
Event
Factory::handleNodesChange(Notifyable *np,
                           int etype,
                           const string &path)
{
    TRACE(CL_LOG, "handleNodesChange");

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    LOG_WARN(CL_LOG,
             "Got nodes change event for : \"%s\"",
             np->getKey().c_str());

    /*
     * Convert to a group object.
     */
    Group *grp = dynamic_cast<Group *>(np);
    if (grp == NULL) {
        LOG_FATAL(CL_LOG,
                  "Expected group object for %s",
                  path.c_str());
        return EN_NO_EVENT;
    }

    /*
     * If we are caching the node objects,
     * then update the cache.
     */
    if (grp->cachingNodes()) {
        grp->recacheNodes();
    }

    return EN_GRP_MEMBERSHIP;
}

/*
 * Handle a change in the value of a property list.
 */
Event
Factory::handlePropertiesChange(Notifyable *np,
                                int etype,
                                const string &path)
{
    TRACE(CL_LOG, "handlePropertiesChange");

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    LOG_DEBUG(CL_LOG,
              "handlePropertiesChange: Got event %d on properties \"%s\"",
              etype,
              np->getKey().c_str());

    /*
     * Re-establish interest in the existence of this notifyable.
     */
    SAFE_CALL_ZK(m_zk.nodeExists(path, 
                                 &m_zkEventAdapter, 
                                 &m_notifyableExistsHandler),
                 "Could not establish interest in the existence of %s: %s",
                 path.c_str(),
                 false,
                 true);

    /*
     * Now decide what to do with the event.
     */
    if (etype == DELETED_EVENT) {
	LOG_WARN(CL_LOG,
                 "handlePropertiesChange: Deleted event for path: %s",
                 path.c_str());
	np->setReady(false);
	return EN_NOTIFYABLE_DELETED;
    }
    if (etype == CREATED_EVENT) {
	LOG_WARN(CL_LOG,
                 "handlePropertiesChange: Created event for path: %s",
                 path.c_str());
	establishNotifyableReady(np);
	return EN_NOTIFYABLE_CREATED;
    }
    if (etype == CHANGED_EVENT) {
	LOG_WARN(CL_LOG,
                 "handlePropertiesChange: Changed event for path: %s",
                 path.c_str());
	np->updateCachedRepresentation();
	return EN_PROP_CHANGE;
    }

    return EN_NO_EVENT;
}

/*
 * Handle change in shards of a distribution.
 */
Event
Factory::handleShardsChange(Notifyable *np,
                            int etype,
                            const string &path)
{
    TRACE(CL_LOG, "handleShardsChange");

    return EN_NO_EVENT;
}

/*
 * Handle change in manual overrides of a distribution.
 */
Event
Factory::handleManualOverridesChange(Notifyable *np,
                                     int etype,
                                     const string &path)
{
    TRACE(CL_LOG, "handleManualOverridesChange");

    return EN_NO_EVENT;
}

/*
 * Handle change in client-reported state for a node.
 */
Event
Factory::handleClientStateChange(Notifyable *np,
                                 int etype,
                                 const string &path)
{
    TRACE(CL_LOG, "handleClientStateChange");

    return EN_NO_EVENT;
}

/*
 * Handle change in master-set desired state for
 * a node.
 */
Event
Factory::handleMasterSetStateChange(Notifyable *np,
                                    int etype,
                                    const string &path)
{
    TRACE(CL_LOG, "handleMasterSetStateChange");

    return EN_NO_EVENT;
}

/*
 * Handle change in the leadership of a group.
 */
Event
Factory::handleLeadershipChange(Notifyable *np,
                                int etype,
                                const string &path)
{
    TRACE( CL_LOG, "handleLeadershipChange" );

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    bool exists = false;
    SAFE_CALL_ZK((exists = m_zk.nodeExists(path,
                                           &m_zkEventAdapter,
                                           &m_leadershipChangeHandler)),
                 "Could not determine whether node %s exists: %s",
                 path.c_str(),
                 false,
                 true);

    Group *grp = dynamic_cast<Group *>(np);
    if (!exists) {
        grp->updateLeader(NULL);
    }
    else {
        string lname = "";
        SAFE_CALL_ZK((lname = m_zk.getNodeData(path,
                                               &m_zkEventAdapter,
                                               &m_leadershipChangeHandler)),
                     "Could not read current leader for group %s: %s",
                     grp->getKey().c_str(),
                     false,
                     true);
        if (lname == "") {
            grp->updateLeader(NULL);
        }
        else {
            grp->updateLeader(getNode(lname, grp, true, false));
        }
    }

    return EN_GRP_LEADERSHIP;
}

/*
 * Handle existence change for preceding leader of
 * a group. This is called whenever a preceding leader
 * being watched by a server in this process abdicates.
 */
Event
Factory::handlePrecLeaderExistsChange(Notifyable *np,
                                      int etype,
                                      const string &path)
{
    TRACE(CL_LOG, "handlePrecLeaderExistsChange");

    /*
     * If there's no notifyable, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Punting on event: %d on %s",
                 etype,
                 path.c_str());
        return EN_NO_EVENT;
    }

    Server *sp;
    LeadershipElectionMultimap c;
    LeadershipIterator i;
    LeadershipElectionMultimapRange r;

    {
        Locker l1(&m_lwLock);

        /*
         * Make our own copy of the watches map.
         */
        c = m_leadershipWatches;

        /*
         * And remove the key from the watches map.
         */
        r = m_leadershipWatches.equal_range(path);
        m_leadershipWatches.erase(r.first, r.second);
    }

    /*
     * Find the range containing the watches for the
     * node we got notified about (there may be several
     * because there can be several Server instances in
     * this process).
     */
    r = c.equal_range(path);

    /*
     * Make all interested Servers participate in the
     * election again.
     */
    Group *grp = dynamic_cast<Group *>(np);
    Group *grp1;

    for (i = r.first; i != r.second; i++) {
        /*
         * Sanity check -- the path is the same...
         */
        if ((*i).first != path) {
            LOG_FATAL(CL_LOG,
                      "Internal error: bad leadership watch (bid) %s vs %s",
                      path.c_str(), (*i).first.c_str());
            ::abort();
        }

        sp = (*i).second;

        /*
         * Sanity check -- the Server must be in this group...
         */
        grp1 = sp->getMyNode()->getGroup();
        if (grp != grp1) {
            LOG_FATAL(CL_LOG,
                      "Internal error: bad leadership watch (grp) %s vs %s",
                      grp->getKey().c_str(), grp1->getKey().c_str());
            ::abort();
        }
        
        /*
         * Try to make this Server the leader. Ignore the result, we
         * must give everyone a chance.
         */
        (void) sp->tryToBecomeLeader();
    }

    return EN_GRP_LEADERSHIP;
}

/*
 * Special handler for synchronization that waits until the zk event
 * is propogated through the m_eventAdapter before returning.  This
 * ensures that all zk events prior to this point are visible by each
 * client.
 */
Event
Factory::handleSynchronizeChange(Notifyable *np,
                                 int etype,
                                 const string &path)
{
    {
        AutoLock l1(m_syncLock);
        ++m_syncIdCompleted;
        m_syncCond.Signal();
    }

    LOG_DEBUG(CL_LOG, "handleSynchronizeChange: sent conditional signal");
    return EN_NO_EVENT;
}

/*
 * Re-establish the ZooKeeper connection and
 * re-initialize all the watches.
 */
void
Factory::reestablishConnectionAndState(const char *what)
{
    /* TBD -- Real implementation */
    throw ClusterException(what);
}

};	/* End of 'namespace clusterlib' */
