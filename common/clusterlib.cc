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
	    if (m_shutdown) { \
		LOG_WARN(CL_LOG, "Call to ZK during shutdown!"); \
		done = true; \
	    } \
	    else if (!ze.isConnected()) { \
		throw ClusterException(ze.what()); \
	    } \
	    else if (_warning) { \
		LOG_WARN(CL_LOG, _message, _node, ze.what()); \
		if (_once) { \
		    /* \
		     * Only warn once. \
		     */ \
		    done = true; \
                } \
	    } \
	    else { \
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
const string ClusterlibStrings::KEYSEPARATOR = "/";

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
const int32_t ClusterlibInts::CLUSTERLIB_INDEX = 1;
const int32_t ClusterlibInts::VERSION_NAME_INDEX = 2;
const int32_t ClusterlibInts::APP_INDEX = 3;
const int32_t ClusterlibInts::APP_NAME_INDEX = 4;

/*
 * Number of components in an Application key
 */
const int32_t ClusterlibInts::APP_COMPONENTS_COUNT = 5;

/*
 * Minimum components necessary to represent each respective key
 */
const int32_t ClusterlibInts::DIST_COMPONENTS_MIN_COUNT = 7;
const int32_t ClusterlibInts::PROP_COMPONENTS_MIN_COUNT = 6;
const int32_t ClusterlibInts::GROUP_COMPONENTS_MIN_COUNT = 5;
const int32_t ClusterlibInts::NODE_COMPONENTS_MIN_COUNT = 7;

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
      m_endEventDispatched(false),
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
      m_propertiesValueChangeHandler(
	  this,
          &Factory::handlePropertiesValueChange),
      m_applicationsChangeHandler(
          this,
          &Factory::handleApplicationsChange),
      m_groupsChangeHandler(
	  this,
          &Factory::handleGroupsChange),
      m_distributionsChangeHandler(
	  this,
          &Factory::handleDataDistributionsChange),
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
      m_nodeConnectionChangeHandler(
          this,
          &Factory::handleNodeConnectionChange),
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

    injectEndEvent();
    waitForThreads();

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
 * Inject an END event so that all threads and clients
 * event loops will finish up in an orderly fashion.
 */
void
Factory::injectEndEvent()
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
Factory::waitForThreads()
{
    TRACE(CL_LOG, "waitForThreads");

    m_eventThread.Join();
    m_timerHandlerThread.Join();
}

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
Factory::createServer(Group *group,
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
        Server *sp = new Server(mp_ops,
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
};

/*
 * Try to synchronize with the underlying data store.
 */
void
Factory::synchronize()
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

    string key(ROOTNODE);
    key.append(CLUSTERLIB);
    SAFE_CALL_ZK(m_zk.sync(key, 
                           &m_zkEventAdapter, 
                           &m_synchronizeChangeHandler),
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

    Locker l(getClientsLock());
    
    m_clients.push_back(clp);
}

void
Factory::removeClient(Client *clp)
{
    TRACE(CL_LOG, "removeClient");

    Locker l(getClientsLock());
    ClientList::iterator clIt = find(m_clients.begin(),
                                     m_clients.end(),
                                     clp);

    if (clIt == m_clients.end()) {
        return;
    }
    m_clients.erase(clIt);
}

void
Factory::removeAllClients()
{
    TRACE(CL_LOG, "removeAllClients");

    Locker l(getClientsLock());
    ClientList::iterator clIt;
    for (clIt  = m_clients.begin();
         clIt != m_clients.end();
         clIt++)
    {
	delete *clIt;
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

    Locker l(getDataDistributionsLock());
    DataDistributionMap::iterator ddIt;

    for (ddIt = m_dataDistributions.begin();
         ddIt != m_dataDistributions.end();
         ddIt++)
    {
	delete ddIt->second;
    }
    m_dataDistributions.clear();
}
void
Factory::removeAllProperties()
{
    TRACE(CL_LOG, "removeAllProperties");

    Locker l(getPropertiesLock());
    PropertiesMap::iterator pIt;

    for (pIt = m_properties.begin();
         pIt != m_properties.end();
         pIt++)
    {
	delete pIt->second;
    }
    m_properties.clear();
}
void
Factory::removeAllApplications()
{
    TRACE(CL_LOG, "removeAllApplications");

    Locker l(getApplicationsLock());
    ApplicationMap::iterator aIt;

    for (aIt = m_applications.begin();
         aIt != m_applications.end();
         aIt++)
    {
	delete aIt->second;
    }
    m_applications.clear();
}
void
Factory::removeAllGroups()
{
    TRACE(CL_LOG, "removeAllGroups");

    Locker l(getGroupsLock());
    GroupMap::iterator gIt;

    for (gIt = m_groups.begin();
         gIt != m_groups.end();
         gIt++)
    {
	delete gIt->second;
    }
    m_groups.clear();
}
void
Factory::removeAllNodes()
{
    TRACE(CL_LOG, "removeAllNodes");

    Locker l(getNodesLock());
    NodeMap::iterator nIt;

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
Factory::dispatchEvents()
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
                              "(type: %d, state: %d, context: 0x%x)",
                              zp->getType(),
                              zp->getState(),
                              (unsigned int) zp->getContext());
                    
                    if ((zp->getType() == ZOO_SESSION_EVENT) &&
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
Factory::dispatchTimerEvent(ClusterlibTimerEvent *tep)
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
Factory::dispatchZKEvent(zk::ZKWatcherEvent *zp)
{
    TRACE(CL_LOG, "dispatchZKEvent");
    
    if (!zp) {
	throw ClusterException("Unexpected NULL ZKWatcherEvent");
    }

    FactoryEventHandler *fehp =
        (FactoryEventHandler *) zp->getContext();
    ClusterEventPayload *cep, *cepp;
    ClientList::iterator clIt;
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
Factory::dispatchSessionEvent(zk::ZKWatcherEvent *zep)
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
Factory::dispatchEndEvent()
{
    TRACE(CL_LOG, "dispatchEndEvent");

    ClientList::iterator clIt;

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
Factory::consumeTimerEvents()
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
 * Factory's cache.
 */
Application *
Factory::getApplication(const string &appName, bool create)
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

    string key = createAppKey(appName);

    {
        Locker l(getApplicationsLock());
        ApplicationMap::const_iterator appIt = m_applications.find(key);

        if (appIt != m_applications.end()) {
            return appIt->second;
        }
    }

    Application *app = loadApplication(appName, key);
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

DataDistribution *
Factory::getDataDistribution(const string &distName,
                             Group *parentGroup,
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

    string key = createDistKey(parentGroup->getKey(), distName);

    {
        Locker l(getDataDistributionsLock());
        DataDistributionMap::const_iterator distIt = 
            m_dataDistributions.find(key);

        if (distIt != m_dataDistributions.end()) {
            return distIt->second;
        }
    }
    DataDistribution *distp = loadDataDistribution(distName, key, parentGroup);
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

Properties *
Factory::getProperties(Notifyable *parent,
		       bool create)
{
    TRACE(CL_LOG, "getProperties");

    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getProperties: NULL parent");                 
        return NULL;
    }

    string key = createPropertiesKey(parent->getKey());

    {
        Locker l(getPropertiesLock());
        PropertiesMap::const_iterator propIt = m_properties.find(key);

        if (propIt != m_properties.end()) {
            return propIt->second;
        }
    }

    Properties *prop = loadProperties(key, parent);
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

Group *
Factory::getGroup(const string &groupName,
                  Group *parentGroup,
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
    string key = createGroupKey(parentGroup->getKey(), groupName);

    {
        Locker l(getGroupsLock());
        GroupMap::const_iterator groupIt = m_groups.find(key);

        if (groupIt != m_groups.end()) {
            return groupIt->second;
        }
    }

    Group *grp = loadGroup(groupName, key, parentGroup);
    if (grp != NULL) {
        return grp;
    }
    if (create == true) {
        return createGroup(groupName, key, parentGroup);
    }

    LOG_WARN(CL_LOG,
             "getGroup: group %s not found nor created",
             groupName.c_str());

    return NULL;
}

Node *
Factory::getNode(const string &nodeName,
                 Group *parentGroup,
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

    string key = createNodeKey(parentGroup->getKey(),
                               nodeName,
                               managed);

    {
        Locker l(getNodesLock());
        NodeMap::const_iterator nodeIt = m_nodes.find(key);

        if (nodeIt != m_nodes.end()) {
            return nodeIt->second;
        }
    }

    Node *np = loadNode(nodeName, key, parentGroup);
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
Factory::updateDataDistribution(const string &distKey,
                                const string &shards,
                                const string &manualOverrides,
                                int32_t shardsVersion,
                                int32_t manualOverridesVersion)
{
    TRACE(CL_LOG, "updateDataDistribution");
    
    string snode = 
	distKey +
        KEYSEPARATOR +
	SHARDS;
    string monode =
        distKey +
        KEYSEPARATOR +
        MANUALOVERRIDES;
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
        SAFE_CALL_ZK(m_zk.createNode(snode, shards, 0, true),
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
    SAFE_CALL_ZK(m_zk.getNodeData(snode,
                                  &m_zkEventAdapter, 
                                  &m_shardsChangeHandler),
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
Factory::updateProperties(const string &propsKey,
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
        SAFE_CALL_ZK(m_zk.createNode(propsKey, propsValue, 0, true),
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
    SAFE_CALL_ZK(m_zk.getNodeData(propsKey,
                                  &m_zkEventAdapter,
                                  &m_propertiesValueChangeHandler),
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
Factory::updateNodeClientState(const string &nodeKey,
                               const string &cs)
{
    TRACE(CL_LOG, "updateNodeClientState");

    string csKey = nodeKey + KEYSEPARATOR + CLIENTSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(csKey)),
                 "Could not determine whether key %s exists: %s",
                 csKey.c_str(),
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
Factory::updateNodeClientStateDesc(const string &nodeKey,
                                   const string &desc)
{
    TRACE(CL_LOG, "updateNodeClientStateDesc");

    string csKey = nodeKey + KEYSEPARATOR + CLIENTSTATEDESC;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(csKey)),
                 "Could not determine whether key %s exists: %s",
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
Factory::updateNodeMasterSetState(const string &nodeKey,
                                  const string &ms)
{
    TRACE(CL_LOG, "updateNodeMasterSetState");

    string msKey = nodeKey + KEYSEPARATOR + MASTERSETSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(msKey)),
                 "Could not determine whether key %s exists: %s",
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

Notifyable *
Factory::getNotifyableFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getNotifyableFromKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return getNotifyableFromComponents(components, -1, create);
}
Notifyable *
Factory::getNotifyableFromComponents(const vector<string> &components,
                                     int32_t elements, 
                                     bool create)
{
    TRACE(CL_LOG, "getNotifyableFromComponents");

    if (elements == -1) {
        elements = components.size();
    }

    Notifyable *ntp = NULL;    
    int32_t clusterObjectElements = elements;

    LOG_DEBUG(CL_LOG, "getNotifyableFromComponents: elements %d", elements);

    while (clusterObjectElements >= ClusterlibInts::APP_COMPONENTS_COUNT) {
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

        clusterObjectElements = removeObjectFromComponents(
            components, 
            clusterObjectElements);
    }

    return NULL;
}

Application *
Factory::getApplicationFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getApplicationFromKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return getApplicationFromComponents(components, -1, create);
}
Application *
Factory::getApplicationFromComponents(const vector<string> &components, 
                                      int32_t elements,
                                      bool create)
{
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!isApplicationKey(components, elements)) {
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

DataDistribution *
Factory::getDataDistributionFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getDataDistributionFromKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return getDataDistributionFromComponents(components, -1, create);
}
DataDistribution *
Factory::getDataDistributionFromComponents(const vector<string> &components, 
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

    if (!isDataDistributionKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getDataDistributionFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        removeObjectFromComponents(components, elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    Group *parent = getGroupFromComponents(components,
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

Properties *
Factory::getPropertiesFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getPropertiesFromKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return getPropertiesFromComponents(components, -1, create);
}
Properties *
Factory::getPropertiesFromComponents(const vector<string> &components, 
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

   if (!isPropertiesKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getPropertiesFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        removeObjectFromComponents(components, elements);
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

Group *
Factory::getGroupFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getGroupFromKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return getGroupFromComponents(components, -1, create);
}
Group *
Factory::getGroupFromComponents(const vector<string> &components, 
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
    if (isApplicationKey(components, elements)) {
        return getApplicationFromComponents(components, elements, create);
    }
    
    if (!isGroupKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getGroupFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    int32_t parentGroupCount = 
        removeObjectFromComponents(components, elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    Group *parent = getGroupFromComponents(components,
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

Node *
Factory::getNodeFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getNodeFromKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return getNodeFromComponents(components, -1, create);
}
Node *
Factory::getNodeFromComponents(const vector<string> &components, 
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

    if (!isNodeKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getNodeFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        removeObjectFromComponents(components, elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    Group *parent = getGroupFromComponents(components,
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
 * Key creation and recognition.
 */
string
Factory::createNodeKey(const string &groupKey,
                       const string &nodeName,
                       bool managed)
{
    string res =
        groupKey +
        KEYSEPARATOR +
        (managed ? NODES : UNMANAGEDNODES) +
        KEYSEPARATOR +
        nodeName
        ;

    return res;
}

string
Factory::createGroupKey(const string &groupKey,
                        const string &groupName)
{
    string res = 
        groupKey +
        KEYSEPARATOR +
        GROUPS +
        KEYSEPARATOR +
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
        KEYSEPARATOR +
        CLUSTERLIBVERSION +
        KEYSEPARATOR +
        APPLICATIONS +
        KEYSEPARATOR +
        appName
        ;

    return res;
}

string
Factory::createDistKey(const string &groupKey,
                       const string &distName)
{
    string res =
        groupKey +
        KEYSEPARATOR +
        DISTRIBUTIONS +
        KEYSEPARATOR +
        distName
        ;

    return res;
}

string
Factory::createPropertiesKey(const string &notifyableKey)
{
    string res =
	notifyableKey +
        KEYSEPARATOR +
        PROPERTIES
	;

    return res;
}

bool
Factory::isApplicationKey(const vector<string> &components, int32_t elements)
{
    TRACE(CL_LOG, "isApplicationKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isApplicationKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isApplicationKey: elements > size of "
                               "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (elements != ClusterlibInts::APP_COMPONENTS_COUNT) {
        return false;
    }

    if ((components.at(CLUSTERLIB_INDEX) != ClusterlibStrings::CLUSTERLIB) ||
        (components.at(APP_INDEX) != ClusterlibStrings::APPLICATIONS)) {
        return false;
    } 

    return true;    
}

bool
Factory::isApplicationKey(const string &key)
{
    TRACE(CL_LOG, "isApplicationKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return isApplicationKey(components);
}

bool
Factory::isDataDistributionKey(const vector<string> &components, 
                               int32_t elements)
{
    TRACE(CL_LOG, "isDataDistributionKey");
    
    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isDataDistributionKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isDataDistributionKey: elements > size of "
                               "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a distribution
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::DIST_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent group are valid.
     */
    if (!isGroupKey(components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is DISTRIBUTIONS and
     * that the distribution name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::DISTRIBUTIONS) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

bool
Factory::isDataDistributionKey(const string &key)
{
    TRACE(CL_LOG, "isDataDistributionKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return isDataDistributionKey(components);    
}

bool
Factory::isGroupKey(const vector<string> &components, 
                    int32_t elements)
{
    TRACE(CL_LOG, "isGroupKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isGroupKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isGroupKey: elements > size of "
                               "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a group/app
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::GROUP_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent groups (recursively) and
     * application are valid.
     */
    if (elements == ClusterlibInts::APP_COMPONENTS_COUNT) {
        if (!isApplicationKey(components, elements)) {
            return false;
        }
    }
    else if (elements >= ClusterlibInts::APP_COMPONENTS_COUNT + 2) {
        if (!isGroupKey(components, elements - 2)) {
            return false;
        }
    }
    else { 
        /*
         * Shouldn't happen.
         */
        return false;
    }

    /*
     * Check that the second to the last element is APPLICATIONS or GROUPS and
     * that the group name is not empty.
     */
    if (((components.at(elements - 2) != ClusterlibStrings::APPLICATIONS) &&
         (components.at(elements - 2) != ClusterlibStrings::GROUPS)) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

bool
Factory::isGroupKey(const string &key)
{
    TRACE(CL_LOG, "isGroupKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return isGroupKey(components);    
}

bool
Factory::isPropertiesKey(const vector<string> &components, 
                         int32_t elements)
{
    TRACE(CL_LOG, "isPropertiesKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isPropertiesKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isPropertiesKey: elements > size of "
                               "components");
    }
    
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a properties
     * and that after the Application key there are an odd number of
     * elements left.
     */
    if ((elements < ClusterlibInts::PROP_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 1))  {
        return false;
    }

    /*
     * Check that the each of the parent of the properties is a
     * Notifyable and that their parents are a group or application.
     */ 
    if (((components.at(elements - 3) != ClusterlibStrings::APPLICATIONS) &&
         (components.at(elements - 3) != ClusterlibStrings::GROUPS) &&
         (components.at(elements - 3) != ClusterlibStrings::DISTRIBUTIONS) &&
         (components.at(elements - 3) != ClusterlibStrings::UNMANAGEDNODES) &&
         (components.at(elements - 3) != ClusterlibStrings::NODES)) ||
        (components.at(elements - 1) != ClusterlibStrings::PROPERTIES)) {
        return false;
    }

    if (elements >= ClusterlibInts::APP_COMPONENTS_COUNT) {
        if ((!isGroupKey(components, elements - 1)) &&
            (!isDataDistributionKey(components, elements - 1)) &&
            (!isNodeKey(components, elements - 1))) {
            return false; 
        }
    }
    else {
        /*
         * Shouldn't happen.
         */
        return false;
    }
            
    return true;    
}

bool
Factory::isPropertiesKey(const string &key)
{
    TRACE(CL_LOG, "isPropertiesKey");
    
    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return isPropertiesKey(components);    
}

bool
Factory::isNodeKey(const vector<string> &components, 
                   int32_t elements)
{
    TRACE(CL_LOG, "isNodeKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isNodeKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isNodeKey: elements > size of "
                               "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a node
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::NODE_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent group are valid.
     */
    if (!isGroupKey(components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is DISTRIBUTIONS and
     * that the distribution name is not empty.
     */
    if (((components.at(elements - 2) != ClusterlibStrings::NODES) &&
         (components.at(elements - 2) != ClusterlibStrings::UNMANAGEDNODES)) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

bool
Factory::isNodeKey(const string &key)
{
    TRACE(CL_LOG, "isNodeKey");

    vector<string> components;
    split(components, key, is_any_of(KEYSEPARATOR));
    return isNodeKey(components);    
}

string 
Factory::removeObjectFromKey(const string &key)
{
    TRACE(CL_LOG, "removeObjectFromKey");

    if (key.empty() ||
        (key.substr(key.size() - 1) == ClusterlibStrings::KEYSEPARATOR)) {
        return string();
    }

    string res = key;
    bool objectFound = false;
    uint32_t beginKeySeparator = numeric_limits<uint32_t>::max();
    uint32_t endKeySeparator = numeric_limits<uint32_t>::max();
    while (objectFound == false) {
        /*
         * Get rid of the leaf node 
         */
        endKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR);
        if ((endKeySeparator == string::npos) ||
            (endKeySeparator == 0)) {
            return string();
        }
        res.erase(endKeySeparator);

        /*
         * If this key represents a valid Notifyable, then it should
         * be /APPLICATIONS|GROUPS|NODES|DISTRIBUTIONS/name.
         */
        endKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR);
        if ((endKeySeparator == string::npos) ||
            (endKeySeparator == 0)) {
            return string();
        }
        beginKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR, 
                                      endKeySeparator - 1);
        if ((beginKeySeparator == string::npos) ||
            (beginKeySeparator == 0)) {
            return string();
        }
        
        /* 
         * Try to find a clusterlib object in this portion of the key 
         */
        string clusterlibObject = 
            res.substr(beginKeySeparator + 1, 
                       endKeySeparator - beginKeySeparator - 1);
        if ((clusterlibObject.compare(APPLICATIONS) == 0) ||
            (clusterlibObject.compare(GROUPS) == 0) ||
            (clusterlibObject.compare(NODES) == 0) ||
            (clusterlibObject.compare(DISTRIBUTIONS) == 0)) {
            objectFound = true;
        }
    }

    LOG_DEBUG(CL_LOG, 
              "removeObjectFromKey: Changed key %s to %s",
              key.c_str(), 
              res.c_str());

    return res;
}

int32_t 
Factory::removeObjectFromComponents(const vector<string> &components,
                                    int32_t elements)
{
    TRACE(CL_LOG, "removeObjectFromComponents");

    if (components.empty() ||
        (components.back() == ClusterlibStrings::KEYSEPARATOR)) {
        return -1;
    }

    int32_t clusterlibObjectIndex = -1;
    bool objectFound = false;
    while (objectFound == false) {
        /*
         * Get rid of the leaf node 
         */
        elements--;
        if (elements < 2) {
            return -1;
        }

        /*
         * If this key represents a valid Notifyable, then it should
         * be /APPLICATIONS|GROUPS|NODES|DISTRIBUTIONS/name.
         */
        clusterlibObjectIndex = elements - 2;
        
        /* 
         * Try to find a clusterlib object in this component
         */
        if ((components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::APPLICATIONS) == 0) ||
            (components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::GROUPS) == 0) ||
            (components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::NODES) == 0) ||
            (components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::DISTRIBUTIONS) == 0)) {
            objectFound = true;
        }
    }

    LOG_DEBUG(CL_LOG, 
              "removeObjectFromComponents: Changed key from %s/%s to %s/%s",
              components.at(elements - 2).c_str(),
              components.at(elements - 1).c_str(),
              components.at(clusterlibObjectIndex).c_str(),
              components.at(clusterlibObjectIndex + 1).c_str());

    /*
     * +2 since 1 for including the name of the clusterlib object and
     * 1 since this is the elements count and not the index.
     */
    return clusterlibObjectIndex + 2;
}

/*
 * Entity loading from ZooKeeper. Also add it to the global cache.
 */
Application *
Factory::loadApplication(const string &name,
                         const string &key)
{
    TRACE(CL_LOG, "loadApplication");

    vector<string> zkNodes;
    Application *app;
    bool exists = false;
    Locker l(getApplicationsLock());

    ApplicationMap::const_iterator appIt = m_applications.find(key);
    if (appIt != m_applications.end()) {
        return appIt->second;
    }

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key, 
                                           &m_zkEventAdapter,
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    app = new Application(name, key, mp_ops);

    /*
     * Set up ready protocol.
     */
    establishNotifyableReady(app);

    m_applications[key] = app;

    return app;
}
    
DataDistribution *
Factory::loadDataDistribution(const string &distName,
                              const string &distKey,
                              Group *parentGroup)
{
    TRACE(CL_LOG, "loadDataDistribution");

    DataDistribution *distp;
    bool exists = false;

    /*
     * Ensure that we have a cached object for this data
     * distribution in the cache.
     */
    Locker l(getDataDistributionsLock());

    DataDistributionMap::const_iterator distIt = 
        m_dataDistributions.find(distKey);
    if (distIt != m_dataDistributions.end()) {
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
    distp = new DataDistribution(parentGroup,
                                 distName,
                                 distKey,
                                 mp_ops);
    m_dataDistributions[distKey] = distp;

    /*
     * Set up the 'ready' protocol.
     */
    establishNotifyableReady(distp);

    return distp;
}

string
Factory::loadShards(const string &key, int32_t &version)
{
    Stat stat;
    string snode =
        key +
        KEYSEPARATOR +
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
        KEYSEPARATOR +
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
Factory::loadProperties(const string &propsKey,
                        Notifyable *parent)
{
    TRACE(CL_LOG, "Properties");

    Properties *prop;
    bool exists = false;
    Locker l(getPropertiesLock());

    PropertiesMap::const_iterator propIt =
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
    SAFE_CALL_ZK(m_zk.getNodeData(propsKey,
                                  &m_zkEventAdapter,
                                  &m_propertiesValueChangeHandler),
                 "Reading the value of %s failed: %s",
                 propsKey.c_str(),
                 false,
                 true);
    prop = new Properties(parent, propsKey, mp_ops);
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
Factory::loadKeyValMap(const string &key, int32_t &version)
{
    Stat stat;
    string kvnode = "";

    SAFE_CALL_ZK((kvnode = m_zk.getNodeData(key,
                                            &m_zkEventAdapter,
                                            &m_propertiesValueChangeHandler,
                                            &stat)),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 false,
                 true);

    version = stat.version;

    return kvnode;
}

Group *
Factory::loadGroup(const string &groupName,
                   const string &groupKey,
                   Group *parentGroup)
{
    TRACE(CL_LOG, "loadGroup");

    Group *grp;
    bool exists = false;
    Locker l(getGroupsLock());

    GroupMap::const_iterator groupIt = m_groups.find(groupKey);
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
    grp = new Group(groupName,
                    groupKey,
                    mp_ops,
                    parentGroup);
    m_groups[groupKey] = grp;

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
    Locker l(getNodesLock());

    NodeMap::const_iterator nodeIt = m_nodes.find(key);
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
    np = new Node(grp, name, key, mp_ops);
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
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        string groups = key + KEYSEPARATOR + GROUPS;
        string dists = key + KEYSEPARATOR + DISTRIBUTIONS;
        string props = key + KEYSEPARATOR + PROPERTIES;

        /*
         * Create the application data structure if needed.
         */
        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(groups, "", 0, true),
                     "Could not create key %s: %s",
                     groups.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(dists, "", 0, true),
                     "Could not create key %s: %s",
                     dists.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
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

DataDistribution *
Factory::createDataDistribution(const string &name,
                                const string &key,
                                const string &marshalledShards,
                                const string &marshalledManualOverrides,
                                Group *parentGroup)
{
    TRACE(CL_LOG, "createDataDistribution");

    DataDistribution *distp;
    bool created = false;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key,
                                           &m_zkEventAdapter,
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        string shards = key + KEYSEPARATOR + SHARDS;
        string mos = key + KEYSEPARATOR + MANUALOVERRIDES;
        string props = key + KEYSEPARATOR + PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(shards, "", 0, true),
                     "Could not create key %s: %s",
                     shards.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(mos, "", 0, true),
                     "Could not create key %s: %s",
                     mos.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
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

Properties *
Factory::createProperties(const string &key,
                          Notifyable *parent) 
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
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.getNodeData(key, 
                                  &m_zkEventAdapter,
                                  &m_propertiesValueChangeHandler),
                 "Could not read value of %s: %s",
                 key.c_str(),
                 false,
                 true);

    /*
     * Load the properties, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    Properties *prop = loadProperties(key, parent);
    
    /*
     * No ready protocol necessary for Properties.
     */
    
    return prop;

}

Group *
Factory::createGroup(const string &groupName, 
		     const string &groupKey,
                     Group *parentGroup)
{
    TRACE(CL_LOG, "createGroup");

    Group *grp = NULL;
    bool created = false;
    bool exists = false;
    
    SAFE_CALL_ZK((exists = m_zk.nodeExists(groupKey,
                                           &m_zkEventAdapter,
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether key %s exists: %s",
                 groupKey.c_str(),
                 false,
                 true);
    if (!exists) {
        string nodes = groupKey + KEYSEPARATOR + NODES;
        string leadership = groupKey + KEYSEPARATOR + LEADERSHIP;
        string bids = leadership + KEYSEPARATOR + BIDS;
        string props = groupKey + KEYSEPARATOR + PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(groupKey, "", 0, true),
                     "Could not create key %s: %s",
                     groupKey.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(nodes, "", 0, true),
                     "Could not create key %s: %s",
                     nodes.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(leadership, "", 0, true),
                     "Could not create key %s: %s",
                     leadership.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(bids, "", 0, true),
                     "Could not create key %s: %s",
                     bids.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
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
    grp = loadGroup(groupName, groupKey, parentGroup);
    
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

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key,
                                           &m_zkEventAdapter, 
                                           &m_notifyableExistsHandler)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        string cs = key + KEYSEPARATOR + CLIENTSTATE;
        string ms = key + KEYSEPARATOR + MASTERSETSTATE;
        string cv = key + KEYSEPARATOR + CLIENTVERSION;
        string props = key + KEYSEPARATOR + PROPERTIES;

        created = true;
        SAFE_CALL_ZK(m_zk.createNode(key, "", 0, true),
                     "Could not create key %s: %s",
                     key.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(cs, "", 0, true),
                     "Could not create key %s: %s",
                     cs.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(ms, "", 0, true),
                     "Could not create key %s: %s",
                     ms.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(cv, "1.0", 0, true),
                     "Could not create key %s: %s",
                     cv.c_str(),
                     true,
                     true);
        SAFE_CALL_ZK(m_zk.createNode(props, "", 0, true),
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
 * Retrieve bits of Node state.
 */
bool
Factory::isNodeConnected(const string &nodeKey)
{
    TRACE(CL_LOG, "isNodeConnected");

    string ckey =
        nodeKey +
        KEYSEPARATOR +
        CONNECTED;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(ckey,
                                           &m_zkEventAdapter,
                                           &m_nodeConnectionChangeHandler)),
                 "Could not determine whether key %s is connected: %s",
                 nodeKey.c_str(),
                 false,
                 true);
    return exists;
}
string
Factory::getNodeClientState(const string &nodeKey)
{
    TRACE(CL_LOG, "getNodeClientState");

    string ckey =
        nodeKey +
        KEYSEPARATOR +
        CLIENTSTATE;
    string res = "";

    SAFE_CALL_ZK((res = m_zk.getNodeData(ckey,
                                         &m_zkEventAdapter,
                                         &m_nodeClientStateChangeHandler)),
                 "Could not read node %s client state: %s",
                 nodeKey.c_str(),
                 false,
                 true);
    return res;
}
int32_t
Factory::getNodeMasterSetState(const string &nodeKey)
{
    TRACE(CL_LOG, "getNodeMasterSetState");

    string ckey =
        nodeKey +
        KEYSEPARATOR +
        MASTERSETSTATE;
    string res = "";

    SAFE_CALL_ZK((res = m_zk.getNodeData(ckey,
                                         &m_zkEventAdapter,
                                         &m_nodeMasterSetStateChangeHandler)),
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
IdList
Factory::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    IdList list;
    string key =
        ROOTNODE +
        CLUSTERLIB +
        KEYSEPARATOR +
        CLUSTERLIBVERSION +
        KEYSEPARATOR +
        APPLICATIONS;

    list.clear();
    SAFE_CALL_ZK(m_zk.getNodeChildren(list,
                                      key, 
                                      &m_zkEventAdapter,
                                      &m_applicationsChangeHandler),
                 "Reading the value of %s failed: %s",
                 key.c_str(),
                 true,
                 true);

    for (IdList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + KEYSEPARATOR.length());
    }

    return list;
}
IdList
Factory::getGroupNames(Group *grp)
{
    TRACE(CL_LOG, "getGroupNames");

    if (grp == NULL) {
        throw ClusterException("NULL group in getGroupNames");
    }

    IdList list;
    string key =
        grp->getKey() +
        KEYSEPARATOR +
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

    for (IdList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + KEYSEPARATOR.length());
    }

    return list;
}
IdList
Factory::getDataDistributionNames(Group *grp)
{
    TRACE(CL_LOG, "getDistributionNames");

    if (grp == NULL) {
        throw ClusterException("NULL group in getDataDistributionNames");
    }

    IdList list;
    string key=
        grp->getKey() +
        KEYSEPARATOR +
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

    for (IdList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + KEYSEPARATOR.length());
    }

    return list;
}
IdList
Factory::getNodeNames(Group *grp)
{
    TRACE(CL_LOG, "getNodeNames");

    if (grp == NULL) {
        throw ClusterException("NULL group in getNodeNames");
    }

    IdList list;
    string key =
        grp->getKey() +
        KEYSEPARATOR +
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

    for (IdList::iterator iIt = list.begin(); iIt != list.end(); ++iIt) {
        /*
         * Remove the key prefix
         */
        *iIt = iIt->substr(key.length() + KEYSEPARATOR.length());
    }

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

    if (grp == NULL) {
        throw ClusterException("getLeader: NULL group");
    }

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
Factory::placeBid(Node *np, Server *sp)
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

    Group *grp = np->getMyGroup();

    if (grp == NULL) {
        throw ClusterException(string("placeBid: NULL group containing node ") +
                               np->getKey());
    }

    string pfx = grp->getLeadershipBidPrefix();
    string value =
        np->getKey() +
        ";" +
        VERSION +
        ";" +
        tmp;
    int64_t bid = 0;

    SAFE_CALL_ZK((bid = m_zk.createSequence(pfx, value, ZOO_EPHEMERAL, true)),
                 "Bidding with prefix %s to become leader failed: %s",
                 pfx.c_str(),
                 true,
                 true);

    Locker l1(getLeadershipWatchesLock());
    m_leadershipWatches.insert(pair<const string, Server *>(np->getKey(), sp));

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

    if (np == NULL) {
        throw ClusterException("tryToBecomeLeader: NULL node");
    }

    IdList list;
    IdList::iterator iIt;
    Group *grp = np->getMyGroup();

    if (grp == NULL) {
        throw ClusterException(
		string("tryToBecomeLeader: NULL group containing ") +
                np->getKey());
    }

    string lnn = grp->getCurrentLeaderNodeName();
    string bnn = grp->getLeadershipBidsNodeName();
    string pfx = grp->getLeadershipBidPrefix();
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
                 grp->getKey().c_str(),
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
Factory::isLeaderKnown(Node *np)
{
    TRACE(CL_LOG, "isLeaderKnown");

    if (np == NULL) {
        return false;
    }

    Group *grp = np->getMyGroup();
    if (grp == NULL) {
        throw ClusterException(string("NULL group containing ") +
                               np->getKey());
    }

    return grp->isLeaderKnown();
}

/*
 * Denote that the leader of the group in which this node
 * is a member is unknown.
 */
void
Factory::leaderIsUnknown(Node *np)
{
    TRACE(CL_LOG, "leaderIsUnknown");

    if (np == NULL) {
        return;
    }

    Group *grp = np->getMyGroup();
    if (grp == NULL) {
        throw ClusterException(string("NULL group containing ") +
                               np->getKey());
    }

    grp->updateLeader(NULL);
}

/*
 * The server represented by the given node, and that owns the
 * given bid, is no longer the leader of its group (if it ever
 * was).
 */
void
Factory::giveUpLeadership(Node *np, int64_t bid)
{
    TRACE(CL_LOG, "giveUpLeadership");

    if (np == NULL) {
        return;
    }

    Group *grp = np->getMyGroup();
    if (grp == NULL) {
        throw ClusterException(string("NULL group containing ") +
                               np->getKey());
    }

    /*
     * Ensure that our view of the leader is not updated by
     * any other thread.
     */
    {
        Locker l1(grp->getLeadershipLock());
        if (grp->getLeader() == np) {
            /*
             * Delete the current leader node.
             */
            SAFE_CALL_ZK(m_zk.deleteNode(grp->getCurrentLeaderNodeName()),
                         "Could not delete current leader for %s: %s",
                         grp->getKey().c_str(),
                         true,
                         true);

            /*
             * We no longer know who the leader of this group is.
             */
            grp->updateLeader(NULL);
        }
    }

    /*
     * Delete the leadership bid for this node.
     */
    char buf[100];

    snprintf(buf, 100, "%lld", bid);
    string sbid = grp->getLeadershipBidPrefix() + buf;

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
Factory::getCurrentLeaderNodeName(const string &gkey)
{
    return
        gkey +
        KEYSEPARATOR +
        LEADERSHIP +
        KEYSEPARATOR +
        CURRENTLEADER;
}
string
Factory::getLeadershipBidsNodeName(const string &gkey)
{
    return
        gkey +
        KEYSEPARATOR +
        LEADERSHIP +
        KEYSEPARATOR +
        BIDS;
}
string
Factory::getLeadershipBidPrefix(const string &gkey)
{
    return
        getLeadershipBidsNodeName(gkey) +
        KEYSEPARATOR +
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
Factory::cancelTimer(TimerId id)
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
Factory::updateCachedObject(FactoryEventHandler *fehp,
                            zk::ZKWatcherEvent *ep)
{
    TRACE(CL_LOG, "updateCachedObject");

    if (fehp == NULL) {
        throw ClusterException("NULL FactoryEventHandler!");
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

    Notifyable *ntp;

    /*
     * SYNC doesn't get a Notifyable.
     */
    if (path.compare(SYNC) == 0) {
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
 * Establish whether the given notifyable object
 * is 'ready' according to the 'ready' protocol.
 */
bool
Factory::establishNotifyableReady(Notifyable *ntp)
{
    string ready = "";

    if (ntp == NULL) {
        return false;
    }

    SAFE_CALL_ZK((ready = m_zk.getNodeData(ntp->getKey(),
                                           &m_zkEventAdapter,
                                           &m_notifyableReadyHandler)),
                 "Reading the value of %s failed: %s",
                 ntp->getKey().c_str(),
                 true,
                 true);

    if (ready == "ready") {
        ntp->setReady(true);
        ntp->initializeCachedRepresentation();
    }
    else {
        ntp->setReady(false);
    }

    return ntp->isReady();
}

/*
 * Implement 'ready' protocol for notifyable objects.
 */
Event
Factory::handleNotifyableReady(Notifyable *ntp,
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

    (void) establishNotifyableReady(ntp);
    return EN_READY;
}

/*
 * Note the existence of a new notifyable, or the destruction of
 * an existing notifyable.
 */
Event
Factory::handleNotifyableExists(Notifyable *ntp,
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
    SAFE_CALL_ZK(m_zk.nodeExists(key,
                                 &m_zkEventAdapter, 
                                 &m_notifyableExistsHandler),
                 "Could not reestablish exists watch on %s: %s",
                 ntp->getKey().c_str(),
                 false,
                 true);

    /*
     * Now decide what to do with the event.
     */
    if (etype == ZOO_DELETED_EVENT) {
        LOG_WARN(CL_LOG,
                 "Deleted event for key: %s",
                 key.c_str());
        ntp->setReady(false);
        return EN_DELETED;
    }
    if (etype == ZOO_CREATED_EVENT) {
        LOG_WARN(CL_LOG,
                 "Created event for key: %s",
                 key.c_str());
        establishNotifyableReady(ntp);
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
Factory::handleApplicationsChange(Notifyable *ntp,
                                  int32_t etype,
                                  const string &key)
{
    TRACE(CL_LOG, "handleApplicationsChange");

    /*
     * For now return EN_NOEVENT.
     */
    return EN_NOEVENT;
}

/*
 * Handle a change in the set of groups for an
 * application.
 */
Event
Factory::handleGroupsChange(Notifyable *ntp,
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
    Group *grp = dynamic_cast<Group *>(ntp);
    if (grp == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    if (grp->cachingGroups()) {
        grp->recacheGroups();
    }

    return EN_GROUPSCHANGE;
}

/*
 * Handle a change in the set of distributions
 * for an application.
 */
Event
Factory::handleDataDistributionsChange(Notifyable *ntp,
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
    Group *grp = dynamic_cast<Group *>(ntp);
    if (grp == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * If we are caching the distribution objects,
     * then update the cache.
     */
    if (grp->cachingDataDistributions()) {
        grp->recacheDataDistributions();
    }

    return EN_DISTSCHANGE;
}

/*
 * Handle a change in the set of nodes in a group.
 */
Event
Factory::handleNodesChange(Notifyable *ntp,
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
    Group *grp = dynamic_cast<Group *>(ntp);
    if (grp == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * If we are caching the node objects,
     * then update the cache.
     */
    if (grp->cachingNodes()) {
        grp->recacheNodes();
    }

    return EN_MEMBERSHIPCHANGE;
}

/*
 * Handle a change in the value of a property list.
 */
Event
Factory::handlePropertiesValueChange(Notifyable *ntp,
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
     * Convert the Notifyable into a Properties *
     */
    Properties *pp = dynamic_cast<Properties *>(ntp);
    string nv = "";
    Stat stat;

    /*
     * If there's no Properties * then punt.
     */
    if (pp == NULL) {
        LOG_WARN(CL_LOG, 
                 "Conversion to Properties * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    SAFE_CALL_ZK((nv = m_zk.getNodeData(key,
                                        &m_zkEventAdapter,
                                        &m_propertiesValueChangeHandler,
                                        &stat)),
                 "Could not obtain value of properties %s: %s",
                 key.c_str(),
                 false,
                 true);
    pp->unmarshall(nv);
    pp->setKeyValVersion(stat.version);
    pp->setValueChangeTime(Factory::getCurrentTimeMillis());

    return EN_PROPCHANGE;
}

/*
 * Handle change in shards of a data distribution.
 */
Event
Factory::handleShardsChange(Notifyable *ntp,
                            int32_t etype,
                            const string &key)
{
    TRACE(CL_LOG, "handleShardsChange");

    /*
     * If the given Notifyable is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No Notifyable provided -- punting");
        return EN_NOEVENT;
    }

    /*
     * Convert it into a DataDistribution *.
     */
    DataDistribution *distp = dynamic_cast<DataDistribution *>(ntp);

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
Factory::handleManualOverridesChange(Notifyable *ntp,
                                     int32_t etype,
                                     const string &key)
{
    TRACE(CL_LOG, "handleManualOverridesChange");

    /*
     * If the given Notifyable is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No Notifyable provided -- punting");
        return EN_NOEVENT;
    }

    /*
     * Convert it into a DataDistribution *.
     */
    DataDistribution *distp = dynamic_cast<DataDistribution *>(ntp);

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
Factory::handleClientStateChange(Notifyable *ntp,
                                 int32_t etype,
                                 const string &key)
{
    TRACE(CL_LOG, "handleClientStateChange");

    /*
     * If the given Notifyable is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No Notifyable provided -- punting");
        return EN_NOEVENT;
    }

    /*
     * Convert it to a Node *.
     */
    Node *np = dynamic_cast<Node *>(ntp);

    /*
     * If there's no node, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Node * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * Get the current client state and re-establish watch.
     */
    string ns = getNodeClientState(np->getKey());

    /*
     * Update the cache and cause a user-level event if
     * the new value is different from the currently
     * cached value.
     */
    if (ns == np->getClientState()) {
        return EN_NOEVENT;
    }
    np->setClientState(ns);
    np->setClientStateTime(Factory::getCurrentTimeMillis());

    return EN_CLIENTSTATECHANGE;
}

/*
 * Handle change in master-set desired state for
 * a node.
 */
Event
Factory::handleMasterSetStateChange(Notifyable *ntp,
                                    int32_t etype,
                                    const string &key)
{
    TRACE(CL_LOG, "handleMasterSetStateChange");

    /*
     * If the given Notifyable is NULL, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No Notifyable provided -- punting");
        return EN_NOEVENT;
    }

    /*
     * Try to convert to Node *
     */
    Node *np = dynamic_cast<Node *>(ntp);

    /*
     * If there's no node, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Node * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * Get the new value and re-establish watch.
     */
    int32_t nv = getNodeMasterSetState(np->getKey());

    /*
     * Update the cache and cause a user-level event
     * if the new value is different than what is in
     * the cache now.
     */
    if (nv == np->getMasterSetState()) {
        return EN_NOEVENT;
    }
    np->setMasterSetState(nv);
    np->setMasterSetStateTime(Factory::getCurrentTimeMillis());

    return EN_MASTERSTATECHANGE;
}

/*
 * Handle change in the connection state of a node.
 */
Event
Factory::handleNodeConnectionChange(Notifyable *ntp,
                                    int32_t etype,
                                    const string &key)
{
    TRACE(CL_LOG, "handleNodeConnectionChange");

    /*
     * If there's no Notifyable, punt.
     */
    if (ntp == NULL) {
        LOG_WARN(CL_LOG, "No Notifyable provided -- punting");
        return EN_NOEVENT;
    }

    /*
     * Convert it to a Node *
     */
    Node *np = dynamic_cast<Node *>(ntp);

    /*
     * If there's no Node, punt.
     */
    if (np == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Node * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * Get the current value and re-establish watch.
     */
    bool curconn = isNodeConnected(np->getKey());

    /*
     * Cause a user level event and update the cache
     * if the returned value is different than the
     * currently cached value.
     */
    if (curconn == np->isConnected()) {
        return EN_NOEVENT;
    }

    np->setConnected(curconn);
    np->setConnectionTime(Factory::getCurrentTimeMillis());
    return (curconn == true) ? EN_CONNECTED : EN_DISCONNECTED;
}

/*
 * Handle change in the leadership of a group.
 */
Event
Factory::handleLeadershipChange(Notifyable *ntp,
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

    Group *grp = dynamic_cast<Group *>(ntp);
    if (grp == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    grp->setLeadershipChangeTime(Factory::getCurrentTimeMillis());

    bool exists = false;
    SAFE_CALL_ZK((exists = m_zk.nodeExists(key,
                                           &m_zkEventAdapter,
                                           &m_leadershipChangeHandler)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);

    if (!exists) {
        grp->updateLeader(NULL);
    }
    else {
        string lname = "";
        SAFE_CALL_ZK((lname = m_zk.getNodeData(key,
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

    return EN_LEADERSHIPCHANGE;
}

/*
 * Handle existence change for preceding leader of
 * a group. This is called whenever a preceding leader
 * being watched by a server in this process abdicates.
 */
Event
Factory::handlePrecLeaderExistsChange(Notifyable *ntp,
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
     * Try to convert the passed Notifyable * to
     * a Group *
     */
    Group *grp = dynamic_cast<Group *>(ntp);
    if (grp == NULL) {
        LOG_WARN(CL_LOG,
                 "Conversion to Group * failed for %s",
                 key.c_str());
        return EN_NOEVENT;
    }

    /*
     * Make all interested Servers participate in the
     * election again.
     */
    Server *sp;
    Group *grp1;
    Node *np1;
    LeadershipElectionMultimap copy;
    LeadershipIterator leIt;
    LeadershipElectionMultimapRange range;

    {
        Locker l1(getLeadershipWatchesLock());

        /*
         * Make our own copy of the watches map.
         */
        copy = m_leadershipWatches;

        /*
         * And remove the key from the watches map.
         */
        range = m_leadershipWatches.equal_range(key);
        m_leadershipWatches.erase(range.first, range.second);
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

        np1 = sp->getMyNode();
        if (np1 == NULL) {
            LOG_FATAL(CL_LOG,
                      "Internal error: NULL node for server 0x%x",
                      (uint32_t) sp);
            ::abort();
        }

        grp1 = np1->getMyGroup();
        if (grp1 == NULL) {
            throw ClusterException(string("handlePrecLeaderChange: ") +
                                   "NULL group containing " +
                                   np1->getKey());
        }

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

    return EN_LEADERSHIPCHANGE;
}

/*
 * Special handler for synchronization that waits until the zk event
 * is propogated through the m_eventAdapter before returning.  This
 * ensures that all zk events prior to this point are visible by each
 * client.
 */
Event
Factory::handleSynchronizeChange(Notifyable *ntp,
                                 int32_t etype,
                                 const string &key)
{
    TRACE(CL_LOG, "handleSynchronizeChange");

    {
        AutoLock l1(m_syncLock);
        ++m_syncIdCompleted;
        m_syncCond.Signal();
    }

    LOG_DEBUG(CL_LOG,
              "handleSynchronizeChange: sent conditional signal");

    return EN_NOEVENT;
}

};	/* End of 'namespace clusterlib' */
