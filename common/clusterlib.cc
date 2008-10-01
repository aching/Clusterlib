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

namespace clusterlib
{

/*
 * All the string constants needed to construct and deconstruct
 * ZK keys.
 */
const string ClusterlibStrings::ROOTNODE = "/";
const string ClusterlibStrings::PATHSEPARATOR = "/";

const string ClusterlibStrings::CLUSTERLIB = "clusterlib";
const string ClusterlibStrings::VERSION = "1.0";

const string ClusterlibStrings::PROPERTIES = "properties";
const string ClusterlibStrings::CONFIGURATION = "configuration";
const string ClusterlibStrings::ALERTS = "alerts";

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

const string ClusterlibStrings::ELECTIONS = "elections";
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
 * Names associated with the special clusterlib master application.
 */
const string ClusterlibStrings::MASTER = "master";

/*
 * Constructor of Factory.
 *
 * Connect to cluster via the registry specification.
 * Initialize all the cache data structures.
 * Create the associated FactoryOps delegate object.
 */
Factory::Factory(const string &registry)
    : m_filledApplicationMap(false),
      m_config(registry, 3000),
      m_zk(m_config, this, false),
      m_timerEventAdapter(m_timerEventSrc),
      m_zkEventAdapter(m_zk),
      m_shutdown(false),
      m_connected(false)
{
    TRACE( CL_LOG, "Factory" );

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
        cerr << "Waiting for connect event from ZooKeeper" << endl;
        if (m_eventSyncLock.lockedWait(3000) == true) {
            LOG_ERROR(CL_LOG,
                      "Did not receive connect event in time, aborting");
        }
        cerr << "After wait, m_connected == " << m_connected << endl;
    } catch (zk::ZooKeeperException &e) {
        throw ClusterException(e.what());
    }
};

/*
 * Destructor of Factory
 */
Factory::~Factory()
{
    TRACE( CL_LOG, "~Factory" );

    delete mp_ops;
};


/*
 * Create a client.
 */
Client *
Factory::createClient()
{
    TRACE( CL_LOG, "createClient" );

    /*
     * Ensure we're connected.
     */
    if (!m_connected) {
        LOG_ERROR(CL_LOG,
                  "Cannot create client when disconnected.");
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
    TRACE( CL_LOG, "createServer" );

    /*
     * Ensure we're connected.
     */
    if (!m_connected) {
        LOG_ERROR(CL_LOG,
                  "Cannot create server when disconnected.");
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
        return NULL;
    }
};

/*
 * Handle events. Must be given to satisfy the interface but
 * not used.
 */
void
Factory::eventReceived(const zk::ZKEventSource &zksrc,
                       const zk::ZKWatcherEvent &e)
{
    TRACE( CL_LOG, "eventReceived" );

    /*
     * Do nothing with this event, the callback will enqueue
     * it to the event handling thread.
     */
};


/**********************************************************************/
/* Below this line are the private methods of class Factory.          */
/**********************************************************************/

/*
 * Add and remove clients.
 */
void
Factory::addClient(Client *clp)
{
    TRACE( CL_LOG, "addClient" );

    Locker l(&m_clLock);
    
    m_clients.push_back(clp);
}

void
Factory::removeClient(Client *clp)
{
    TRACE( CL_LOG, "removeClient" );

    Locker l(&m_clLock);
    ClientList::iterator i = find(m_clients.begin(),
                                  m_clients.end(),
                                  clp);

    if (i == m_clients.end()) {
        return;
    }
    m_clients.erase(i);
}

/*
 * Dispatch events to all registered clients.
 */
void
Factory::dispatchEvents()
{
    TRACE( CL_LOG, "dispatchEvents" );

    unsigned int eventSeqId = 0;
    bool sentEndEvent = false;
    GenericEvent ge;

#ifdef	VERY_VERY_VERBOSE
    cerr << "Hello from dispatchEvents, this: "
         << this
         << ", thread: "
         << self
         << endl;
#endif

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

#ifdef	VERY_VERY_VERBOSE
            cerr << "["
                 << eventSeqId
                 << ", "
                 << self
                 << "] dispatchEvents() received event of type: "
                 << ge.getType()
                 << endl;
#endif

            LOG_INFO(CL_LOG,
                     "[%d] dispatchEvent received event of type: %d",
                     eventSeqId, ge.getType());

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

#ifdef	VERY_VERY_VERBOSE
                      cerr << "Dispatching timer event: "
                           << tp
                           << ", id: "
                           << tp->getID()
                           << ", alarm time: "
                           << tp->getAlarmTime()
                           << endl;
#endif

                      dispatchTimerEvent(tp);

                      break;
                  }
              case ZKEVENT:
                  {
                      zk::ZKWatcherEvent *zp =
                          (zk::ZKWatcherEvent *) ge.getEvent();

                      LOG_INFO(CL_LOG,
                               "Processing ZK event "
                               "(type: %d, state: %d, context: 0x%x)",
                               zp->getType(),
                               zp->getState(),
                               (unsigned int) zp->getContext());

                      if (zp->getType() == SESSION_EVENT) {
                          dispatchSessionEvent(zp);
                      } else {
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
        LOG_ERROR( CL_LOG, "ZooKeeperException: %s", zke.what() );
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
        LOG_ERROR( CL_LOG, "Unknown exception: %s", stde.what() );
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
    TRACE( CL_LOG, "dispatchTimerEvent" );

#ifdef	VERY_VERY_VERBOSE    
    cerr << "Dispatching timer event" << endl;
#endif

    TimerEventPayload *tp = (TimerEventPayload *) te->getUserData();

    m_timerEventQueue.put(tp);
}

/*
 * Dispatch a ZK event.
 */
void
Factory::dispatchZKEvent(zk::ZKWatcherEvent *zp)
{
    TRACE( CL_LOG, "dispatchZKEvent" );
    
    ClientEventHandler *cp =
        (ClientEventHandler *) zp->getContext();
    ClusterEventPayload *cep, *cepp;
    ClientList::iterator i;

    /*
     * Protect against NULL context.
     */
    if (cp == NULL) {
        LOG_WARN(CL_LOG,
                 "Unexpected NULL context ZK event");
        return;
    }

    /*
     * Update the cache representation of the clusterlib
     * repository object and get back a prototypical
     * cluster event payload to send to clients.
     */
    cep = updateCachedObject(cp);

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
    TRACE( CL_LOG, "dispatchSessionEvent" );

    LOG_INFO(CL_LOG,
             "Session event: (type: %d, state: %d)",
             ze->getType(), ze->getState());

#ifdef	VERY_VERY_VERBOSE
    cerr << "Session event: "
         << "(type: " << ze->getType() 
         << ", state: " << ze->getState()
         << ")"
         << endl;
#endif

    if ((ze->getState() == ASSOCIATING_STATE) ||
        (ze->getState() == CONNECTING_STATE)) {
        /*
         * Not really clear what to do here.
         * For now do nothing.
         */
#ifdef	VERY_VERY_VERBOSE
        cerr << "Do nothing." << endl;
#endif
    } else if (ze->getState() == CONNECTED_STATE) {
        /*
         * Mark as connected.
         */
#ifdef	VERY_VERY_VERBOSE
        cerr << "Marked connected." << endl;
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
        cerr << "Giving up." << endl;
#endif
        m_shutdown = true;
        m_connected = false;

        /*
         * Notify anyone waiting that this factory is
         * now disconnected.
         */
        m_eventSyncLock.lockedNotify();
    } else {
#ifdef	VERY_VERY_VERBOSE
        cerr << "No idea what to do with state: " << ze->getState() << endl;
#endif
        LOG_WARN(CL_LOG,
                 "Session event with unknown state "
                 "(type: %d, state: %d)",
                 ze->getType(), ze->getState());
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
    TRACE( CL_LOG, "dispatchEndEvent" );

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
    TRACE( CL_LOG, "consumeTimerEvents" );

    TimerEventPayload *pp;

#ifdef	VERY_VERY_VERBOSE
    cerr << "Hello from consumeTimerEvents, this: "
         << this
         << ", thread: "
         << pthread_self()
         << endl;
#endif

    try {
        for (;;) {
            pp = m_timerEventQueue.take();

            /*
             * If we received the terminate signal,
             * then exit from the loop.
             */
            if (pp == NULL) {
                LOG_INFO( CL_LOG,
                          "Received terminate signal, finishing loop" );
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

            LOG_INFO( CL_LOG,
                      "Serviced timer %d, handler 0x%x, client data 0x%x",
                      pp->getId(), 
                      (int) pp->getHandler(),
                      (int) pp->getData() );

            /*
             * Deallocate the payload object.
             */
            delete pp;
        }
    } catch (zk::ZooKeeperException &zke) {
        LOG_ERROR( CL_LOG, "ZooKeeperException: %s", zke.what() );
        throw ClusterException(zke.what());
    } catch (ClusterException &ce) {
        throw ClusterException(ce.what());
    } catch (std::exception &stde) {
        LOG_ERROR( CL_LOG, "Unknown exception: %s", stde.what() );
        throw ClusterException(stde.what());
    }        
}

/*
 * Retrieve (and potentially create) instances of objects.
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
        return createApplication(key);
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
        return createDistribution(key, "", "", app);
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
        return createGroup(key, app);
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
        np = createNode(key, grp);
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
                   getGroup(appName, groupName, create),
                   managed,
                   create);
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
        return getApplication(components[3], create);
    }
    return NULL;
}
DataDistribution *
Factory::getDistributionFromKey(const string &key, bool create)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if (hasDistKeyPrefix(components)) {
        return getDistribution(components[5],
                               getApplication(components[3], false),
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
        return getGroup(components[5],
                        getApplication(components[3], false),
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
        return getNode(components[7],
                       getGroup(components[3],
                                components[5],
                                false),
                       (components[6] == "nodes") ? true : false,
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
        VERSION +
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
        ((components[6] != NODES) &&
         (components[6] != UNMANAGEDNODES))) {
        return false;
    }
    if (managedP != NULL) {
        *managedP = ((components[6] == NODES) ? true : false);
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
        components[7];
}

bool
Factory::isGroupKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 8) ||
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
    if ((components.size() < 8) ||
        (hasAppKeyPrefix(components) == false) ||
        (components[5] != GROUPS)) {
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
        components[5];
}

bool
Factory::isAppKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 4) ||
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
        (components[0] != CLUSTERLIB) ||
        (components[1] != VERSION) ||
        (components[2] != APPLICATIONS)) {
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
        VERSION +
        PATHSEPARATOR +
        APPLICATIONS +
        PATHSEPARATOR +
        components[3];
}

bool
Factory::isDistKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 6) ||
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
        (components[4] != DISTRIBUTIONS)) {
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
        components[5];
}

string
Factory::appNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 4) ||
        (hasAppKeyPrefix(components) == false)) {
        return "";
    }
    return components[3];
}
string
Factory::distNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 6) ||
        (hasDistKeyPrefix(components) == false)) {
        return "";
    }
    return components[5];
}
string
Factory::groupNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 6) ||
        (hasGroupKeyPrefix(components) == false)) {
        return "";
    }
    return components[5];
}
string
Factory::nodeNameFromKey(const string &key)
{
    vector<string> components;

    split(components, key, is_any_of(PATHSEPARATOR));
    if ((components.size() != 8) ||
        (hasDistKeyPrefix(components) == false)) {
        return "";
    }
    return components[7];
}

/*
 * Entity loading from ZooKeeper. Also add it to the
 * global cache.
 */
Application *
Factory::loadApplication(const string &name,
                         const string &key)
{
    TRACE( CL_LOG, "loadApplication");

    Application *app;
    {
        /*
         * Scope the lock to the shortest sequence possible.
         */
        Locker l(&m_appLock);

        app = m_applications[key];
        if (app != NULL) {
            return app;
        }
        if (m_zk.nodeExists(key, this, NULL) == false) {
            return NULL;
        }
        app = new Application(name, key, mp_ops);
        m_applications[key] = app;
    }

#ifdef	NOTDEF
    addInterests(key, app, EN_APP_INTERESTS);
    addInterests(key + PATHSEPARATOR + GROUPS,
                 app,
                 EN_GRP_CREATION | EN_GRP_DELETION);
    addInterests(key + PATHSEPARATOR + DISTRIBUTIONS,
                 app,
                 EN_DIST_CREATION | EN_DIST_DELETION);
#endif

    return app;
}
void
Factory::fillApplicationMap(ApplicationMap *amp)
{
    if (filledApplicationMap()) {
        return;
    }

    vector<string> children;
    vector<string>::iterator i;
    string appsKey =
        ROOTNODE +
        CLUSTERLIB +
        PATHSEPARATOR +
        VERSION +
        PATHSEPARATOR +
        APPLICATIONS;
    string appKey;

    /*
     * TBD -- lock the applications map!!!
     */
    m_zk.getNodeChildren(children, appsKey, this, (void *) EN_APP_INTERESTS);
    for (i = children.begin(); i != children.end(); i++) {
        appKey = appsKey + PATHSEPARATOR + *i;
        (*amp)[appKey] = getApplicationFromKey(appKey, false);
    }
    setFilledApplicationMap(true);
}

    
DataDistribution *
Factory::loadDistribution(const string &name,
                          const string &key,
                          Application *app)
{
    TRACE( CL_LOG, "loadDataDistribution" );

    DataDistribution *dist;

    {
        /*
         * Scope the lock to the shortest sequence possible.
         */
        Locker l(&m_ddLock);

        dist = m_dataDistributions[key];
        if (dist != NULL) {
            return dist;
        }
        if (m_zk.nodeExists(key, this, NULL) == false) {
            return NULL;
        }
        dist = new DataDistribution(app, name, key, mp_ops);
        m_dataDistributions[key] = dist;
    }

#ifdef	NOTDEF
    addInterests(key, dist, EN_DIST_INTERESTS);
    addInterests(key + PATHSEPARATOR + SHARDS,
                 dist,
                 EN_DIST_CHANGE);
    addInterests(key + PATHSEPARATOR + MANUALOVERRIDES,
                 dist,
                 EN_DIST_CHANGE);
#endif

    return dist;
}
void
Factory::fillDataDistributionMap(DataDistributionMap *dmp,
                                 Application *app)
{
    if (app->filledDataDistributionMap()) {
        return;
    }

    vector<string> children;
    vector<string>::iterator i;
    string distsKey =
        ROOTNODE +
        CLUSTERLIB +
        PATHSEPARATOR +
        VERSION +
        PATHSEPARATOR +
        app->getName() +
        PATHSEPARATOR +
        DISTRIBUTIONS;
    string distKey;

    m_zk.getNodeChildren(children, distsKey, this, (void *) EN_DIST_INTERESTS);
    Locker l(app->getDistributionMapLock());
    for (i = children.begin(); i != children.end(); i++) {
        distKey = distsKey + PATHSEPARATOR + *i;
        (*dmp)[distKey] = getDistribution(*i, app, false);
    }
    app->setFilledDataDistributionMap(true);
}

string
Factory::loadShards(const string &key)
{
    string snode =
        key +
        PATHSEPARATOR +
        SHARDS;
    return m_zk.getNodeData(snode,
                            this,
                            (void *) EN_DIST_CHANGE);
}
string
Factory::loadManualOverrides(const string &key)
{
    string monode =
        key +
        PATHSEPARATOR +
        MANUALOVERRIDES;
    return m_zk.getNodeData(monode,
                            this,
                            (void *) EN_DIST_CHANGE);
}

Group *
Factory::loadGroup(const string &name,
                   const string &key,
                   Application *app)
{
    TRACE( CL_LOG, "loadGroup" );

    Group *grp;

    {
        /*
         * Scope the lock to the shortest sequence possible.
         */
        Locker l(&m_grpLock);

        grp = m_groups[key];
        if (grp != NULL) {
            return grp;
        }
        if (m_zk.nodeExists(key, this, NULL) == false) {
            return NULL;
        }
        grp = new Group(app, name, key, mp_ops);
        m_groups[key] = grp;
    }

#ifdef	NOTDEF
    addInterests(key, grp, EN_GRP_INTERESTS);
    addInterests(key + PATHSEPARATOR + NODES,
                 grp,
                 EN_GRP_MEMBERSHIP);
#endif

    return NULL;
}
void
Factory::fillGroupMap(GroupMap *gmp, Application *app)
{
    if (app->filledGroupMap()) {
        return;
    }

    vector<string> children;
    vector<string>::iterator i;
    string groupsKey =
        app->getKey() +
        PATHSEPARATOR +
        GROUPS;
    string groupKey;

    m_zk.getNodeChildren(children, groupsKey, this, (void *) EN_GRP_INTERESTS);
    Locker l(app->getGroupMapLock());
    for (i = children.begin(); i != children.end(); i++) {
        groupKey = groupsKey + PATHSEPARATOR + *i;
        (*gmp)[groupKey] = getGroup(*i, app, false);
    }
    app->setFilledGroupMap(true);
}

Node *
Factory::loadNode(const string &name,
                  const string &key,
                  Group *grp)
{
    TRACE( CL_LOG, "loadNode" );

    Node *node;

    {
        /*
         * Scope the lock to the shortest sequence possible.
         */
        Locker l(&m_nodeLock);

        node = m_nodes[key];
        if (node != NULL) {
            return node;
        }
        if (m_zk.nodeExists(key, this, NULL) == false) {
            return NULL;
        }
        node = new Node(grp, name, key, mp_ops);
        m_nodes[key] = node;
    }

#ifdef	NOTDEF
    addInterests(key, node, EN_NODE_INTERESTS);
    addInterests(key + PATHSEPARATOR + CLIENTSTATE,
                 node,
                 EN_NODE_HEALTHCHANGE);
    addInterests(key + PATHSEPARATOR + CONNECTED,
                 node,
                 EN_NODE_CONNECTCHANGE);
    addInterests(key + PATHSEPARATOR + MASTERSETSTATE,
                 node,
                 EN_NODE_MASTERSTATECHANGE);
#endif

    return node;
}
void
Factory::fillNodeMap(NodeMap *nmp, Group *grp)
{
    if (grp->filledNodeMap()) {
        return;
    }

    vector<string> children;
    vector<string>::iterator i;
    string nodesKey =
        grp->getKey() +
        PATHSEPARATOR +
        NODES;
    string nodeKey;

    m_zk.getNodeChildren(children, nodesKey, this, (void *) EN_NODE_INTERESTS);
    Locker l(grp->getNodeMapLock());
    for (i = children.begin(); i != children.end(); i++) {
        nodeKey = nodesKey + PATHSEPARATOR + *i;
        (*nmp)[nodeKey] = getNode(*i, grp, true, false);
    }
    grp->setFilledNodeMap(true);
}

/*
 * Entity creation in ZooKeeper.
 */
Application *
Factory::createApplication(const string &key)
{
    TRACE( CL_LOG, "createApplication" );

    return NULL;
}

DataDistribution *
Factory::createDistribution(const string &key,
                            const string &marshalledShards,
                            const string &marshalledManualOverrides,
                            Application *app)
{
    TRACE( CL_LOG, "createDataDistribution" );

    return NULL;
}

Group *
Factory::createGroup(const string &key, Application *app)
{
    TRACE( CL_LOG, "createGroup" );

    return NULL;
}

Node *
Factory::createNode(const string &key, Group *grp)
{
    TRACE( CL_LOG, "createNode" );

    return NULL;
}

/*
 * Register a timer handler.
 */
TimerId
Factory::registerTimer(TimerEventHandler *handler,
                       uint64_t afterTime,
                       ClientData data)
    throw(ClusterException)
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
    throw(ClusterException)
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
Factory::updateCachedObject(ClientEventHandler *cp)
    throw(ClusterException)
{
    /*
     * TO BE WRITTEN
     */
    return NULL;
}

};	/* End of 'namespace clusterlib' */
