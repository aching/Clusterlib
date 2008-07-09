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
 * Connect to ZooKeeper via the registry specification.
 * Initialize all the cache data structures.
 * Create the associated FactoryOps delegate object.
 */
Factory::Factory(const string &registry)
{
    TRACE( CL_LOG, "Factory" );

    m_dataDistributions.clear();
    m_applications.clear();
    m_groups.clear();
    m_nodes.clear();
    m_notificationInterests.clear();

    ZooKeeperConfig cfg(registry, 3000);
    mp_zk = new ZooKeeperAdapter(cfg, this, true);
};

/*
 * Destructor of Factory
 */
Factory::~Factory()
{
    TRACE( CL_LOG, "~Factory" );
};

/*
 * Create a client.
 */
ClusterClient *
Factory::createClient()
{
    TRACE( CL_LOG, "createClient" );

    return NULL;
};

/*
 * Create a server.
 */
ClusterServer *
Factory::createServer(const string &app,
                      const string &group,
                      const string &node,
                      HealthChecker *checker,
                      bool createReg)
{
    TRACE( CL_LOG, "createServer" );

    return NULL;
};

/*
 * Handle events.
 */
void
Factory::eventReceived(const ZKEventSource zksrc, const ZKWatcherEvent &e)
{
    TRACE( CL_LOG, "eventReceived" );
};

/**********************************************************************/
/* Below this line are the private methods of class Factory.          */
/**********************************************************************/

/*
 * Manage interests in events.
 */
void
Factory::addInterests(Notifyable *nrp, const Event events)
{
    TRACE( CL_LOG, "addInterests" );

    Locker l(&m_notificationLock);
    InterestRecord *r = m_notificationInterests[nrp->getKey()];

    if (r == NULL) {
        r = new InterestRecord(nrp, events);
        m_notificationInterests[nrp->getKey()] = r;
    }
    r->addInterests(events);
};

void
Factory::removeInterests(Notifyable *nrp, const Event events)
{
    TRACE( CL_LOG, "removeInterests" );

    Locker l(&m_notificationLock);
    InterestRecord *r = m_notificationInterests[nrp->getKey()];

    if (r != NULL) {
        r->removeInterests(events);
        if (r->getInterests() == 0) {
            m_notificationInterests.erase(nrp->getKey());
            delete r;
        }
    }
};

/*
 * Retrieve (and potentially create) instances of objects.
 */
Application *
Factory::getApplication(const string &name)
{
    string key = createAppKey(name);
    Locker l(&m_appLock);
    Application *app = m_applications[key];

    if (app != NULL) {
        return app;
    }
    app = loadApplication(key);
    /*
     * TODO: Check for null app.
     */
    m_applications[key] = app;

    return app;
}

Group *
Factory::getGroup(const string &groupName,
                  Application *app)
{
    string key = createGroupKey(app->getName(), groupName);
    Locker l(&m_grpLock);
    Group *grp = m_groups[key];

    if (grp != NULL) {
        return grp;
    }
    grp = loadGroup(key, app);
    m_groups[key] = grp;

    return grp;
}
Group *
Factory::getGroup(const string &appName,
                  const string &groupName)
{
    string key = createGroupKey(appName, groupName);
    Locker l(&m_grpLock);
    Group *grp = m_groups[key];

    if (grp != NULL) {
        return grp;
    }

    Application *app = getApplication(appName);
    /*
     * TODO: Check for null app.
     */
    grp = loadGroup(key, app);
    m_groups[key] = grp;

    return grp;
}

Node *
Factory::getNode(const string &nodeName, Group *grp, bool managed)
{
    string key = createNodeKey(grp->getApplication()->getName(),
                               grp->getName(),
                               nodeName,
                               managed);
    Locker l(&m_nodeLock);
    Node *np = m_nodes[key];

    if (np != NULL) {
        return np;
    }
    np = loadNode(key, grp);
    /*
     * TODO: Check for null np.
     */
    m_nodes[key] = np;

    return np;
}
Node *
Factory::getNode(const string &appName,
                 const string &groupName,
                 const string &nodeName,
                 bool managed)
{
    string key = createNodeKey(appName, groupName, nodeName, managed);
    Locker l(&m_nodeLock);
    Node *np = m_nodes[key];

    if (np != NULL) {
        return np;
    }
    Group *grp = getGroup(appName, groupName);
    /*
     * TODO: Check for null grp.
     */
    np = loadNode(key, grp);
    /*
     * TODO: Check for null np.
     */
    m_nodes[key] = np;

    return np;
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

};	/* End of 'namespace clusterlib' */
