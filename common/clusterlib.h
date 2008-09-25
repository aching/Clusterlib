/*
 * clusterlib.h --
 *
 * The main include file for users of clusterlib.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERLIB_H_
#define	_CLUSTERLIB_H_

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;

#include "forwarddecls.h"

#include "log.h"
#include "clusterexception.h"
#include "blockingqueue.h"
#include "mutex.h"
#include "thread.h"
#include "event.h"
#include "command.h"
#include "zkadapter.h"

// using namespace zk;

#include "healthchecker.h"
#include "notifyable.h"
#include "clusterclient.h"
#include "clusterserver.h"
#include "application.h"
#include "group.h"
#include "node.h"
#include "datadistribution.h"

DEFINE_LOGGER( CL_LOG, "clusterlib" )

namespace clusterlib
{

/*
 * Typedefs for the various event adapter types.
 */
typedef EventListenerAdapter<ClusterlibTimerEvent, TIMEREVENT>
    ClusterlibTimerEventAdapter;
typedef EventListenerAdapter<zk::ZKWatcherEvent, ZKEVENT>
    ZooKeeperEventAdapter;

/**
 * Class containing static variables for all string constants.
 */
class ClusterlibStrings
{
  public:
    /*
     * All string constants used to name ZK nodes.
     */
    static const string ROOTNODE;
    static const string PATHSEPARATOR;

    static const string CLUSTERLIB;
    static const string VERSION;

    static const string PROPERTIES;
    static const string CONFIGURATION;
    static const string ALERTS;

    static const string APPLICATIONS;
    static const string GROUPS;
    static const string NODES;
    static const string UNMANAGEDNODES;
    static const string DISTRIBUTIONS;

    static const string CLIENTSTATE;
    static const string CLIENTSTATEDESC;
    static const string ADDRESS;
    static const string LASTCONNECTED;
    static const string CLIENTVERSION;
    static const string CONNECTED;
    static const string BOUNCY;
    static const string READY;
    static const string ALIVE;
    static const string MASTERSETSTATE;
    static const string SUPPORTEDVERSIONS;

    static const string ELECTIONS;
    static const string BIDS;
    static const string CURRENTLEADER;

    static const string SHARDS;
    static const string GOLDENSHARDS;
    static const string MANUALOVERRIDES;

    static const string LOCKS;
    static const string QUEUES;
    static const string BARRIERS;
    static const string TRANSACTIONS;

    /*
     * Strings used as values of ZK nodes (or parts of values).
     */
    static const string BIDPREFIX;
    static const string INFLUX;
    static const string HEALTHY;
    static const string UNHEALTHY;

    /*
     * Names associated with the special clusterlib master
     * application.
     */
    static const string MASTER;

    /*
     * Default constructor.
     */
    ClusterlibStrings() {}
};

/*
 * The actual factory class.
 */
class Factory
    : public virtual ClusterlibStrings,
      public virtual zk::ZKEventListener
{
  public:
    /*
     * Create a factory instance, connect it to
     * the specified cluster registry.
     */
    Factory(const string &registry);

    /*
     * Destructor.
     */
    virtual ~Factory();

    /*
     * Create a cluster client object.
     */
    Client *createClient();

    /*
     * Create a cluster server object. Also
     * create the needed registration if createReg
     * is set to true.
     */
    Server *createServer(const string &app,
                         const string &group,
                         const string &node,
                         HealthChecker *checker,
                         ServerFlags flags);

    /*
     * Convenience function -- return the current time in ms
     * from the unix epoch.
     */
    static int64_t getCurrentTimeMillis()
    {
        return Timer<int>::getCurrentTimeMillis();
    }

    /*
     * Is the factory connected to ZooKeeper?
     */
    bool isConnected() { return m_connected; }

    /*
     * Handle events received from ZooKeeper. Must provide
     * and inherit from ZKEventListener so that Factory can
     * be used as an event sink. Not used, just a place holder
     * and the real event processing happens in the callback.
     */
    void eventReceived(const zk::ZKEventSource &source,
                       const zk::ZKWatcherEvent &event);

  private:
    /*
     * Friend declaration for FactoryOps so it'll have
     * access to all these private operations.
     */
    friend class FactoryOps;

    /*
     * Add and remove clients.
     */
    void addClient(Client *clp);
    void removeClient(Client *clp);

    /*
     * Register/cancel a timer handler.
     */
    TimerId registerTimer(TimerEventHandler *handler,
                          uint64_t afterTime,
                          ClientData data)
        throw(ClusterException);
    bool cancelTimer(TimerId id)
        throw(ClusterException);
    void forgetTimer(TimerId id);
    
    /*
     * Dispatch all events. Reads from the
     * event sources and sends events to
     * the registered client for each event.
     */
    void dispatchEvents();

    /*
     * Dispatch timer, zk, and session events.
     */
    void dispatchTimerEvent(ClusterlibTimerEvent *te);
    void dispatchZKEvent(zk::ZKWatcherEvent *ze);
    void dispatchSessionEvent(zk::ZKWatcherEvent *ze);
    bool dispatchEndEvent();

    /*
     * Helper method that updates the cached representation
     * of a clusterlib repository object and generates the
     * prototypical cluster event payload to send to clients.
     */
    ClusterEventPayload *updateCachedObject(ClientEventHandler *cp)
        throw(ClusterException);

    /*
     * This method consumes timer events. It runs in a separate
     * thread.
     */
    void consumeTimerEvents();

    /*
     * Retrieve a list of all (currently known) applications.
     */
    IdList getApplicationNames();

    /*
     * Retrieve (and potentially create) instances of
     * objects representing applications, groups, nodes,
     * and distributions.
     */
    Application *getApplication(const string &name,
                                bool create);

    Group *getGroup(const string &name,
                    Application *app,
                    bool create);
    Group *getGroup(const string &appName,
                    const string &grpName,
                    bool create);

    Node *getNode(const string &name,
                  Group *grp,
                  bool managed,
                  bool create);
    Node *getNode(const string &appName,
                  const string &grpName,
                  const string &nodeName,
                  bool managed,
                  bool create);

    DataDistribution *getDistribution(const string &name,
                                      Application *app,
                                      bool create);
    DataDistribution *getDistribution(const string &appName,
                                      const string &distName,
                                      bool create);

    string createNodeKey(const string &appName,
                         const string &groupName,
                         const string &nodeName,
                         bool managed);
    string createGroupKey(const string &appName,
                          const string &groupName);
    string createAppKey(const string &appName);
    string createDistKey(const string &appName,
                         const string &distName);

    bool isNodeKey(const string &key, bool *managedP = NULL);
    bool hasNodeKeyPrefix(const string &key, bool *managedP = NULL);
    bool hasNodeKeyPrefix(vector<string> &components, bool *managedP = NULL);
    string getNodeKeyPrefix(const string &key, bool *managedP = NULL);
    string getNodeKeyPrefix(vector<string> &components);

    bool isGroupKey(const string &key);
    bool hasGroupKeyPrefix(const string &key);
    bool hasGroupKeyPrefix(vector<string> &components);
    string getGroupKeyPrefix(const string &key);
    string getGroupKeyPrefix(vector<string> &components);

    bool isAppKey(const string &key);
    bool hasAppKeyPrefix(const string &key);
    bool hasAppKeyPrefix(vector<string> &components);
    string getAppKeyPrefix(const string &key);
    string getAppKeyPrefix(vector<string> &components);

    bool isDistKey(const string &key);
    bool hasDistKeyPrefix(const string &key);
    bool hasDistKeyPrefix(vector<string> &components);
    string getDistKeyPrefix(const string &key);
    string getDistKeyPrefix(vector<string> &components);

    /*
     * Load entities from ZooKeeper.
     */
    Application *loadApplication(const string &name,
                                 const string &key);
    DataDistribution *loadDistribution(const string &name,
                                       const string &key,
                                       Application *app);
    Group *loadGroup(const string &name,
                     const string &key,
                     Application *app);
    Node *loadNode(const string &name,
                   const string &key,
                   Group *grp);

    void fillApplicationMap(ApplicationMap *amp);
    void fillGroupMap(GroupMap *gmp, Application *app);
    void fillNodeMap(NodeMap *nmp, Group *grp);
    void fillDataDistributionMap(DataDistributionMap *dmp,
                                 Application *app);

    string loadShards(const string &key);
    string loadManualOverrides(const string &key);

    /*
     * Did we already fill the application map?
     */
    bool filledApplicationMap() { return m_filledApplicationMap; }
    void setFilledApplicationMap(bool v) { m_filledApplicationMap = v; }

    /*
     * Create entities in ZooKeeper.
     */
    Application *createApplication(const string &key);
    DataDistribution *createDistribution(
	const string &key,
        const string &marshalledShards,
        const string &marshalledManualOverrides,
        Application *app);
    Group *createGroup(const string &key, Application *app);
    Node *createNode(const string &key, Group *grp);

  private:

    /*
     * The factory ops delegator.
     */
    FactoryOps *mp_ops;

    /*
     * The registry of attached clients (and servers).
     */
    ClientList m_clients;
    Mutex m_clLock;

    /*
     * The registry of cached data distributions.
     */
    DataDistributionMap m_dataDistributions;
    Mutex m_ddLock;

    /*
     * The registry of cached applications.
     */
    ApplicationMap m_applications;
    Mutex m_appLock;

    /*
     * The registry of cached groups.
     */
    GroupMap m_groups;
    Mutex m_grpLock;

    /*
     * The registry of cached nodes.
     */
    NodeMap m_nodes;
    Mutex m_nodeLock;

    /*
     * Did we already fill the application map?
     */
    bool m_filledApplicationMap;

    /*
     * The registry of timer handlers.
     */
    TimerRegistry m_timerRegistry;
    Mutex m_timerRegistryLock;

    /*
     * The ZooKeeper config object.
     */
    zk::ZooKeeperConfig m_config;

    /*
     * The ZooKeeper adapter object being used.
     */
    zk::ZooKeeperAdapter m_zk;

    /*
     * The timer event source.
     */
    ClusterlibTimerEventSource m_timerEventSrc;

    /**
     * The timer source adapter.
     */
    ClusterlibTimerEventAdapter m_timerEventAdapter;

    /**
     * The queue of timer events.
     */
    TimerEventQueue m_timerEventQueue;

    /**
     * The timer event handler thread.
     */
    CXXThread<Factory> m_timerHandlerThread;

    /**
     * The Zookeeper source adapter.
     */
    ZooKeeperEventAdapter m_zkEventAdapter;

    /*
     * Synchronous event adapter.
     */
    SynchronousEventAdapter<GenericEvent> m_eventAdapter;

    /*
     * The thread running the synchronous event adapter.
     */
    CXXThread<Factory> m_eventThread;

    /*
     * Is the event loop terminating?
     */
    bool m_shutdown;

    /*
     * Is the factory connected to ZooKeeper?
     */
    volatile bool m_connected;

    /*
     * Lock for event synchronization.
     */
    Lock m_eventSyncLock;
};

/*
 * Definition and implementation of class FactoryOps. An
 * instance of this class is given to internal objects so
 * that these objects can call operations on the factory
 * that shouldn't be generally available.
 */
class FactoryOps
{
  public:
    void addClient(Client *clp)
    {
        mp_f->addClient(clp);
    }
    void removeClient(Client *clp)
    {
        mp_f->removeClient(clp);
    }

    TimerId registerTimer(TimerEventHandler *handler,
                          uint64_t afterTime,
                          ClientData data)
        throw(ClusterException)
    {
        return mp_f->registerTimer(handler, afterTime, data);
    }
    bool cancelTimer(TimerId id)
        throw(ClusterException)
    {
        return mp_f->cancelTimer(id);
    }

    Application *getApplication(const string &name,
                                bool create = false)
    {
        return mp_f->getApplication(name, create);
    }
    void fillApplicationMap(ApplicationMap *amp)
    {
        if (mp_f->filledApplicationMap() == false) {
            mp_f->fillApplicationMap(amp);
        }
    }

    Group *getGroup(const string &name,
                    Application *app,
                    bool create = false)
    {
        return mp_f->getGroup(name, app, create);
    }
    Group *getGroup(const string &appName,
                    const string &grpName,
                    bool create = false)
    {
        return mp_f->getGroup(appName, grpName, create);
    }
    void fillGroupMap(GroupMap *gmp, Application *app)
    {
        if (app->filledGroupMap() == false) {
            mp_f->fillGroupMap(gmp, app);
        }
    }

    Node *getNode(const string &name, 
                  Group *grp,
                  bool managed = true,
                  bool create = false)
    {
        return mp_f->getNode(name, grp, managed, create);
    }
    Node *getNode(const string &appName,
                  const string &grpName,
                  const string &nodeName,
                  bool managed = true,
                  bool create = false)
    {
        return mp_f->getNode(appName,
                             grpName,
                             nodeName,
                             managed,
                             create);
    }
    void fillNodeMap(NodeMap *nmp, Group *grp)
    {
        if (grp->filledNodeMap() == false) {
            mp_f->fillNodeMap(nmp, grp);
        }
    }

    DataDistribution *getDistribution(const string &name,
                                      Application *app,
                                      bool create = false)
    {
        return mp_f->getDistribution(name, app, create);
    }
    DataDistribution *getDistribution(const string &appName,
                                      const string &distName,
                                      bool create = false)
    {
        return mp_f->getDistribution(appName, distName, create);
    }
    void fillDataDistributionMap(DataDistributionMap *dmp,
                                 Application *app)
    {
        if (app->filledDataDistributionMap() == false) {
            mp_f->fillDataDistributionMap(dmp, app);
        }
    }
    string loadShards(const string &key)
    {
        return mp_f->loadShards(key);
    }
    string loadManualOverrides(const string &key)
    {
        return mp_f->loadManualOverrides(key);
    }

    string createNodeKey(const string &appName,
                         const string &groupName,
                         const string &nodeName,
                         bool managed)
    {
        return mp_f->createNodeKey(appName,
                                   groupName,
                                   nodeName,
                                   managed);
    }
    string createGroupKey(const string &appName,
                          const string &groupName)
    {
        return mp_f->createGroupKey(appName, groupName);
    }
    string createAppKey(const string &appName)
    {
        return mp_f->createAppKey(appName);
    }
    string createDistKey(const string &appName,
                         const string &distName)
    {
        return mp_f->createDistKey(appName, distName);
    }

    bool isNodeKey(const string &key, bool *managedP = NULL)
    {
        return mp_f->isNodeKey(key, managedP);
    }
    bool hasNodeKeyPrefix(const string &key, bool *managedP = NULL)
    {
        return mp_f->hasNodeKeyPrefix(key, managedP);
    }
    bool isGroupKey(const string &key)
    {
        return mp_f->isGroupKey(key);
    }
    bool hasGroupKeyPrefix(const string &key)
    {
        return mp_f->hasGroupKeyPrefix(key);
    }
    bool isAppKey(const string &key)
    {
        return mp_f->isAppKey(key);
    }
    bool hasAppKeyPrefix(const string &key)
    {
        return mp_f->hasAppKeyPrefix(key);
    }
    bool isDistKey(const string &key)
    {
        return mp_f->isDistKey(key);
    }
    bool hasDistKeyPrefix(const string &key)
    {
        return mp_f->hasDistKeyPrefix(key);
    }
    
  private:
    /*
     * Friend declaration for Factory so it can call
     * the constructor.
     */
    friend class Factory;
    
    /*
     * Constructor used by Factory.
     */
    FactoryOps(Factory *f) : mp_f(f) {};

    /*
     * Default constructor throws an exception.
     */
    FactoryOps()
    {
        throw ClusterException("Someone called the default "
                               "FactoryOps constructor!");
    }

  private:
    /*
     * The factory associated with this instance of FactoryOps.
     */
    Factory *mp_f;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIB_H_ */
