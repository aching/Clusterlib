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
Factory::getGroup(const string &appName, const string &groupName)
{
    string key = createGroupKey(appName, groupName);
    Locker l(&m_grpLock);
    Group *grp = m_groups[key];

    if (grp != NULL) {
        return grp;
    }

    Application *app = getApplication(appName);
    grp = loadGroup(key, app);
    m_groups[key] = grp;

    return grp;
}

Node *
Factory::getNode(const string &nodeName, Group *grp)
{
    string key = createNodeKey(grp->getApplication()->getName(),
                               grp->getName(),
                               nodeName,
                               true);
    Locker l(&m_nodeLock);
    Node *np = m_nodes[key];

    if (np != NULL) {
        return np;
    }
    np = loadNode(key, grp);
    m_nodes[key] = np;

    return np;
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
    string res = "";

    return res;
}

string
Factory::createGroupKey(const string &appname,
                        const string &groupName)
{
    string res = "";

    return res;
}

string
Factory::createAppKey(const string &appName)
{
    string res = "";

    return res;
}

string
Factory::createDistKey(const string &appName,
                       const string &distName)
{
    string res = "";

    return res;
}

bool
Factory::isNodeKey(const string &key, bool *managedP)
{
    return true;
}

bool
Factory::isGroupKey(const string &key)
{
    return true;
}

bool
Factory::isAppKey(const string &key)
{
    return true;
}

bool
Factory::isDistKey(const string &key)
{
    return true;
}

};	/* End of 'namespace clusterlib' */
