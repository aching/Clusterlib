/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#ifndef __ZKADAPTER_H__
#define __ZKADAPTER_H__

extern "C" {
#include <c-client-src/zookeeper.h>
}

namespace zk {
    
/**
 * \brief A cluster related exception.
 */
class ZooKeeperException :
    public std::exception
{
  public:    
    /**
     * \brief Constructor.
     * 
     * @param msg the detailed message associated with this exception
     */
    ZooKeeperException(const std::string &msg,
                       bool connected = true)
        throw()
        : m_message(msg),
          m_zkErrorCode(0),
          m_connected(connected) {}
    
    /**
     * \brief Constructor.
     * 
     * @param msg the detailed message associated with this exception
     * @param errorCode the ZK error code associated with this exception
     */
    ZooKeeperException(const std::string &msg,
                       int32_t errorCode,
                       bool connected = true)
        throw()
        : m_zkErrorCode(errorCode), m_connected(connected) 
    {
        char tmp[100];
        sprintf( tmp, " (ZK error code: %d)", errorCode );
        m_message = msg + tmp;
    }
    
    /**
     * \brief Destructor.
     */
    ~ZooKeeperException() throw() {}
    
    /**
     * \brief Returns detailed description of the exception.
     */
    const char *what() const throw()
    {
        return m_message.c_str();
    }
    
    /**
     * \brief Returns the ZK error code.
     */
    int32_t getZKErrorCode() const 
    {
        return m_zkErrorCode;
    }
    
    /**
     * \brief Returns whether the cause of the exception is that
     * the ZooKeeper connection is disconnected.
     */
    bool isConnected() 
    {
        return m_connected;
    }
    
  private:
    
    /**
     * The detailed message associated with this exception.
     */
    std::string m_message;
    
    /**
     * The optional error code received from ZK.
     */
    int32_t m_zkErrorCode;
    
    /**
     * Whether the ZooKeeper connection is open.
     */
    bool m_connected;        
};
    
/**
 * \brief This class encapsulates configuration of a ZK client.
 */
class ZooKeeperConfig
{
  public:       
    /**
     * \brief Constructor.
     * 
     * @param hosts the comma separated list of host and port pairs of 
     *              ZK nodes
     * @param leaseTimeout the lease timeout (heartbeat)
     * @param autoReconnect whether to allow for auto-reconnect
     * @param connectTimeout the connect timeout, in milliseconds;
     */
    ZooKeeperConfig(const std::string &hosts, 
                    int32_t leaseTimeout, 
                    bool autoReconnect = true, 
                    int64_t connectTimeout = 10000)
        : m_hosts(hosts), 
          m_leaseTimeout(leaseTimeout), 
          m_autoReconnect(autoReconnect), 
          m_connectTimeout(connectTimeout) {}
        
    /**
     * \brief Returns the list of ZK hosts to connect to.
     */
    std::string getHosts() const { return m_hosts; }
        
    /**
     * \brief Returns the lease timeout.
     */
    int32_t getLeaseTimeout() const { return m_leaseTimeout; }
        
    /**
     * \brief Returns whether {@link ZooKeeperAdapter} should attempt 
     * \brief to automatically reconnect in case of a connection failure.
     */
    bool getAutoReconnect() const { return m_autoReconnect; }

    /**
     * \brief Gets the connect timeout.
     * 
     * @return the connect timeout
     */
    int64_t getConnectTimeout() const { return m_connectTimeout; }
                  
  private:        
    /**
     * The host addresses of ZK nodes.
     */
    const std::string m_hosts;

    /**
     * The ZK lease timeout.
     */
    const int32_t m_leaseTimeout;
        
    /**
     * True if this adapater should attempt to autoreconnect in case 
     * the current session has been dropped.
     */
    const bool m_autoReconnect;
        
    /**
     * How long to wait, in milliseconds, before a connection 
     * is established to ZK.
     */
    const int64_t m_connectTimeout;
        
};

/**
 * \brief A data value object representing a watcher event received
 * from the ZK.
 */
class ZKWatcherEvent
{
    public:
        /**
         * \brief The type representing the user's context.
         */
        typedef void *ContextType;
        
        /**
         * \brief Constructor.
         * 
         * @param type the type of this event
         * @param state the state of this event
         * @param path the corresponding path, may be empty for some event 
         *             types
         * @param context the user specified context; possibly NULL
         */
        ZKWatcherEvent() : 
            m_type(-1), m_state(-1), m_path(""), mp_context(NULL) {}
                        
        /**
         * \brief Constructor.
         * 
         * @param type the type of this event
         * @param state the state of this event
         * @param path the corresponding path, may be empty for some event types
         * @param context the user specified context; possibly NULL
         */
        ZKWatcherEvent(int32_t type,
                       int32_t state,
                       const std::string &path, 
                       ContextType context = NULL) :
            m_type(type), m_state(state), m_path(path), mp_context(context) {}
        
        int32_t getType() const { return m_type; }
        int32_t getState() const { return m_state; }
        std::string const &getPath() const { return m_path; }
        ContextType getContext() const { return mp_context; }
        
        bool operator==(const ZKWatcherEvent &we) const {
            return m_type == we.m_type && m_state == we.m_state 
                    && m_path == we.m_path && mp_context == we.mp_context;
        }
        
    private:
        
        /**
         * The type of this event. It can be either CREATED_EVENT,
         * DELETED_EVENT, CHANGED_EVENT, CHILD_EVENT, SESSION_EVENT or
         * NOTWATCHING_EVENT.  See zookeeper.h for more details.
         */
        const int32_t m_type;
        
        /**
         * The state of ZK at the time of sending this event.
         * It can be either CONNECTING_STATE, ASSOCIATING_STATE, 
         * CONNECTED_STATE, EXPIRED_SESSION_STATE or AUTH_FAILED_STATE.
         * See {@file zookeeper.h} for more details.
         */
        const int32_t m_state;
        
        /**
         * The corresponding path of the node in subject. It may be empty
         * for some event types.
         */
        const std::string m_path;
        
        /**
         * The pointer to the user specified context, possibly NULL.
         */
        ContextType mp_context;
        
};

/**
 * \brief The type definition of ZK event source.
 */
typedef clusterlib::EventSource<ZKWatcherEvent> ZKEventSource;

/**
 * \brief The type definition of ZK event listener.
 */
typedef clusterlib::EventListener<ZKWatcherEvent> ZKEventListener;

/**
 * \brief This is a helper class for handling events using a member function.
 * 
 * This is currently unused.
 */
#ifdef NOTDEF
template<class T>
class EventHandlerWrapper
{
  public:
    /*
     * Signature for embedded method to call on event.
     */
    typedef void (T::*EventHandler)(const ZKWatcherEvent &watcherEvent);

    /*
     * Constructor.
     */
    EventHandlerWrapper(T &obj, EventHandler handler)
        : m_obj(obj),
          m_handler(handler)
    {
    }

    /*
     * Call the embedded method.
     */
    void handleZKEvent(const ZKWatcherEvent &watcherEvent)
    {
        (m_obj.*m_handler)(watcherEvent);
    }
        
  private:
    /*
     * The instance to call.
     */
    T &m_obj;

    /*
     * Pointer to the embedded method.
     */
    EventHandler m_handler;
};
#endif

/**
 * \brief This is a wrapper around ZK C synchrounous API.
 */
class ZooKeeperAdapter
    : public ZKEventSource
{
  public:

    /**
     * Get the event type as a string (used primarily for debugging)
     *
     * @param etype the type passed in to the event
     * @return the stringified etype
     */
    static std::string getEventString(int32_t etype);

    /**
     * Get the state as a string (used primarily for debugging)
     *
     * @param state the state
     * @return the stringified state
     */
    static std::string getStateString(int32_t state);
 
    /**
     * \brief The global function that handles all ZK asynchronous
     * notifications.
     */
    friend void zkWatcher(zhandle_t *, int, int, const char *, void *);
        
    /**
     * \brief The type representing the user's context.
     */
    typedef void *ContextType;
        
    /**
     * \brief All possible states of this client, in respect to 
     * \brief connection to the ZK server.
     */
    enum AdapterState {
        //mp_zkHandle is NULL
        AS_DISCONNECTED = 0,
        //mp_zkHandle is NULL, but no reconnection will be allowed
        AS_NORECONNECT,
        //mp_zkHandle is valid but this client is reconnecting
        AS_CONNECTING,
        //mp_zkHandle is valid and this client is connected
        AS_CONNECTED,
        //mp_zkHandle is valid, however no more calls can be made to ZK API
        AS_SESSION_EXPIRED
    };

    /**
     * \brief Constructor.
     * Attempts to create a ZK adapter, optionally connecting
     * to the ZK. Note, that if the connection is to be established
     * and the given listener is NULL, some events may be lost, 
     * as they may arrive asynchronously before this method finishes.
     * 
     * @param config the ZK configuration
     * @param listener the event listener to be used for listening 
     *                 on incoming ZK events;
     *                 if <code>NULL</code> not used
     * @param establishConnection whether to establish connection to 
     *                            the ZK
     * 
     * @throw ZooKeeperException if cannot establish connection to the 
     *                           given ZK
     */
    ZooKeeperAdapter(ZooKeeperConfig config, 
                     ZKEventListener *listener = NULL,
                     bool establishConnection = false);

    /**
     * \brief Destructor.
     */
    ~ZooKeeperAdapter(); 
                  
    /**
     * \brief Returns the current config.
     */
    const ZooKeeperConfig &getZooKeeperConfig() const {
        return m_zkConfig;                      
    }

    /**
     * \brief Restablishes connection to the ZK. 
     * If this adapter is already connected, the current connection 
     * will be dropped and a new connection will be established.
     * 
     * @throw ZooKeeperException if cannot establish connection to the ZK
     */
    void reconnect();
        
    /**
     * \brief Disconnects from the ZK and unregisters {@link #mp_zkHandle}. 
     *
     * @param final if true, no reconnection will be allowed.
     */
    void disconnect(bool final = false);

    /**
     * \brief Stops the ZK event loop from dispatching events.
     */
    void stopEventDispatch();

    /**
     * \brief Synchronizes all events with ZK with the local server.
     */
    bool sync(const std::string &path,
              ZKEventListener *listener,
              void *context);

    /**
     * \brief Creates a new node identified by the given path. 
     * This method will optionally attempt to create all missing ancestors.
     * 
     * @param path the absolute path name of the node to be created
     * @param value the initial value to be associated with the node
     * @param flags the ZK flags of the node to be created
     * @param createAncestors if true and there are some missing ancestor 
     *                        nodes, this method will attempt to 
     *                        create them
     * 
     * @return true if the node has been successfully created; false
     *              otherwise
     * @throw ZooKeeperException if the operation has failed
     */ 
    bool createNode(const std::string &path, 
                    const std::string &value = "", 
                    int flags = 0, 
                    bool createAncestors = true);
                  
    /**
     * \brief Creates a new sequence node using the give path as
     * the prefix.  This method will optionally attempt to create
     * all missing ancestors.
     * 
     * @param path the absolute path name of the node to be created; 
     * @param value the initial value to be associated with the node
     * @param flags the ZK flags of the sequence node to be created 
     *              (in addition to SEQUENCE)
     * @param createAncestors if true and there are some missing ancestor 
     *                        nodes, this method will attempt to
     *                        create them
     * @param createdPath this is set to the actual created path if the 
     *                    call is successful
     * 
     * @return the sequence number associate with newly created node,
     *         or -1 if it couldn't be created
     * @throw ZooKeeperException if the operation has failed
     */ 
    int64_t createSequence(const std::string &path, 
                           const std::string &value,
                           int flags, 
                           bool createAncestors,
                           std::string &createdPath);
        
    /**
     * \brief Deletes a node identified by the given path.
     * 
     * @param path the absolute path name of the node to be deleted
     * @param recursive if true this method will attempt to remove 
     *                  all children of the given node if any exist
     * @param version the expected version of the node. The function will 
     *                fail if the actual version of the node does not 
     *                match the expected version
     * 
     * @return true if the node has been deleted; false otherwise
     * @throw ZooKeeperException if the operation has failed
     */
    bool deleteNode(const std::string &path,
                    bool recursive = false,
                    int version = -1);
        
    /**
     * \brief Checks whether the given node exists or not.
     * 
     * @param path the absolute path name of the node to be checked
     * @param listener the listener for ZK watcher events; 
     *                 passing non <code>NULL</code> effectively establishes
     *                 a ZK watch on the given node
     * @param context the user specified context that is to be passed
     *                in a corresponding {@link ZKWatcherEvent} at later time; 
     *                not used if <code>listener</code> is <code>NULL</code>
     * @param stat the optional node statistics to be filled in by ZK
     * 
     * @return true if the given node exists; false otherwise
     * @throw ZooKeeperException if the operation has failed
     */
    bool nodeExists(const std::string &path, 
                    ZKEventListener *listener = NULL, 
                    void *context = NULL,
                    Stat *stat = NULL);

    /**
     * \brief Retrieves list of all children of the given node.
     * 
     * Gets the data from the node.  If the listener is set, then a
     * watch is set on the node.  Context can be passed along with the
     * event, but is not required.
     * 
     * @param path the absolute path name of the node for which to 
     *             get children
     * @param listener the listener for ZK watcher events; 
     *                 passing non <code>NULL</code> effectively 
     *                 establishes
     *                 a ZK watch on the given node
     * @param context the user specified context that is to be passed
     *                in a corresponding {@link ZKWatcherEvent} at later
     *                time; not used if <code>listener</code> is
     *                <code>NULL</code>
     * 
     * @return the list of absolute paths of child nodes, possibly empty
     * @throw ZooKeeperException if the operation has failed
     */
    void getNodeChildren(std::vector<std::string> &children,
                         const std::string &path, 
                         ZKEventListener *listener = NULL, 
                         void *context = NULL);
                
    /**
     * \brief Gets the given node's data.
     * 
     * @param path the absolute path name of the node to get data from
     * @param listener the listener for ZK watcher events; 
     *                 passing non <code>NULL</code> effectively 
     *                 establishes a ZK watch on the given node
     * @param context the user specified context that is to be passed
     *                in a corresponding {@link ZKWatcherEvent} at later time; 
     *                not used if <code>listener</code> is <code>NULL</code>
     * @param stat the optional node statistics to be filled in by ZK
     * 
     * @return the node's data
     * @throw ZooKeeperException if the operation has failed
     */
    std::string getNodeData(const std::string &path, 
                            ZKEventListener *listener = NULL, 
                            void *context = NULL,
                            Stat *stat = NULL);
        
    /**
     * \brief Sets the given node's data.
     * 
     * @param path the absolute path name of the node to get data from
     * @param value the node's data to be set
     * @param version the expected version of the node. The function will 
     *                fail if the actual version of the node does not match 
     *                the expected version
     * 
     * @throw ZooKeeperException if the operation has failed
     */
    void setNodeData(const std::string &path, 
                     const std::string &value, 
                     int version = -1,
                     Stat *stat = NULL);
        
    /**
     * \brief Validates the given path to a node in ZK.
     * 
     * @param the path to be validated
     * 
     * @throw ZooKeeperException if the given path is not valid
     *        (for instance it doesn't start with "/")
     */
    void validatePath(const std::string &path);

    /**
     * Returns the current state of this adapter.
     * 
     * @return the current state of this adapter
     * @see AdapterState
     */
    AdapterState getState() const {
        return m_state;
    }          
        
  private:
        
    /**
     * This enum defines methods from this class than can trigger an event.
     */
    enum WatchableMethod {
        NODE_EXISTS = 0,
        GET_NODE_CHILDREN,
        GET_NODE_DATA,
        SYNC_DATA
    };

    /**
     * Data structure to pass in as watcherCtx for Zookeeper zoo_w*()
     * functions.  This struct should only be memory managed using
     * CreateUserContextAndListener() and DeleteUserContextAndListener().
     */
    struct UserContextAndListener {
        UserContextAndListener(void *userContextParam, 
                               ZKEventListener *listenerParam) 
            : userContext(userContextParam), listener(listenerParam) {}
        
        /**
         * Context passed in from the user.
         */
        void *userContext;
        
        /**
         * Listener passed in by the user.  If NULL, fire this to all
         * listeners.
         */
        ZKEventListener *listener;
    };
                                
    /**
     * \brief Creates a new node identified by the given path. 
     * This method is used internally to implement {@link createNode(...)} 
     * and {@link createSequence(...)}. On success, this method will set
     * <code>createdPath</code>.
     * 
     * @param path the absolute path name of the node to be created
     * @param value the initial value to be associated with the node
     * @param flags the ZK flags of the node to be created
     * @param createAncestors if true and there are some missing ancestor 
     *        nodes, this method will attempt to create them
     * @param createdPath the actual path of the node that has been 
     *        created; useful for sequences
     * 
     * @return true if the node has been successfully created; false 
     *         otherwise
     * @throw ZooKeeperException if the operation has failed
     */ 
    bool createNode(const std::string &path, 
                    const std::string &value, 
                    int flags, 
                    bool createAncestors,
                    std::string &createdPath);
        
    /**
     * Handles an asynchronous event received from the ZK.
     */
    void handleAsyncEvent(const ZKWatcherEvent &event);
        
    /**
     * \brief Enqueues the given event in {@link #m_events} queue.
     */
    void enqueueEvent(int type, 
                      int state, 
                      const std::string &path, 
                      ContextType context);
        
    /**
     * \brief Processes all ZK adapter events in a loop.
     */
    void processEvents(void *param);

    /**
     * \brief Processes all user events in a loop.
     */
    void processUserEvents(void *param);

    /**
     * Sets the new state in case it's different then the current one.
     * This method assumes that {@link #m_stateLock} has been already locked.
     * 
     * @param newState the new state to be set
     */
    void setState(AdapterState newState); 
        
    /**
     * Waits until this client gets connected. The total wait time 
     * is given by {@link getRemainingConnectTimeout()}.
     * If a timeout elapses, this method will throw an exception.
     * 
     * @throw ZooKeeperException if unable to connect within the given timeout
     */
    void waitUntilConnected();
                                      
    /**
     * Verifies whether the connection is established,
     * optionally auto reconnecting.
     * 
     * @throw ZooKeeperConnection if this client is disconnected
     *        and auto-reconnect failed or was not allowed
     */
    void verifyConnection();

    /**
     * Returns the remaining connect timeout. The timeout resets
     * to {@link #m_connectTimeout} on a successfull connection to the ZK.
     * 
     * @return the remaining connect timeout, in milliseconds
     */
    long long int getRemainingConnectTimeout() { 
        return m_remainingConnectTimeout; 
    }
        
    /**
     * Resets the remaining connect timeout to {@link #m_connectTimeout}.
     */
    void resetRemainingConnectTimeout() { 
        m_remainingConnectTimeout = m_zkConfig.getConnectTimeout(); 
    }
        
    /**
     * Updates the remaining connect timeout to reflect the given wait time.
     * 
     * @param time the time for how long waited so far on connect to succeed
     */
    void waitedForConnect(long long time) { 
        m_remainingConnectTimeout -= time; 
    }

    /**
     * \brief Simulate a SESSION_EXPIRED event so that the connection ends.
     */
    void injectEndEvent();
        
    /**
     * Is this an end event? 
     *
     * @return true if this is an end event
     */
    bool isEndEvent(const ZKWatcherEvent &event) const;

    /**
     * Gets the lock that makes {@link #m_userContextAndListenerSet} 
     * thread-safe.
     *
     * @return pointer to the lock
     */
    clusterlib::Mutex *getUserContextAndListenerSetLock()
    {
        return &m_userContextAndListenerSetLock;
    }

    /**
     * Allocate the memory and setup with UserContextAndListener.
     * This should be the only way to allocate memory for
     * UserContextAndListener so it can be tracked by
     * ZooKeeperAdapter.
     *
     * @param userContextAndListener the pointer to track
     */
    UserContextAndListener *createUserContextAndListener(
        void *userContext,
        ZKEventListener *listener);
   
    /**
     * Free the memory associated with the UserContextAndListener and
     * remove it from being tracked.
     */
    void deleteUserContextAndListener(
        UserContextAndListener *userContextAndListener);

  private:
        
    /**
     * The current ZK configuration.
     */
    const ZooKeeperConfig m_zkConfig;

    /**
     * The current ZK session.
     */
    zhandle_t *mp_zkHandle;
        
    /**
     * The blocking queue of all events waiting to be processed by ZK adapter.
     */
    clusterlib::BlockingQueue<ZKWatcherEvent> m_events;
        
    /**
     * The blocking queue of all events waiting to be processed by users
     * of ZK adapter.
     */
    clusterlib::BlockingQueue<ZKWatcherEvent> m_userEvents;
        
    /**
     * The thread that dispatches all events from {@link #m_events} queue.
     */
    clusterlib::CXXThread<ZooKeeperAdapter> m_eventDispatcher;

    /**
     * The thread that dispatches all events from {@link #m_userEvents} queue.
     */
    clusterlib::CXXThread<ZooKeeperAdapter> m_userEventDispatcher;
                
    /**
     * Whether this adapter is connected to the ZK.
     */
    volatile bool m_connected;
        
    /**
     * The state of this adapter.
     */
    AdapterState m_state;

    /**
     * Is event dispatch allowed? (default == true)
     */
    bool m_eventDispatchAllowed;
        
    /**
     * The lock used to synchronize access to {@link #m_state}.
     */
    clusterlib::Lock m_stateLock;

    /**
     * Makes {@link #m_userContextAndListenerSet} thread-safe.
     */
    clusterlib::Mutex m_userContextAndListenerSetLock;
    
    /**
     * Contains pointers to all the UserContextAndListeners that are
     * on the heap.
     */
    std::set<UserContextAndListener *> m_userContextAndListenerSet;
    
    /**
     * How much time left for the connect to succeed, in milliseconds.
     */
    long long int m_remainingConnectTimeout;
                
};
        
}   /* end of 'namespace zk' */

#endif /* __ZKADAPTER_H__ */
