/*
 * factoryops.h --
 *
 * Implementation of FactoryOps class.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_FACTORYOPS_H_
#define	_FACTORYOPS_H_

/**
 * Macro for safely executing calls to ZooKeeper.
 *
 * NOTE: If your call needs to return a value, please use
 * SAFE_CALL_ZK((var = call()), "some message %s", false, true)
 */
#define SAFE_CALL_ZK(_action, _message, _node, _warning, _once) \
{ \
    bool __done = false; \
    while (!__done) { \
	try { \
	    _action ; __done = true; \
	} catch (zk::ZooKeeperException &ze) { \
	    if (getOps()->isShutdown()) { \
		LOG_WARN(CL_LOG, "Call to ZK during shutdown!"); \
		__done = true; \
	    } \
	    else if (!ze.isConnected()) { \
		throw RepositoryConnectionFailureException(ze.what()); \
	    } \
	    else if (_warning) { \
		LOG_WARN(CL_LOG, _message, _node, ze.what()); \
		if (_once) { \
		    /* \
		     * Only warn once. \
		     */ \
		    __done = true; \
                } \
	    } \
	    else { \
		throw RepositoryInternalsFailureException(ze.what()); \
	    } \
	} \
    } \
}

/**
 * Macro for safely setting up callbacks for zookeeper.
 * @param _action1 the action that occurs if the handler callback == 0
 * @param _action2 the action that occurs if the handler callback > 0
 * @param _changeHandler the CachedObjectChange (i.e. NODES_CHANGE)
 * @param _key a string that represents the ZooKeeper node
 */
#define SAFE_CALLBACK_ZK(_action1, \
                         _action2, \
                         _changeHandler, \
                         _key, \
                         _message, \
                         _node, \
                         _warning, \
                         _once) \
{ \
    Locker __l1(getCachedObjectChangeHandlers()->getLock()); \
    bool __ready = getCachedObjectChangeHandlers()-> \
        isHandlerCallbackReady(_changeHandler, _key); \
    if (__ready == false) { \
        SAFE_CALL_ZK(_action1, _message, _node, _warning, _once); \
        getCachedObjectChangeHandlers()-> \
            setHandlerCallbackReady(_changeHandler, _key); \
    } \
    else { \
        SAFE_CALL_ZK(_action2, _message, _node, _warning, _once); \
    } \
} 

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
 * This class does all the actual work of the Factory
 */
 class FactoryOps 
{
  public:
    /**
     * Constructor that should only be called from Factory
     */
    FactoryOps(const std::string &registry);

    /**
     * Destructory
     */
    ~FactoryOps();

    /**
     * Create a cluster client object.
     *
     * @return a Client pointer
     */
    Client *createClient();

    /**
     * Convenience function -- return the current time in ms
     * from the unix epoch.
     *
     * @return current time in milliseconds
     */
    static int64_t getCurrentTimeMillis();

    /**
     * Is the factory connected to ZooKeeper?
     * 
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * Ensure that all operations at this point have been pushed to
     * the underlying data store.
     */
    void synchronize();

    /**
     * For use by unit tests only: get the zkadapter so that the test can
     * synthesize ZK events and examine the results.
     * 
     * @return the ZooKeeperAdapter * from Factory Ops
     */
    zk::ZooKeeperAdapter *getRepository()
    {
        return &m_zk;
    }

    /*
     * Add and remove clients.
     */
    void addClient(ClientImpl *clp);
    void removeClient(ClientImpl *clp);

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
     * Clean up all removed notifyables
     */
    void removeAllRemovedNotifyables();

    /*
     * Register/cancel a timer handler.
     */
    TimerId registerTimer(TimerEventHandler *handler,
                          uint64_t afterTime,
                          ClientData data);
    bool cancelTimer(TimerId id);
    
    /*
     * Dispatch timer, zk, and session events.
     */
    void dispatchTimerEvent(ClusterlibTimerEvent *tep);
    void dispatchZKEvent(zk::ZKWatcherEvent *zep);

    /**
     * Dispatch a session event. These events are handled here.  This
     * can trigger a shutdown of clusterlib threads by setting
     * m_shutdown to true.
     *
     * @param zep the pointer to the event to dispatch
     */
    void dispatchSessionEvent(zk::ZKWatcherEvent *zep);

    /**
     * Dispatch a final event to all registered clients
     * to indicate that no more events will be sent
     * following this one.
     *
     * @return true if the end event was dispatched, false if not
     */
    bool dispatchEndEvent();

    /**
     * Checks whether this is an internal event.  Should only be
     * processed by dispatchInternalEvents.
     *
     * @param ge the GenericEvent pointer
     */
    bool isInternalEvent(GenericEvent *ge);

    /*
     * Helper method that updates the cached representation
     * of a clusterlib repository object and generates the
     * prototypical cluster event payload to send to clients.
     */
    ClusterEventPayload *updateCachedObject(CachedObjectEventHandler *cp,
                                            zk::ZKWatcherEvent *zep);

    /*
     * Retrieve a list of all (currently known) applications.
     */
    NameList getApplicationNames();

    /*
     * Retrieve a list of all (currently known) group names within
     * the given group. This also establishes a watch on
     * group changes.
     */
    NameList getGroupNames(GroupImpl *group);

    /*
     * Retrieve a list of all (currently known) distribution
     * names within the given group. This also establishes
     * a watch on distribution changes.
     */
    NameList getDataDistributionNames(GroupImpl *group);

    /*
     * Retrieve a list of all (currently known) node names
     * within the given group. This also establishes a
     * watch on node changes.
     */
    NameList getNodeNames(GroupImpl *group);

    /**
     * Get any immediate children of this NotifyableImpl.  In order to
     * guarantee that this is atomic, hold the lock of this
     * NotifyableImpl first.
     *
     * @param ntp the NotifyableImpl to look for children on
     * @return list of pointers to the Notifyable children.
     */
    NotifyableList getChildren(Notifyable *ntp);
    
    /*
     * Retrieve (and potentially create) instances of
     * objects representing applications, groups, nodes,
     * and distributions.
     */
    
    /** 
     * Get the root. 
     *
     * @return the pointer to the Root
     */
    RootImpl *getRoot();

    /** 
     * Get a application from the root. 
     *
     * @param appName name of the application under the root
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the Application
     */
    ApplicationImpl *getApplication(const std::string &appName,
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
    GroupImpl *getGroup(const std::string &groupName,
                        GroupImpl *parentGroup,
                        bool create = false);

    NodeImpl *getNode(const std::string &nodeName,
                      GroupImpl *parentGroup,
                      bool create = false);

    DataDistributionImpl *getDataDistribution(const std::string &distName,
                                              GroupImpl *parentGroup,
                                              bool create = false);

    PropertiesImpl *getProperties(Notifyable *parent,
                                  bool create = false);

    void updateDataDistribution(const std::string &key,
                                const std::string &shards,
                                const std::string &manualOverrides,
                                int32_t shardsVersion,
                                int32_t manualOverridesVersion);
    void updateProperties(const std::string &key,
			  const std::string &properties,
			  int32_t versionNumber,
                          int32_t &finalVersionNumber);
    void updateNodeClientState(const std::string &key,
                               const std::string &cs);
    void updateNodeClientStateDesc(const std::string &key,
                                   const std::string &desc);
    void updateNodeMasterSetState(const std::string &key,
                                  const std::string &ms);
    
    /**
     * Get the notifyable represented by this key. 
     *
     * @param key the key that should contain a clusterlib object
     * @param create try to create this object if it does not exist?
     * @return NULL if no Notifyable can be found, else the Notifyable *
     */
    NotifyableImpl *getNotifyableFromKey(const std::string &key, 
                                         bool create = false);

    /**
     * Get the notifyable represented by these components.
     *
     * @param components Should represent the Notifyable object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     */
    NotifyableImpl *getNotifyableFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the exact application represented by this key
     *
     * @param key should represent the Application object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the Application *
     */
    ApplicationImpl *getApplicationFromKey(const std::string &key,
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
    ApplicationImpl *getApplicationFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the exact root represented by this key
     *
     * @param key should represent the Root object
     * @return NULL if cannot be found, else the Root *
     */
    RootImpl *getRootFromKey(const std::string &key);
    /**
     * Get the exact Root represented by these components.
     *
     * @param components Should represent the Root object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @return NULL if cannot be found, else the Root *
     */
    RootImpl *getRootFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1);

    /**
     * Get the exact data distribution represented by this key
     *
     * @param key should represent the DataDistribution object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the DataDistribution *
     */
    DataDistributionImpl *getDataDistributionFromKey(
        const std::string &key,
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
    DataDistributionImpl *getDataDistributionFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the exact properties represented by this key
     *
     * @param key should represent the PropertiesImpl object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the PropertiesImpl *
     */
    PropertiesImpl *getPropertiesFromKey(const std::string &key,
                                         bool create = false);

    /**
     * Get the exact PropertiesImpl represented by these components.
     *
     * @param components Should represent the PropertiesImpl object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the PropertiesImpl *
     */
    PropertiesImpl *getPropertiesFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the exact group or application represented by this key.
     *
     * @param key should represent the Group/Applications object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the GroupImpl *
     */
    GroupImpl *getGroupFromKey(const std::string &key, bool create = false);

    /**
     * Get the exact Group/Applications represented by these components.
     *
     * @param components Should represent the GroupImpl object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the GroupImpl *
     */
    GroupImpl *getGroupFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the node represented exactly by this key
     *
     * @param key should represent the NodeImpl object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the NodeImpl *
     */
    NodeImpl *getNodeFromKey(const std::string &key, bool create = false);

    /**
     * Get the exact NodeImpl represented by these components.
     *
     * @param components Should represent the NodeImpl object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the NodeImpl *
     */
    NodeImpl *getNodeFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        bool create = false);

    /*
     * Load entities from ZooKeeper.
     */
    ApplicationImpl *loadApplication(const std::string &name,
                                     const std::string &key);
    DataDistributionImpl *loadDataDistribution(const std::string &distName,
                                               const std::string &distKey,
                                               GroupImpl *parentGroup);
    PropertiesImpl* loadProperties(const std::string &key,
                                   Notifyable *parent);
    GroupImpl *loadGroup(const std::string &groupName,
                         const std::string &groupKey,
                         GroupImpl *parentGroup);
    NodeImpl *loadNode(const std::string &nodeName,
                       const std::string &nodeKey,
                       GroupImpl *parentGroup);

    std::string loadShards(const std::string &key, int32_t &version);
    std::string loadManualOverrides(const std::string &key, int32_t &version);
    std::string loadKeyValMap(const std::string &key, int32_t &version);

    /*
     * Create entities in ZooKeeper.
     */
    ApplicationImpl *createApplication(const std::string &appName, 
                                       const std::string &key);
    DataDistributionImpl *createDataDistribution(
	const std::string &distName,
        const std::string &distKey,
        const std::string &marshalledShards,
        const std::string &marshalledManualOverrides,
        GroupImpl *parentGroup);
    PropertiesImpl *createProperties(const std::string &propsKey,
                                     Notifyable *parent);
    GroupImpl *createGroup(const std::string &groupName,
                           const std::string &groupKey,
                           GroupImpl *parentGroup);
    NodeImpl *createNode(const std::string &nodeName,
                         const std::string &nodeKey,
                         GroupImpl *parentGroup);
    
    /*
     * Remove entities in Zookeeper.
     */
    void removeApplication(ApplicationImpl *app);
    void removeDataDistribution(DataDistributionImpl *dist);
    void removeProperties(PropertiesImpl *prop);
    void removeGroup(GroupImpl *group);
    void removeNode(NodeImpl *ntp);

    /*
     * Remove Notifyable from the factory cache and into
     * m_removedNotifyables (dead pool) that will be cleaned up later.
     */
    void removeNotifyableFromCacheByKey(const std::string &key);

    /*
     * Get bits of Node state.
     */
    bool isNodeConnected(const std::string &key);
    std::string getNodeClientState(const std::string &key);
    int32_t getNodeMasterSetState(const std::string &key);

    /*
     * Manage a node's connected state.
     */
    bool createConnected(const std::string &key);
    void removeConnected(const std::string &key);

    /*
     * Get various locks and conditionals.
     */
    Mutex *getClientsLock() { return &m_clLock; }
    Mutex *getPropertiesLock() { return &m_propLock; }
    Mutex *getDataDistributionsLock() { return &m_distLock; }
    Mutex *getRootLock() { return &m_rootLock; }
    Mutex *getApplicationsLock() { return &m_appLock; }
    Mutex *getGroupsLock() { return &m_groupLock; }
    Mutex *getNodesLock() { return &m_nodeLock; }
    Mutex *getRemovedNotifyablesLock() { return &m_removedNotifyablesLock; }
    Mutex *getTimersLock() { return &m_timerRegistryLock; }
    Mutex *getSyncLock() { return &m_syncLock; }
    Cond *getSyncCond() { return &m_syncCond; }
    Mutex *getEndEventLock() { return &m_endEventLock; }

    /**
     * Get the removed notifyables list
     */
    NotifyableList *getRemovedNotifyablesList()
    {
        return &m_removedNotifyables;
    }

    /**
     * Increment the sync completed
     */
    void incrSyncIdCompleted() 
    {
        ++m_syncIdCompleted; 
    }

    /**
     * Set up event handler for changes in the clusterlib object state.
     *
     * @param ntp Notifyable to watch for changes on
     */
    void establishNotifyableStateChange(NotifyableImpl *ntp);

    /**
     * Get the cached object change handlers object
     *
     * @return pointer to CachedObjectChangeHandlers for handling change
     */
    CachedObjectChangeHandlers *getCachedObjectChangeHandlers()
    {
        return &m_cachedObjectChangeHandlers;
    }

    /**
     * Get the internal change handlers object
     *
     * @return pointer to InternalChangeHandlers for handling change
     */
    InternalChangeHandlers *getInternalChangeHandlers()
    {
        return &m_internalChangeHandlers;
    }

    /**
     * Get the distributed locks object
     *
     * @return pointer to DistributedLocks for doing lock operations
     */
    DistributedLocks *getDistributedLocks()
    {
        return &m_distributedLocks;
    }

    /**
     * Get the Zookeeper source adapter.
     */
    ZooKeeperEventAdapter *getZooKeeperEventAdapter()
    {
        return &m_zkEventAdapter;
    }

    /*
     * Orderly termination mechanism.
     */
    void stopZKEventDispatch();
    void injectEndEvent();
    void waitForThreads();

    /**
     * Has shut down?
     */
    bool isShutdown() 
    {
        return m_shutdown;
    }

  private:
    /**
     * Required to support SAFE_CALL_ZK macro for various classes
     */
    FactoryOps *getOps() { return this; }

    /*
     * Dispatch all events. Reads from the
     * event sources and sends events to
     * the registered client for each event.
     */
    void dispatchExternalEvents(void *param);

    /**
     * Dispatch only internal clusterlib events that will not be
     * propaged to clusterlib clients.  Since these events are not
     * visible to clusterlib clients, they may violate the strict
     * ordering of events from m_zkEventAdapter.
     */
    void dispatchInternalEvents(void *param);

    /*
     * This method consumes timer events. It runs in a separate
     * thread.
     */
    void consumeTimerEvents(void *param);

  private:
    /*
     * The registry of attached clients (and servers).
     */
    ClientImplList m_clients;
    Mutex m_clLock;

    /*
     * The cached root.
     */
    RootImpl *mp_root;
    Mutex m_rootLock;

    /*
     * The registry of cached properties maps.
     */
    NotifyableImplMap m_props;
    Mutex m_propLock;

    /*
     * The registry of cached data distributions.
     */
    NotifyableImplMap m_dists;
    Mutex m_distLock;

    /*
     * The registry of cached applications.
     */
    NotifyableImplMap m_apps;
    Mutex m_appLock;

    /*
     * The registry of cached groups.
     */
    NotifyableImplMap m_groups;
    Mutex m_groupLock;

    /*
     * The registry of cached nodes.
     */
    NotifyableImplMap m_nodes;
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
     * The registry of deleted Notifyables
     */
    NotifyableList m_removedNotifyables;
    Mutex m_removedNotifyablesLock;

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
    CXXThread<FactoryOps> m_timerHandlerThread;

    /**
     * The Zookeeper source adapter.
     */
    ZooKeeperEventAdapter m_zkEventAdapter;

    /**
     * Synchronous event adapter for m_externalEventThread.
     */
    SynchronousEventAdapter<GenericEvent> m_externalEventAdapter;

    /**
     * The thread that processes all clusterlib external events from the
     * synchronous event adapter.
     */
    CXXThread<FactoryOps> m_externalEventThread;

    /**
     * Synchronous event adapter for m_internalEventAdapter.
     */
    SynchronousEventAdapter<GenericEvent> m_internalEventAdapter;

    /**
     * The thread that processes all clusterlib internal events from
     * the m_eventAdapter.  It is meant to handle events that the
     * m_clientEventThread depends on.
     */
    CXXThread<FactoryOps> m_internalEventThread;

    /**
     * Is the event loop terminating?
     */
    volatile bool m_shutdown;

    /**
     * Is the factory connected to ZooKeeper?
     */
    volatile bool m_connected;

    /**
     * Lock for event synchronization.
     */
    Lock m_eventSyncLock;

    /**
     * Handles all the events for clusterlib objects
     */
    CachedObjectChangeHandlers m_cachedObjectChangeHandlers;

    /**
     * Handles all event for internal event changes (not visible to
     * clusterlib clients)
     */
    InternalChangeHandlers m_internalChangeHandlers;

    /**
     * Handles all the locks for clusterlib objects
     */
    DistributedLocks m_distributedLocks;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_FACTORYOPS_H_ */
