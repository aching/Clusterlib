/*
 * factoryops.h --
 *
 * Implementation of FactoryOps class.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_FACTORYOPS_H_
#define	_CL_FACTORYOPS_H_

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
    Locker __l1(getOps()->getCachedObjectChangeHandlers()->getLock()); \
    bool __ready = getOps()->getCachedObjectChangeHandlers()-> \
        isHandlerCallbackReady(_changeHandler, _key); \
    if (__ready == false) { \
        SAFE_CALL_ZK(_action1, _message, _node, _warning, _once); \
        getOps()->getCachedObjectChangeHandlers()-> \
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
class FactoryOps {
  public:
    /**
     * Constructor that should only be called from Factory
     * 
     * @param registry the Zookeeper comma separated list of
     *        server:port (i.e. localhost:2221,localhost2:2222).
     * @param connectTimeout the amount of milliseconds to wait for a 
     *        connection to the specified registry
     */
    FactoryOps(const std::string &registry, int64_t connectTimeout);

    /**
     * Destructory
     */
    ~FactoryOps();

    /**
     * Create a cluster client object.  This object is a gateway to
     * the clusterlib objects and a context for user-level events.
     *
     * @return a Client pointer
     */
    Client *createClient();

    /**
     * Create a client for handling JSON-RPC responses.
     *
     * Register a special handler to allow this client to support
     * JSON-RPC response handling.  After JSON-RPC requests are sent,
     * this handler watches for responses.  This handler must be
     * registered prior to any JSON-RPC requests if a response is
     * desired.
     *
     * @param responseQueue the queue this client specifies for the
     *        response to its JSON-RPC requests
     * @param completedQueue the queue this client specifies for the
     *        problems with elements in the response queue
     * @return a Client pointer
     */
    Client *createJSONRPCResponseClient(Queue *responseQueue,
                                        Queue *completedQueue);
    
    /**
     * Create a client for handling JSON-RPC methods.
     *
     * Register a special handler to allow this client to support
     * JSON-RPC according to a JSONRPCManager.  Any encoded JSON-RPC
     * messages in the recvQueue are processed according to the
     * rpcManager and placed in the sender's response queue if
     * provided or the completedQueue.
     *
     * @param recvQueue the queue where this client receives JSON-RPC requests
     * @param completedQueue the queue where this client places responses or 
     *        errors for JSON-RPC requests if no destination is specified.
     * @param rpcManager actually invokes the methods to process JSON-RPC
     *        requests
     * @return a Client pointer
     */
    Client *createJSONRPCMethodClient(Queue *recvQueue,
                                      Queue *completedQueue,
                                      ::json::rpc::JSONRPCManager *rpcManager);
    
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

    /**
     * Register a timer handler.
     * 
     * @param handler the handler to invoke at the proper time
     * @param afterMsecs the number of msecs to invoke the handler
     * @param data the data to be used by the handler
     * @return the id of the registered timer (to be used in cancelTimer)
     */
    TimerId registerTimer(TimerEventHandler *handler,
                          uint64_t afterMsecs,
                          ClientData data);

    /**
     * Cancel a timer event handler.
     * 
     * @param id the id from registerTimer to cancel
     * @return true if the timer was successfully cancelled
     */
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

    /*
     * Helper method that updates the cached representation
     * of a clusterlib repository object and generates the
     * prototypical user event payload to send to clients.
     */
    UserEventPayload *updateCachedObject(CachedObjectEventHandler *cp,
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

    /*
     * Retrieve a list of all (currently known) process slot names
     * within the given node. This also establishes a
     * watch on process slot changes.
     */
    NameList getProcessSlotNames(NodeImpl *node);

    /*
     * Retrieve a list of all (currently known) property list names
     * within the given notifyable. This also establishes a
     * watch on property list changes.
     */
    NameList getPropertyListNames(NotifyableImpl *ntp);

    /*
     * Retrieve a list of all (currently known) queue names
     * within the given notifyable. This also establishes a
     * watch on queue changes.
     */
    NameList getQueueNames(NotifyableImpl *ntp);

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
     * @return the pointer to the root
     */
    RootImpl *getRoot();

    /** 
     * Get a application from the root. 
     *
     * @param name the application name under the root
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the application
     */
    ApplicationImpl *getApplication(const std::string &name,
                                    bool create = false);
    
    /** 
     * Get a group from a parent group key. 
     *
     * @param name the group name under the parent
     * @param parentGroup key to this parent group
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the group
     */
    GroupImpl *getGroup(const std::string &name,
                        GroupImpl *parentGroup,
                        bool create = false);

    /** 
     * Get a node from a parent group key. 
     *
     * @param name the node name under the parent
     * @param parentGroup key to this parent group
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the node
     */
    NodeImpl *getNode(const std::string &name,
                      GroupImpl *parentGroup,
                      bool create = false);

    /** 
     * Get a process slot from a parent node key. 
     *
     * @param name the process slot name under the parent
     * @param parentNode key to this parent node
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the node
     */
    ProcessSlotImpl *getProcessSlot(const std::string &name,
                                    NodeImpl *parentNode,
                                    bool create = false);
    
    /** 
     * Get a data distribution from a parent group key. 
     *
     * @param name the data distribution name under the parent
     * @param parentGroup key to this parent group
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the data distribution
     */
    DataDistributionImpl *getDataDistribution(const std::string &name,
                                              GroupImpl *parentGroup,
                                              bool create = false);

    /** 
     * Get a property list from a parent notifyable key. 
     *
     * @param name the property list name under the parent
     * @param parent key to this parent
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the property list
     */
    PropertyListImpl *getPropertyList(const std::string &name,
                                      Notifyable *parent,
                                      bool create = false);

    /** 
     * Get a queue from a parent notifyable key. 
     *
     * @param name the queue under the parent
     * @param parent key to this parent
     * @param create if true try to create it if it doesn't exist
     * @return NULL if not found or creation failed, otherwise the pointer 
     *         to the queue
     */
    QueueImpl *getQueue(const std::string &name,
                        Notifyable *parent,
                        bool create = false);

    void updateDataDistribution(const std::string &distKey,
                                const std::string &shards,
                                int32_t version,
                                int32_t &finalVersion);
    void updatePropertyList(const std::string &propListKey,
			  const std::string &propListValue,
			  int32_t version,
                          int32_t &finalVersion);
    void updateNodeClientState(const std::string &nodeKey,
                               const std::string &cs);
    void updateNodeClientStateDesc(const std::string &nodeKey,
                                   const std::string &desc);
    void updateNodeMasterSetState(const std::string &nodeKey,
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
     * Get the exact property list represented by this key
     *
     * @param key should represent the PropertyListImpl object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the PropertyListImpl *
     */
    PropertyListImpl *getPropertyListFromKey(const std::string &key,
                                         bool create = false);

    /**
     * Get the exact PropertyListImpl represented by these components.
     *
     * @param components Should represent the PropertyListImpl object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the PropertyListImpl *
     */
    PropertyListImpl *getPropertyListFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        bool create = false);

    /**
     * Get the exact queue represented by this key
     *
     * @param key should represent the QueueImpl object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the QueueImpl *
     */
    QueueImpl *getQueueFromKey(const std::string &key,
                                         bool create = false);

    /**
     * Get the exact QueueImpl represented by these components.
     *
     * @param components Should represent the QueueImpl object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the QueueImpl *
     */
    QueueImpl *getQueueFromComponents(
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

    /**
     * Get the process slot represented exactly by this key
     *
     * @param key should represent the ProcessSlotImpl object
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the NodeImpl *
     */
    ProcessSlotImpl *getProcessSlotFromKey(const std::string &key, 
                                           bool create = false);

    /**
     * Get the exact ProcessSlotImpl represented by these components.
     *
     * @param components Should represent the ProcessSlotImpl object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     * @return NULL if cannot be found, else the ProcessSlotImpl *
     */
    ProcessSlotImpl *getProcessSlotFromComponents(
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
    PropertyListImpl* loadPropertyList(const std::string &propListName,
                                   const std::string &propListKey,
                                   Notifyable *parent);
    QueueImpl* loadQueue(const std::string &queueName,
                         const std::string &queueKey,
                         Notifyable *parent);
    GroupImpl *loadGroup(const std::string &groupName,
                         const std::string &groupKey,
                         GroupImpl *parentGroup);
    NodeImpl *loadNode(const std::string &nodeName,
                       const std::string &nodeKey,
                       GroupImpl *parentGroup);
    ProcessSlotImpl *loadProcessSlot(const std::string &processSlotName,
                                     const std::string &processSlotKey,
                                     NodeImpl *parentNode);

    std::string loadShards(const std::string &key, int32_t &version);
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
        GroupImpl *parentGroup);
    PropertyListImpl *createPropertyList(const std::string &propListName,
                                         const std::string &propListKey,
                                         Notifyable *parent);
    QueueImpl *createQueue(const std::string &queueName,
                           const std::string &queueKey,
                           Notifyable *parent);
    GroupImpl *createGroup(const std::string &groupName,
                           const std::string &groupKey,
                           GroupImpl *parentGroup);
    NodeImpl *createNode(const std::string &nodeName,
                         const std::string &nodeKey,
                         GroupImpl *parentGroup);
    ProcessSlotImpl *createProcessSlot(const std::string &processSlotName,
                                       const std::string &processSlotKey,
                                       NodeImpl *parentNode);
    
    /*
     * Remove entities in Zookeeper.
     */
    void removeApplication(ApplicationImpl *app);
    void removeDataDistribution(DataDistributionImpl *dist);
    void removePropertyList(PropertyListImpl *propList);
    void removeQueue(QueueImpl *queueList);
    void removeGroup(GroupImpl *group);
    void removeNode(NodeImpl *node);
    void removeProcessSlot(ProcessSlotImpl *processSlot);

    /*
     * Remove Notifyable from the factory cache and into
     * m_removedNotifyables (dead pool) that will be cleaned up later.
     */
    void removeNotifyableFromCacheByKey(const std::string &key);

    /*
     * Retrieve Node connected state.
     *
     * @param key the key of the node to check
     * @param id filled in if connected (who connected)
     * @param msecs filled in if connected (when connected)
     * @return true if connected, false otherwise
     */
    bool isNodeConnected(const std::string &key, 
                         std::string &id, 
                         int64_t &msecs);
    std::string getNodeClientState(const std::string &key);
    int32_t getNodeMasterSetState(const std::string &key);

    /*
     * Manage a node's connected state.
     */
    bool createConnected(const std::string &key, const std::string &id);
    void removeConnected(const std::string &key);

    /*
     * Get various locks and conditionals.
     */
    Mutex *getClientsLock();
    Mutex *getPropertyListLock();
    Mutex *getQueueLock();
    Mutex *getDataDistributionsLock();
    Mutex *getRootLock();
    Mutex *getApplicationsLock();
    Mutex *getGroupsLock();
    Mutex *getNodesLock();
    Mutex *getProcessSlotsLock();
    Mutex *getRemovedNotifyablesLock();
    Mutex *getTimersLock();
    Mutex *getSyncEventLock();
    Cond *getSyncEventCond();
    Mutex *getEndEventLock();

    /**
     * Get the removed notifyables set
     */
    std::set<Notifyable *> *getRemovedNotifyables()
    {
        return &m_removedNotifyables;
    }

    /**
     * Increment the sync event id completed
     */
    void incrSyncEventIdCompleted() 
    {
        ++m_syncEventIdCompleted; 
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
    bool isShutdown() const
    {
        return m_shutdown;
    }

    /**
     * Get the CallbackAndContextManager for FactoryOps.
     */
    CallbackAndContextManager *getHandlerAndContextManager()
    {
        return &m_handlerAndContextManager;
    }

    /**
     * Add a response to an id.
     *
     * @param id the id of the request that generated this response
     * @param response the response of the request
     */
    void setIdResponse(const std::string &id, 
                       ::json::JSONValue::JSONObject response);

    /**
     * Get a response from an id.  After this is called, the response
     * object is removed.  It should only be called once per id.
     *
     * @param id the id for the response
     * @return the response object
     */
    ::json::JSONValue::JSONObject getIdResponse(const std::string &id);

    /**
     * Get the sync event signal map
     *
     * @return pointer to the synchronization event signal map
     */
    SignalMap *getSyncEventSignalMap() { return &m_syncEventSignalMap; }

    /**
     * Get the lock signal map
     *
     * @return pointer to the lock signal map.
     */
    SignalMap *getLockEventSignalMap() { return &m_lockEventSignalMap; }

    /**
     * Get the queue signal map
     *
     * @return pointer to the queue signal map.
     */
    SignalMap *getQueueEventSignalMap() { return &m_queueEventSignalMap; }
    
    /**
     * Get the response signal map pointer.
     *
     * @return the pointer to the response signal map
     */
    SignalMap *getResponseSignalMap() 
    {
        return &m_responseSignalMap;
    }

  private:
    /**
     * Clean up clients
     */
    void discardAllClients();

    /**
     * Clean up data distributions
     */
    void discardAllDataDistributions();

    /**
     * Clean up propertyLists
     */
    void discardAllPropertyLists();

    /**
     * Clean up queues
     */
    void discardAllQueues();

    /**
     * Clean up applications
     */
    void discardAllApplications();

    /**
     * Clean up groups
     */
    void discardAllGroups();

    /**
     * Clean up nodes
     */
    void discardAllNodes();

    /**
     * Clean up process slots
     */
    void discardAllProcessSlots();

    /**
     * Clean up all removed notifyables
     */
    void discardAllRemovedNotifyables();

    /**
     * Required to support SAFE_CALL_ZK macro for various classes
     */
    FactoryOps *getOps() { return this; }

    /**
     * Dispatch all events. Reads from the
     * event sources and sends events to
     * the registered client for each event.
     */
    void dispatchExternalEvents(void *param);

    /**
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
     * The registry of cached property lists.
     */
    NotifyableImplMap m_propLists;
    Mutex m_propListsLock;

    /*
     * The registry of cached queues.
     */
    NotifyableImplMap m_queues;
    Mutex m_queuesLock;

    /*
     * The registry of cached data distributions.
     */
    NotifyableImplMap m_dists;
    Mutex m_distsLock;

    /*
     * The registry of cached applications.
     */
    NotifyableImplMap m_apps;
    Mutex m_appsLock;

    /*
     * The registry of cached groups.
     */
    NotifyableImplMap m_groups;
    Mutex m_groupsLock;

    /*
     * The registry of cached nodes.
     */
    NotifyableImplMap m_nodes;
    Mutex m_nodesLock;

    /*
     * The registry of cached process slots.
     */
    NotifyableImplMap m_processSlots;
    Mutex m_processSlotsLock;

    /*
     * The registry of timer handlers.
     */
    TimerRegistry m_timerRegistry;
    Mutex m_timerRegistryLock;

    /*
     * The registry of outstanding sync event operations
     */
    int64_t m_syncEventId;
    int64_t m_syncEventIdCompleted;
    Mutex m_syncEventLock;
    Cond m_syncEventCond;

    /*
     * The registry of removed Notifyables
     */
    std::set<Notifyable *> m_removedNotifyables;
    Mutex m_removedNotifyablesLock;

    /**
     * Protects m_idResponseMap.
     */
    Mutex m_idResponseMapMutex;

    /**
     * Map of JSON-RPC request ids to responses.
     */
    std::map<std::string, ::json::JSONValue::JSONObject> m_idResponseMap;

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
     * Is the event loop terminating?
     */
    volatile bool m_shutdown;

    /**
     * Is the factory connected to ZooKeeper?
     */
    volatile bool m_connected;

    /**
     * Notifies when the first Zookeeper connection has been recognized.
     */
    PredMutexCond m_firstConnect;

    /**
     * Handles all the events for clusterlib objects
     */
    CachedObjectChangeHandlers m_cachedObjectChangeHandlers;

    /**
     * Manages the CachedObjectChangeHandlers allocated by this object
     */
    clusterlib::CallbackAndContextManager m_handlerAndContextManager;

    /**
     * Keeps track of the outstanding sync events waiting to propagate
     * up the queues.
     */
    SignalMap m_syncEventSignalMap;

    /**
     * Keeps track of the notification of (distributed) lock events
     */
    SignalMap m_lockEventSignalMap;

    /**
     * Keeps track of notification of (distributed) queue events
     */
    SignalMap m_queueEventSignalMap;

    /**
     * Keeps track of the notification of JSON-RPC responses.
     */
    SignalMap m_responseSignalMap;

    /**
     * Handles all the locks for clusterlib objects
     */
    DistributedLocks m_distributedLocks;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_FACTORYOPS_H_ */
