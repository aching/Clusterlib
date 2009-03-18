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
#include "healthchecker.h"
#include "notifyable.h"
#include "clusterclient.h"
#include "clusterserver.h"
#include "group.h"
#include "application.h"
#include "node.h"
#include "datadistribution.h"
#include "properties.h"

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
    static const string KEYSEPARATOR;

    static const string CLUSTERLIB;
    static const string CLUSTERLIBVERSION;

    static const string PROPERTIES;
    static const string CONFIGURATION;
    static const string ALERTS;
    static const string SYNC;

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

    static const string LEADERSHIP;
    static const string BIDS;
    static const string CURRENTLEADER;

    static const string SHARDS;
    static const string GOLDENSHARDS;
    static const string MANUALOVERRIDES;

    static const string LOCKS;
    static const string QUEUES;
    static const string BARRIERS;
    static const string TRANSACTIONS;

    static const string BIDPREFIX;
    static const string INFLUX;
    static const string HEALTHY;
    static const string UNHEALTHY;

    /*
     * Names of predefined properties.
     */
    static const string HEARTBEATMULTIPLE;
    static const string HEARTBEATCHECKPERIOD;
    static const string HEARTBEATHEALTHY;
    static const string HEARTBEATUNHEALTHY;
    static const string TIMEOUTUNHEALTHYYTOR;
    static const string TIMEOUTUNHEALTHYRTOD;
    static const string TIMEOUTDISCONNECTYTOR;
    static const string TIMEOUTDISCONNECTRTOD;
    static const string NODESTATEGREEN;
    static const string NODEBOUNCYPERIOD;
    static const string NODEBOUNCYNEVENTS;
    static const string NODEMOVEBACKPERIOD;
    static const string CLUSTERUNMANAGED;
    static const string CLUSTERDOWN;
    static const string CLUSTERFLUXPERIOD;
    static const string CLUSTERFLUXNEVENTS;
    static const string HISTORYSIZE;
    static const string LEADERFAILLIMIT;
    static const string SERVERBIN;
    
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

class ClusterlibInts
{
  public:
    /* 
     * All indices use for parsing ZK node names
     */
    static const int32_t CLUSTERLIB_INDEX;
    static const int32_t VERSION_NAME_INDEX;
    static const int32_t APP_INDEX;
    static const int32_t APP_NAME_INDEX;

    /*
     * Number of components in an Application key
     */
    static const int32_t APP_COMPONENTS_COUNT;

    /*
     * Minimum components necessary to represent each respective key
     */
    static const int32_t DIST_COMPONENTS_MIN_COUNT;
    static const int32_t PROP_COMPONENTS_MIN_COUNT;
    static const int32_t GROUP_COMPONENTS_MIN_COUNT;
    static const int32_t NODE_COMPONENTS_MIN_COUNT;
};

/*
 * The actual factory class.
 */
class Factory
    : public virtual ClusterlibStrings, public virtual ClusterlibInts
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
    Server *createServer(Group *group,
                         const string &nodeName,
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
     * Ensure that all operations at this point have been pushed to
     * the underlying data store.
     */
    void synchronize();

    /*
     * For use by unit tests only: get the zkadapter so that the test can
     * synthesize ZK events and examine the results.
     */
    zk::ZooKeeperAdapter *getRepository() { return &m_zk; }

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
     * Clean up clients
     */
    void removeAllClients();

    /*
     * Clean up data distributions
     */
    void removeAllDataDistributions();

    /*
     * Clean up properties maps
     */
    void removeAllProperties();

    /*
     * Clean up applications
     */
    void removeAllApplications();

    /*
     * Clean up groups
     */
    void removeAllGroups();

    /*
     * Clean up nodes
     */
    void removeAllNodes();

    /*
     * Register/cancel a timer handler.
     */
    TimerId registerTimer(TimerEventHandler *handler,
                          uint64_t afterTime,
                          ClientData data);
    bool cancelTimer(TimerId id);
    
    /*
     * Dispatch all events. Reads from the
     * event sources and sends events to
     * the registered client for each event.
     */
    void dispatchEvents();

    /*
     * Dispatch timer, zk, and session events.
     */
    void dispatchTimerEvent(ClusterlibTimerEvent *tep);
    void dispatchZKEvent(zk::ZKWatcherEvent *zep);
    void dispatchSessionEvent(zk::ZKWatcherEvent *zep);
    bool dispatchEndEvent();

    /*
     * Helper method that updates the cached representation
     * of a clusterlib repository object and generates the
     * prototypical cluster event payload to send to clients.
     */
    ClusterEventPayload *updateCachedObject(FactoryEventHandler *cp,
                                            zk::ZKWatcherEvent *zep);

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
     * Retrieve a list of all (currently known) group names within
     * the given group. This also establishes a watch on
     * group changes.
     */
    IdList getGroupNames(Group *grp);

    /*
     * Retrieve a list of all (currently known) distribution
     * names within the given group. This also establishes
     * a watch on distribution changes.
     */
    IdList getDataDistributionNames(Group *grp);

    /*
     * Retrieve a list of all (currently known) node names
     * within the given group. This also establishes a
     * watch on node changes.
     */
    IdList getNodeNames(Group *grp);

    /*
     * Leadership protocol.
     */
    Node *getLeader(Group *grp);
    int64_t placeBid(Node *np, Server *sp);
    bool tryToBecomeLeader(Node *np, int64_t bid);
    bool isLeaderKnown(Node *np);
    void leaderIsUnknown(Node *np);
    void giveUpLeadership(Node *np, int64_t bid);

    /*
     * Methods to prepare strings for leadership protocol.
     */
    string getCurrentLeaderNodeName(const string &gkey);
    string getLeadershipBidsNodeName(const string &gkey);
    string getLeadershipBidPrefix(const string &gkey);

    /*
     * Retrieve (and potentially create) instances of
     * objects representing applications, groups, nodes,
     * and distributions.
     */
    Application *getApplication(const string &appName,
                                bool create = false);

    /** 
     * Get a group from a parent group key. 
     *
     * @param groupName name of the group under the parent
     * @param groupKey key to this parent group
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the Group
     */
    Group *getGroup(const string &groupName,
                    Group *parentGroup,
                    bool create = false);

    Node *getNode(const string &nodeName,
                  Group *parentGroup,
                  bool managed,
                  bool create = false);

    DataDistribution *getDataDistribution(const string &distName,
                                      Group *parentGroup,
                                      bool create = false);

    Properties *getProperties(Notifyable *parent,
			      bool create = false);

    void updateDataDistribution(const string &key,
                                const string &shards,
                                const string &manualOverrides,
                                int32_t shardsVersion,
                                int32_t manualOverridesVersion);
    void updateProperties(const string &key,
			  const string &properties,
			  int32_t versionNumber,
                          int32_t &finalVersionNumber);
    void updateNodeClientState(const string &key,
                               const string &cs);
    void updateNodeClientStateDesc(const string &key,
                                   const string &desc);
    void updateNodeMasterSetState(const string &key,
                                  const string &ms);

    /**
     * Get the notifyable represented by this key.  If this key does
     * not represent a Notifyable, keep stripping off sections of the
     * key until a Notifyable is found or the key is empty.
     *
     * @param key the key that should contain a clusterlib object
     * @param create try to create this object if it does not exist?
     * @return NULL if no Notifyable can be found, else the Notifyable *
     */
    Notifyable *getNotifyableFromKey(const string &key, bool create = false);

    /**
     * Get the notifyable represented by these components.  If the
     * components does not represent a Notifyable, keep stripping off
     * sections of the components until a Notifyable is found or the
     * components run out of elements.
     *
     * @param components Should represent the Notifyable object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     */
    Notifyable *getNotifyableFromComponents(const vector<string> &components,
                                            int32_t elements = -1, 
                                            bool create = false);

    /**
     * Get the exact application represented by this key
     *
     * @param key should represent the Application object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Application *
     */
    Application *getApplicationFromKey(const string &key,
                                       bool create = false);
    /**
     * Get the exact Application represented by these components.
     *
     * @param components Should represent the Application object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Application *
     */
    Application *getApplicationFromComponents(const vector<string> &components,
                                              int32_t elements = -1, 
                                              bool create = false);
    /**
     * Get the exact data distribution represented by this key
     *
     * @param key should represent the DataDistribution object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the DataDistribution *
     */
    DataDistribution *getDataDistributionFromKey(const string &key,
                                                 bool create = false);

    /**
     * Get the exact DataDistribution represented by these components.
     *
     * @param components Should represent the DataDistribution object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the DataDistribution *
     */
    DataDistribution *getDataDistributionFromComponents(
        const vector<string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the exact properties represented by this key
     *
     * @param key should represent the Properties object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Properties *
     */
    Properties *getPropertiesFromKey(const string &key,
                                     bool create = false);

    /**
     * Get the exact Properties represented by these components.
     *
     * @param components Should represent the Properties object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Properties *
     */
    Properties *getPropertiesFromComponents(
        const vector<string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the exact group or application represented by this key.
     *
     * @param key should represent the Group/Applications object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Group *
     */
    Group *getGroupFromKey(const string &key,
                           bool create = false);


    /**
     * Get the exact Group/Applications represented by these components.
     *
     * @param components Should represent the Group object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Group *
     */
    Group *getGroupFromComponents(
        const vector<string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the node represented exactly by this key
     *
     * @param key should represent the Node object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Node *
     */
    Node *getNodeFromKey(const string &key, bool create = false);

    /**
     * Get the exact Node represented by these components.
     *
     * @param components Should represent the Node object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Node *
     */
    Node *getNodeFromComponents(
        const vector<string> &components,
        int32_t elements = -1, 
        bool create = false);

    /* 
     * Generate valid keys for various clusterlib objects given that
     * the inputs are valid.  Does not create the objects or check
     * that they exist. 
     */
    string createNodeKey(const string &groupKey,
                         const string &nodeName,
                         bool managed);
    string createGroupKey(const string &groupKey,
                          const string &groupName);
    string createAppKey(const string &appName);
    string createDistKey(const string &groupKey,
                         const string &distName);
    string createPropertiesKey(const string &notifyableKey);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Application exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    bool isApplicationKey(const vector<string> &components, 
                          int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Application exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    bool isApplicationKey(const string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual DataDistribution exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    bool isDataDistributionKey(const vector<string> &components, 
                               int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual DataDistribution exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    bool isDataDistributionKey(const string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Group exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    bool isGroupKey(const vector<string> &components, 
                    int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Group exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    bool isGroupKey(const string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Properties exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    bool isPropertiesKey(const vector<string> &components, 
                         int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Properties exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    bool isPropertiesKey(const string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Node exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    bool isNodeKey(const vector<string> &components, 
                   int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Node exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    bool isNodeKey(const string &key);

    /**
     * Remove the leaf node and returns the closest clusterlib
     * Notifyable object key.  For example, if the initial key is
     * .../nodes/foo-server/properties, it will return
     * .../nodes/foo-server.  If the key is
     * .../group/client/nodes/foo-server, it will return
     * .../group/client.  If the key is .../applications/foo-app, it
     * will return an empty string since they is nothing left. The key
     * must not end in a KEYSEPARATOR.
     *
     * @param key a path to be trimmed
     * @return trimmed key or empty string if no parent clusterlib object key
     */
    string removeObjectFromKey(const string &key);

    /**
     * Remove the leaf node and returns the closest clusterlib
     * Notifyable object key.  For example, if the initial components form
     * .../nodes/foo-server/properties, it will return elements that include
     * .../nodes/foo-server.  If the components form 
     * .../group/client/nodes/foo-server, it will return elements that include
     * .../group/client.  If the components are .../applications/foo-app, it
     * will return an empty string since they is nothing left. The key
     * must not end in a KEYSEPARATOR.
     *
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return size of the trimmed key or -1 if no Notifyable parent possible
     */
    int32_t removeObjectFromComponents(const vector<string> &comps,
                                       int32_t elements);

    /*
     * Load entities from ZooKeeper.
     */
    Application *loadApplication(const string &name,
                                 const string &key);
    DataDistribution *loadDataDistribution(const string &distName,
                                       const string &distKey,
                                       Group *parentGroup);
    Properties* loadProperties(const string &key,
                               Notifyable *parent);
    Group *loadGroup(const string &groupName,
                     const string &groupKey,
                     Group *parentGroup);
    Node *loadNode(const string &nodeName,
                   const string &nodeKey,
                   Group *parentGroup);

    string loadShards(const string &key, int32_t &version);
    string loadManualOverrides(const string &key, int32_t &version);
    string loadKeyValMap(const string &key, int32_t &version);

    /*
     * Create entities in ZooKeeper.
     */
    Application *createApplication(const string &appName, 
				   const string &key);
    DataDistribution *createDataDistribution(
	const string &distName,
        const string &distKey,
        const string &marshalledShards,
        const string &marshalledManualOverrides,
        Group *parentGroup);
    Properties *createProperties(const string &propsKey,
                                 Notifyable *parent);
    Group *createGroup(const string &groupName,
                       const string &groupKey,
                       Group *parentGroup);
    Node *createNode(const string &nodeName,
		     const string &nodeKey,
		     Group *parentGroup);

    /*
     * Get bits of Node state.
     */
    bool isNodeConnected(const string &key);
    string getNodeClientState(const string &key);
    int32_t getNodeMasterSetState(const string &key);

    /*
     * Get various locks.
     */
    Mutex *getClientsLock() { return &m_clLock; }
    Mutex *getLeadershipWatchesLock() { return &m_lwLock; }
    Mutex *getPropertiesLock() { return &m_propLock; }
    Mutex *getDataDistributionsLock() { return &m_ddLock; }
    Mutex *getApplicationsLock() { return &m_appLock; }
    Mutex *getGroupsLock() { return &m_grpLock; }
    Mutex *getNodesLock() { return &m_nodeLock; }
    Mutex *getTimersLock() { return &m_timerRegistryLock; }
    Mutex *getSyncLock() { return &m_syncLock; }
    Mutex *getEndEventLock() { return &m_endEventLock; }

    /*
     * Implement ready protocol for notifyables.
     */
    bool establishNotifyableReady(Notifyable *ntp);
    Event handleNotifyableReady(Notifyable *ntp,
                                int32_t etype,
                                const string &key);

    /*
     * Handle existence events on notifyables.
     */
    Event handleNotifyableExists(Notifyable *ntp,
                                 int32_t etype,
                                 const string &key);

    /*
     * Handle changes in the set of applications.
     */
    Event handleApplicationsChange(Notifyable *ntp,
                                   int32_t etype,
                                   const string &key);

    /*
     * Handle changes in the set of groups in
     * a group.
     */
    Event handleGroupsChange(Notifyable *ntp,
                             int32_t etype,
                             const string &key);

    /*
     * Handle changes in the set of data distributions
     * in a group.
     */
    Event handleDataDistributionsChange(Notifyable *ntp,
                                        int32_t etype,
                                        const string &key);

    /*
     * Handle changes in the set of nodes in a group.
     */
    Event handleNodesChange(Notifyable *ntp,
                            int32_t etype,
                            const string &key);

    /*
     * Handle changes in a property list value.
     */
    Event handlePropertiesValueChange(Notifyable *ntp,
                                      int32_t etype,
                                      const string &key);

    /*
     * Handle changes in shards of a distribution.
     */
    Event handleShardsChange(Notifyable *ntp,
                             int32_t etype,
                             const string &key);

    /*
     * Handle changes in manual overrides in
     * a distribution.
     */
    Event handleManualOverridesChange(Notifyable *ntp,
                                      int32_t etype,
                                      const string &key);

    /*
     * Handle changes in client-reported state for
     * a node.
     */
    Event handleClientStateChange(Notifyable *ntp,
                                  int32_t etype,
                                  const string &key);

    /*
     * Handle changes in master-set desired state
     * for a node.
     */
    Event handleMasterSetStateChange(Notifyable *ntp,
                                     int32_t etype,
                                     const string &key);

    /*
     * Handle a change in the connected state for
     * a node.
     */
    Event handleNodeConnectionChange(Notifyable *ntp,
                                     int32_t etype,
                                     const string &key);

    /*
     * Handle changes in the leadership of a
     * group.
     */
    Event handleLeadershipChange(Notifyable *ntp,
                                 int32_t etype,
                                 const string &key);

    /*
     * Handle existence change for preceding leader of
     * a group.
     */
    Event handlePrecLeaderExistsChange(Notifyable *ntp,
                                       int32_t etype,
                                       const string &key);

    /*
     * Handle changes in synchronization of a zookeeper key.
     */
    Event handleSynchronizeChange(Notifyable *ntp,
                                  int32_t etype,
                                  const string &key);

    /*
     * Orderly termination mechanism.
     */
    void injectEndEvent();
    void waitForThreads();

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
     * The registry of leadership election watches.
     */
    LeadershipElectionMultimap m_leadershipWatches;
    Mutex m_lwLock;

    /*
     * The registry of cached properties maps.
     */
    PropertiesMap m_properties;
    Mutex m_propLock;

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
     * The registry of timer handlers.
     */
    TimerRegistry m_timerRegistry;
    Mutex m_timerRegistryLock;

    /*
     * The registry of outstanding sync operations
     */
    int64_t m_syncId;
    int64_t m_syncIdCompleted;
    Mutex m_syncLock;
    Cond m_syncCond;

    /*
     * Remember whether an END event has been dispatched
     * so that all threads wind down. Can do this only
     * once!!!
     */
    bool m_endEventDispatched;
    Mutex m_endEventLock;

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

    /*
     * Handlers for event delivery.
     */
    FactoryEventHandler m_notifyableReadyHandler;
    FactoryEventHandler m_notifyableExistsHandler;
    FactoryEventHandler m_propertiesValueChangeHandler;
    FactoryEventHandler m_applicationsChangeHandler;
    FactoryEventHandler m_groupsChangeHandler;
    FactoryEventHandler m_distributionsChangeHandler;
    FactoryEventHandler m_shardsChangeHandler;
    FactoryEventHandler m_manualOverridesChangeHandler;
    FactoryEventHandler m_nodesChangeHandler;
    FactoryEventHandler m_nodeClientStateChangeHandler;
    FactoryEventHandler m_nodeMasterSetStateChangeHandler;
    FactoryEventHandler m_nodeConnectionChangeHandler;
    FactoryEventHandler m_leadershipChangeHandler;
    FactoryEventHandler m_precLeaderExistsHandler;
    FactoryEventHandler m_synchronizeChangeHandler;
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
    {
        return mp_f->registerTimer(handler, afterTime, data);
    }
    bool cancelTimer(TimerId id)
    {
        return mp_f->cancelTimer(id);
    }

    Application *getApplication(const string &name,
                                bool create = false)
    {
        return mp_f->getApplication(name, create);
    }

    Group *getGroup(const string &groupName,
                    Group *parentGroup,
                    bool create = false)
    {
        return mp_f->getGroup(groupName, parentGroup, create);
    }

    Node *getNode(const string &nodeName, 
                  Group *parentGroup,
                  bool managed = true,
                  bool create = false)
    {
        return mp_f->getNode(nodeName, parentGroup, managed, create);
    }

    DataDistribution *getDataDistribution(const string &name,
                                      Group *group,
                                      bool create = false)
    {
        return mp_f->getDataDistribution(name, group, create);
    }
    
    Properties *getProperties(Notifyable *parent, 
                              bool create = false)
    {
	return mp_f->getProperties(parent, create);
    }

    string loadShards(const string &key, int32_t &version)
    {
        return mp_f->loadShards(key, version);
    }
    string loadManualOverrides(const string &key, int32_t &version)
    {
        return mp_f->loadManualOverrides(key, version);
    }
    string loadKeyValMap(const string &key, int32_t &version) {
	return mp_f->loadKeyValMap(key, version);
    }

    void updateDataDistribution(const string &key,
                                const string &shards,
                                const string &manualOverrides,
                                int32_t shardsVersion,
                                int32_t manualOverridesVersion)
    {
        mp_f->updateDataDistribution(key,
                                     shards,
                                     manualOverrides,
                                     shardsVersion,
                                     manualOverridesVersion);
    }
    void updateProperties(const string &key,
			  const string &properties,
			  int32_t versionNumber,
                          int32_t &finalVersionNumber)
    {
	mp_f->updateProperties(key,
			       properties,
			       versionNumber,
                               finalVersionNumber);
    }	

    void updateNodeClientState(const string &key,
                               const string &cs)
    {
        mp_f->updateNodeClientState(key, cs);
    }
    void updateNodeClientStateDesc(const string &key,
                                   const string &desc)
    {
        mp_f->updateNodeClientStateDesc(key, desc);
    }
    void updateNodeMasterSetState(const string &key,
                                  const string &ms)
    {
        mp_f->updateNodeMasterSetState(key, ms);
    }

    Notifyable *getNotifyableFromKey(const string &key, bool create = false)
    {
        return mp_f->getNotifyableFromKey(key, create);
    }
    Application *getApplicationFromKey(const string &key,
                                       bool create = false)
    {
        return mp_f->getApplicationFromKey(key, create);
    }
    DataDistribution *getDataDistributionFromKey(const string &key,
                                                 bool create = false)
    {
        return mp_f->getDataDistributionFromKey(key, create);
    }
    Properties *getPropertiesFromKey(const string &key,
                                     bool create = false)
    {
        return mp_f->getPropertiesFromKey(key, create);
    }
    Group *getGroupFromKey(const string &key,
                           bool create = false)
    {
        return mp_f->getGroupFromKey(key, create);
    }
    Node *getNodeFromKey(const string &key,
                         bool create = false)
    {
        return mp_f->getNodeFromKey(key, create);
    }
    
    string createNodeKey(const string &groupKey,
                         const string &nodeName,
                         bool managed)
    {
        return mp_f->createNodeKey(groupKey,
                                   nodeName,
                                   managed);
    }
    string createGroupKey(const string &groupKey,
                          const string &groupName)
    {
        return mp_f->createGroupKey(groupKey, groupName);
    }
    string createAppKey(const string &appName)
    {
        return mp_f->createAppKey(appName);
    }
    string createDistKey(const string &groupKey,
                         const string &distName)
    {
        return mp_f->createDistKey(groupKey, distName);
    }
    string createPropertiesKey(const string &notifyableKey)
    {
	return mp_f->createPropertiesKey(notifyableKey);
    }

    string removeObjectFromKey(const string &key)
    {
	return mp_f->removeObjectFromKey(key);
    }

    /*
     * Establish the ready protocol.
     */
    bool establishNotifyableReady(Notifyable *ntp)
    {
        return mp_f->establishNotifyableReady(ntp);
    }

    /*
     * Return entity names.
     */
    IdList getApplicationNames()
    {
        return mp_f->getApplicationNames();
    }
    IdList getGroupNames(Group *grp)
    {
        return mp_f->getGroupNames(grp);
    }
    IdList getDataDistributionNames(Group *grp)
    {
        return mp_f->getDataDistributionNames(grp);
    }
    IdList getNodeNames(Group *grp)
    {
        return mp_f->getNodeNames(grp);
    }

    /*
     * Group leadership protocol.
     */
    Node *getLeader(Group *grp)
    {
        return mp_f->getLeader(grp);
    }
    int64_t placeBid(Node *np, Server *sp)
    {
        return mp_f->placeBid(np, sp);
    }
    bool tryToBecomeLeader(Node *np, int64_t bid)
    {
        return mp_f->tryToBecomeLeader(np, bid);
    }
    bool isLeaderKnown(Node *np)
    {
        return mp_f->isLeaderKnown(np);
    }
    void leaderIsUnknown(Node *np)
    {
        mp_f->leaderIsUnknown(np);
    }
    void giveUpLeadership(Node *np, int64_t bid)
    {
        mp_f->giveUpLeadership(np, bid);
    }

    /*
     * Helper methods to prepare strings for leadership
     * protocol.
     */
    string getCurrentLeaderNodeName(const string &gkey)
    {
        return mp_f->getCurrentLeaderNodeName(gkey);
    }
    string getLeadershipBidsNodeName(const string &gkey)
    {
        return mp_f->getLeadershipBidsNodeName(gkey);
    }
    string getLeadershipBidPrefix(const string &gkey)
    {
        return mp_f->getLeadershipBidPrefix(gkey);
    }

    /*
     * Get bits of Node state.
     */
    bool isNodeConnected(const string &key)
    {
        return mp_f->isNodeConnected(key);
    }
    string getNodeClientState(const string &key)
    {
        return mp_f->getNodeClientState(key);
    }
    int32_t getNodeMasterSetState(const string &key)
    {
        return mp_f->getNodeMasterSetState(key);
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
    FactoryOps(Factory *fp) : mp_f(fp) {};

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
