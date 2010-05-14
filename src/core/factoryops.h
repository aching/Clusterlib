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
     * @param rpcManager actually invokes the methods to process JSON-RPC
     *        requests
     * @return a Client pointer
     */
    Client *createJSONRPCMethodClient(ClusterlibRPCManager *rpcManager);
    
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

    /**
     * Register a notifyable for use in clusterlib.  A notifyable may
     * only be registered once.  This function will add them to the
     * m_registeredNotifyableMap and make sure the safeNotifyableMap
     * is set for that registered notifyable.  The memory allocated by
     * the pointers will be deallocated in unregisterAllNotifyables().
     * Once registered, the RegisteredNotifyable cannot be
     * unregistered.
     * 
     * @param ntp the RegisteredNotifyable pointer to register.
     */
    void registerNotifyable(RegisteredNotifyable *regNtp);

    /**
     * Clean up the cached notifyable maps.  The SafeNotifyableMap
     * destructors will actually clean up the notifyable heap
     * allocated memory.
     */
    void cleanCachedNotifyableMaps();

    /**
     * Gets a notifyable from the cache or creates it.
     *
     * @param parent pointer to the parent notifyable (NULL if no parent)
     * @param registeredNotifyableName the name of the registered notifyable
     *        the user is looking for 
     * @param name the name of the actual notifyable to get or create
     * @param accessType the access allowed to get this notifyable
     * @return Returns the notifyable pointer or NULL if could not be 
     *         accessed with the given accessType.
     */
    NotifyableImpl *getNotifyable(NotifyableImpl *parent,
                                  const std::string &registeredNotifyableName, 
                                  const std::string &name,
                                  AccessType accessType);

    /*
     * Add and remove clients.
     */
    void addClient(ClientImpl *clp);
    bool removeClient(ClientImpl *clp);

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
    
    /**
     * Dispatch a timer event (used by the dispatchExternalEvents
     * thread) for the consumeTimerEvents thread to pick up.
     *
     * @param tep The pointer to the timer event.
     */
    void dispatchTimerEvent(ClusterlibTimerEvent *tep);

    /**
     * Dispatch a ZK event (used by the dispatchExternalEvents thread)
     * by converting it to a clusterlib event for users.
     *
     * @param zep The pointer to the event to dispatch.o
     */
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
     * Helper method that updates the cached representation
     * of a clusterlib repository object and generates the
     * prototypical user event payload to send to clients.
     *
     * @param cp The cached object event handler to be called
     * @param zep The zookeeper event to convert to clusterlib event
     * @return A payload of the new user-level event (or NULL if none)
     */
    UserEventPayload *updateCachedObject(CachedObjectEventHandler *cp,
                                         zk::ZKWatcherEvent *zep);

    /**
     * Get the names of all the children of a particular Notifyable
     * type (based on the key).  This does not guarantee they will
     * exist after this call, but is a snapshot in time.  It also
     * registers the handler so if the change happens, the handler
     * will be invoked.
     *
     * @param notifyableKey This is the key of where to look for the children
     * @param change This will determine which handler gets called if an event 
     *        happens on the key.
     */
    NameList getChildrenNames(
        const std::string &notifyableKey,
        CachedObjectChangeHandlers::CachedObjectChange change);

    /**
     * After calling getChildrenNames(), this function can be used to
     * take the returned NameList and get all the Notifyable pointers.
     * Note that if the accessType is not CREATE_IF_NOT_FOUND, some of
     * these names may no longer exist if the distributed lock on the
     * parent was not held.
     *
     * @param parent The parent Notifyable of the nameList
     * @param registeredNotifyableName The registered Notifyable name (only
     *        these types of notifyables will be acquired.
     * @param nameList A list of names for this particular type of Notifyable
     * @param accessType The access type.
     * @return A list of Notifyable * that match the type of 
     *         registeredNotifyableName
     */
    NotifyableList getNotifyableList(
        NotifyableImpl *parent,
        const std::string &registeredNotifyableName,
        const NameList &nameList,
        AccessType accessType);
        
    /**
     * Check for valid key from a vector of RegisteredNotifyable objects.
     * 
     * @param registeredNameVec The list of names of 
     *        RegisteredNotifyable objects to check.  If this vector is empty, 
     *        check all registered RegisteredNotifyable objects.
     * @param key A key to test if it matches this RegisteredNotifyable
     * @return True if at least one isValidKey from a RegisteredNotifyable 
     *         isValidKey passes, false otherwise.
     */
    bool isValidKey(const std::vector<std::string> &registeredNameVec,
                    const std::string &key);

    /**
     * Check for valid key from a vector of RegisteredNotifyable objects.
     * 
     * @param registeredNameVec The list of names of 
     *        RegisteredNotifyable objects to check.  If this vector is empty, 
     *        check all registered RegisteredNotifyable objects.
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return True if at least one isValidKey from a RegisteredNotifyable 
     *         isValidKey passes, false otherwise.
     */
    bool isValidKey(const std::vector<std::string> &registeredNameVec,
                    const std::vector<std::string> &components, 
                    int32_t elements = -1);

    /**
     * Notifyables have one key that represent the object name in the
     * Zookeeper repository.  Each notifyable may have zookeeper nodes
     * attached to it (at most one level deep).  Any zookeeper node
     * beyond one level is not part of that object.  This function
     * will get the a key that refers to a Notifyable from any input
     * key.  For example, if the key is
     * .../group/client/nodes/foo-server/connected, it will return
     * .../group/client/nodes/foo-server. If the input key is not
     * related to any Notifyable, return an empty string.
     * 
     * @param key the key that should contain path that is part of a
     *        clusterlib object
     * @return the potential notifyable key, empty if no possible key.
     */
    std::string getNotifyableKeyFromKey(const std::string &key);

    /**
     * Get the notifyable represented by this key. 
     *
     * @param registeredNameVec The list of names of 
     *        RegisteredNotifyable objects to check.  If this vector is empty, 
     *        check all registered RegisteredNotifyable objects.
     * @param key the key that should contain a clusterlib object
     * @param create try to create this object if it does not exist?
     * @return NULL if no Notifyable can be found, else the Notifyable *
     */
    NotifyableImpl *getNotifyableFromKey(
        const std::vector<std::string> &registeredNameVec,
        const std::string &key, 
        AccessType accessType = LOAD_FROM_REPOSITORY);
    
    /**
     * Get the notifyable represented by these components.
     *
     * @param registeredNameVec The list of names of 
     *        RegisteredNotifyable objects to check.  If this vector is empty, 
     *        check all registered RegisteredNotifyable objects.
     * @param components Should represent the Notifyable object
     * @param elements The number of elements to use in the components
                       (-1 for all)
     * @param create try to create this object if it does not exist?
     */
    NotifyableImpl *getNotifyableFromComponents(
        const std::vector<std::string> &registeredNameVec,
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        AccessType accessType = LOAD_FROM_REPOSITORY);
    
    /**
     * Removes the notifyable from the m_cachedNotifyableMap, puts it
     * in the dead pool of m_removedNotifyables and set its to
     * removed.
     *
     * @param ntp pointer to the notifyable to remove
     */
    void removeCachedNotifyable(NotifyableImpl *ntp);
    
    /*
     * Get various locks and conditionals.
     */
    Mutex *getClientsLock();
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

    /**
     * Wait for the FactoryOps threads to be joined.
     */
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

    /**
     * Get a RegisteredNotifyable.  Since all RegisteredNotifyable
     * objects should have been registered in the constructor, the
     * pointer can be safely returned.  They cannot be removed until
     * the FactoryOps destructor.
     *
     * @param registeredName The RegisteredNotifyable name to look for.
     * @param throwIfNotFound Throw an exception is the RegisteredNotifyable 
     *        cannot be found?  If false, will just return NULL.
     * @return Pointer to the RegisteredNotifyable or NULL if cannot be found.
     */
    RegisteredNotifyable *getRegisteredNotifyable(
        const std::string &registeredName, bool throwIfNotFound = false);

    /**
     * Register a new Periodic object.  This Periodic object will be
     * run at regular intervals according to its set frequency.
     *
     * @param periodic The periodic object to start running at regular
     *        intervals.
     */
    void registerPeriodicThread(Periodic &periodic);

    /**
     * Unregister a Periodic object. This will cause it to stop
     * running.
     * 
     * @param periodic The Periodic object to stop.
     * @return True if found and stopped, false otherwise.
     */
    bool cancelPeriodicThread(Periodic &periodic);

  private:
    /**
     * Unregister all registered notifyables.
     */
    void unregisterAllNotifyables();

    /**
     * Clean up clients
     */
    void discardAllClients();

    /**
     * Clean up all Periodic threads.
     */
    void discardAllPeriodicThreads();

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

    /**
     * This method is is run by the threads for each registered
     * Periodic object.  It will run as frequently as the Periodic
     * object dictacts.
     */
    void runPeriodic(void *param);

  private:
    /*
     * The registry of attached clients (and servers).
     */
    ClientImplList m_clients;
    Mutex m_clLock;

    /**
     * The cache of all clusterlib notifyables
     */
    std::map<std::string, SafeNotifyableMap *> m_cachedNotifyableMap;

    /**
     * The lock that protects m_cachedNotifyableMap
     */
    Mutex m_cachedNotifyableMapLock;

    /**
     * All the registered notifyables
     */
    std::map<std::string, RegisteredNotifyable *> m_registeredNotifyableMap;

    /**
     * The lock that protects m_registeredNotifyableMap.
     */
    RdWrLock m_registeredNotifyableMapRdWrLock;

    

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
     * Threads that will be running any user-defined Periodic
     * functions and the PredMutexCond that will coordinate them.
     */
    std::map<Periodic *, CXXThread<FactoryOps> *> m_periodicMap;

    /**
     * Protects m_periodicMap.
     */
    Mutex m_periodicMapLock;

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
