/*
 * clusterlib.cc --
 *
 * Implementation of the Factory class.
 *
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
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
      m_zkEventAdapter(m_zk)
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
    m_notificationInterests.clear();

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
     * Connect to ZK (TBD).
     */
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
ClusterClient *
Factory::createClient()
{
    TRACE( CL_LOG, "createClient" );

    return new ClusterClient(mp_ops);
};

/*
 * Create a server.
 */
ClusterServer *
Factory::createServer(const string &app,
                      const string &group,
                      const string &node,
                      HealthChecker *checker,
                      ServerFlags flags)
{
    TRACE( CL_LOG, "createServer" );

    return new ClusterServer(mp_ops,
                             app,
                             group,
                             node,
                             checker,
                             flags);
};

/*
 * Handle events.
 */
void
Factory::eventReceived(const ZKEventSource &zksrc, const ZKWatcherEvent &e)
{
    TRACE( CL_LOG, "eventReceived" );
};

/**********************************************************************/
/* Below this line are the private methods of class Factory.          */
/**********************************************************************/

/*
 * Add and remove clients.
 */
void
Factory::addClient(ClusterClient *clp)
{
    TRACE( CL_LOG, "addClient" );

    Locker l(&m_clLock);
    
    m_clients.push_back(clp);
}

void
Factory::removeClient(ClusterClient *clp)
{
    TRACE( CL_LOG, "removeClient" );

    Locker l(&m_clLock);
    ClusterClientList::iterator i = find(m_clients.begin(),
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
    
    try {
        for (m_shutdown = false;
             m_shutdown == false;
             ) 
        {
            GenericEvent ge;

            LOG_INFO(CL_LOG,
                     "[%d]: Asking for next event",
                     eventSeqId);

            /*
             * Get the next event and send it off to the
             * correct handler.
             */
            ge = m_eventAdapter.getNextEvent();
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

                      dispatchTimerEvent(tp);
                      
                      LOG_INFO(CL_LOG,
                               "Processed timer event %d with data 0x%x "
                               "triggered at %lld",
                               tp->getID(),
                               (unsigned int) tp->getUserData(),
                               tp->getAlarmTime());

                      delete tp;

                      break;
                  }
              case ZKEVENT:
                  {
                      ZKWatcherEvent *zp =
                          (ZKWatcherEvent *) ge.getEvent();

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
        dispatchEndEvent();
    } catch (ZooKeeperException &zke) {
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
 * Dispatch a timer event.
 */
void
Factory::dispatchTimerEvent(ClusterlibTimerEvent *te)
{
    TRACE( CL_LOG, "dispatchTimerEvent" );
    
    TimerPayload *tp = (TimerPayload *) te->getUserData();

    LOG_INFO(CL_LOG,
             "Got timer event %d with data 0x%x "
             "triggered at %lld",
             te->getID(),
             (unsigned int) tp,
             te->getAlarmTime());

    /*
     * Protect against NULL.
     */
    if (tp == NULL) {
        LOG_WARN(CL_LOG,
                 "Unexpected NULL timer event %d triggered "
                 "at %lld",
                 te->getID(),
                 te->getAlarmTime());
        return;
    }

    /*
     * If cancelled then no need to deliver.
     */
    if (tp->cancelled()) {
        delete tp;
        return;
    }

    /*
     * Deliver the event.
     */
    tp->getClient()->sendEvent(tp);
}

/*
 * Dispatch a ZK event.
 */
void
Factory::dispatchZKEvent(ZKWatcherEvent *zp)
{
    TRACE( CL_LOG, "dispatchZKEvent" );

    /***********************************************************************/
    /* NEEDS TO BE REIMPLEMENTED COMPLETELY!!!!                            */
    /***********************************************************************/
    
    Payload *cp = (Payload *) zp->getContext();

    if (cp == NULL) {
        LOG_WARN(CL_LOG,
                 "Unexpected NULL context ZK event");
        return;
    }

    cp->getClient()->sendEvent(cp);
}

/*
 * Dispatch a session event. These events
 * are in fact handled directly, here.
 */
void
Factory::dispatchSessionEvent(ZKWatcherEvent *ze)
{
    TRACE( CL_LOG, "dispatchSessionEvent" );

    LOG_INFO(CL_LOG,
             "Session event: (type: %d, state: %d)",
             ze->getType(), ze->getState());

    if ((ze->getState() == ASSOCIATING_STATE) ||
        (ze->getState() == CONNECTING_STATE)) {
        /*
         * Not really clear what to do here.
         * For now do nothing.
         */
    } else if (ze->getState() == CONNECTED_STATE) {
        /*
         * Rerun leadership protocol for all
         * elections we participate in.
         * (TBD)
         */
    } else if (ze->getState() == EXPIRED_SESSION_STATE) {
        /*
         * We give up on SESSION_EXPIRED.
         */
        m_shutdown = true;
    } else {
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
void
Factory::dispatchEndEvent()
{
    TRACE( CL_LOG, "dispatchEndEvent" );

    /*
     * TO BE IMPLEMENTED.
     */
}

/*
 * Retrieve (and potentially create) instances of objects.
 */
Application *
Factory::getApplication(const string &name)
{
    string key = createAppKey(name);

    {
        /*
         * Scope the lock to the smallest extent
         * possible.
         */
        Locker l(&m_appLock);
        Application *app = m_applications[key];
        if (app != NULL) {
            return app;
        }
    }
    return loadApplication(name, key);
}

DataDistribution *
Factory::getDistribution(const string &distName,
                         Application *app)
{
    string key = createDistKey(app->getName(), distName);

    {
        /*
         * Scope the lock to the smallest extent possible.
         */
        Locker l(&m_ddLock);
        DataDistribution *dist = m_dataDistributions[key];
        if (dist != NULL) {
            return dist;
        }
    }
    return loadDistribution(distName, key, app);
}

Group *
Factory::getGroup(const string &groupName,
                  Application *app)
{
    string key = createGroupKey(app->getName(), groupName);

    {
        /*
         * Scope the lock to the smallest extent possible.
         */
        Locker l(&m_grpLock);
        Group *grp = m_groups[key];
        if (grp != NULL) {
            return grp;
        }
    }
    return loadGroup(groupName, key, app);
}
Group *
Factory::getGroup(const string &appName,
                  const string &groupName)
{
    Application *app = getApplication(appName);

    return getGroup(groupName, app);
}

Node *
Factory::getNode(const string &nodeName, Group *grp, bool managed)
{
    string key = createNodeKey(grp->getApplication()->getName(),
                               grp->getName(),
                               nodeName,
                               managed);

    {
        /*
         * Scope the lock to the smallest extent
         * possible.
         */
        Locker l(&m_nodeLock);
        Node *np = m_nodes[key];
        if (np != NULL) {
            return np;
        }
    }
    return loadNode(nodeName, key, grp);
}
Node *
Factory::getNode(const string &appName,
                 const string &groupName,
                 const string &nodeName,
                 bool managed)
{
    Group *grp = getGroup(appName, groupName);

    return getNode(nodeName, grp, managed);
}

/*
 * Return an object representing a shard.
 */
Shard *
Factory::createShard(DataDistribution *dp,
                     const string &startRange,
                     const string &endRange,
                     const string &appName,
                     const string &grpName,
                     const string &nodeName)
{
    Node *np = getNode(appName, grpName, nodeName, true);
    unsigned long long lo = atoll(startRange.c_str());
    unsigned long long hi = atoll(endRange.c_str());

    /*
     * TODO: Check np for NULL.
     */

    return new Shard(dp, np, lo, hi);
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
    if ((components.size() < 9) ||
        (hasGroupKeyPrefix(components) == false) ||
        ((components[7] != NODES) &&
         (components[7] != UNMANAGEDNODES))) {
        return false;
    }
    if (managedP != NULL) {
        *managedP = ((components[7] == NODES) ? true : false);
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
        components[7] +
        PATHSEPARATOR +
        components[8];
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
        components[6];
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
    if ((components.size() < 5) ||
        (components[1] != CLUSTERLIB) ||
        (components[2] != VERSION) ||
        (components[3] != APPLICATIONS)) {
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
        components[1] +
        PATHSEPARATOR +
        components[2] +
        PATHSEPARATOR +
        components[3] +
        PATHSEPARATOR +
        components[4];
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
    if ((components.size() < 7) ||
        (hasAppKeyPrefix(components) == false) ||
        (components[5] != DISTRIBUTIONS)) {
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
        components[6];
}

/*
 * Entity loading from ZooKeeper. Also add it to the
 * global cache.
 */
Application *
Factory::loadApplication(const string &name, const string &key)
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
        if (m_zk.nodeExists(key, this, (void *) EN_APP_INTERESTS) == 
            false) {
            return NULL;
        }
        app = new Application(name, key, mp_ops);
        m_applications[key] = app;
    }

#ifdef	REWRITE_FOR_NOTIFICATION_RECEIVER
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
    Application *app;

    m_zk.getNodeChildren(children, appsKey, this, (void *) EN_APP_INTERESTS);
    for (i = children.begin(); i != children.end(); i++) {
        app = getApplication(*i);
        if (app == NULL) {
            appKey = appsKey + PATHSEPARATOR + *i;
            app = loadApplication(*i, appKey);
            (*amp)[appKey] = app;
        }
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
        if (m_zk.nodeExists(key, this, (void *) EN_DIST_INTERESTS) 
            == false) {
            return NULL;
        }
        dist = new DataDistribution(app, name, key, mp_ops);
        m_dataDistributions[key] = dist;
    }

#ifdef	REWRITE_FOR_NOTIFICATION_RECEIVER
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
    DataDistribution *dist;

    m_zk.getNodeChildren(children, distsKey, this, (void *) EN_DIST_INTERESTS);
    Locker l(app->getDistributionMapLock());
    for (i = children.begin(); i != children.end(); i++) {
        dist = getDistribution(*i, app);
        if (dist == NULL) {
            distKey = distsKey + PATHSEPARATOR + *i;
            dist = loadDistribution(*i, distKey, app);
            (*dmp)[distKey] = dist;
        }
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
        if (m_zk.nodeExists(key, this, (void *) EN_GRP_INTERESTS)
            == false) {
            return NULL;
        }
        grp = new Group(app, name, key, mp_ops);
        m_groups[key] = grp;
    }

#ifdef	REWRITE_FOR_NOTIFICATION_RECEIVER
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
    Group *grp;

    m_zk.getNodeChildren(children, groupsKey, this, (void *) EN_GRP_INTERESTS);
    Locker l(app->getGroupMapLock());
    for (i = children.begin(); i != children.end(); i++) {
        grp = getGroup(*i, app);
        if (grp == NULL) {
            groupKey = groupsKey + PATHSEPARATOR + *i;
            grp = loadGroup(*i, groupKey, app);
            (*gmp)[groupKey] = grp;
        }
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
        if (m_zk.nodeExists(key, this, (void *) EN_NODE_INTERESTS)
            == false) {
            return NULL;
        }
        node = new Node(grp, name, key, mp_ops);
        m_nodes[key] = node;
    }

#ifdef	REWRITE_FOR_NOTIFICATION_RECEIVER
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
    Node *node;
    bool managed;

    m_zk.getNodeChildren(children, nodesKey, this, (void *) EN_NODE_INTERESTS);
    Locker l(grp->getNodeMapLock());
    for (i = children.begin(); i != children.end(); i++) {
        node = getNode(*i, grp, &managed);
        if (node == NULL) {
            nodeKey = nodesKey + PATHSEPARATOR + *i;
            node = loadNode(*i, nodeKey, grp);
            (*nmp)[nodeKey] = node;
        }
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

};	/* End of 'namespace clusterlib' */
