/*
 * factoryops.cc --
 *
 * Implementation of the FactoryOps class.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#include <sys/types.h>
#include <linux/unistd.h>

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib
{

FactoryOps::FactoryOps(const string &registry, int64_t connectTimeout)
    : m_syncEventId(0),
      m_syncEventIdCompleted(0),
      m_endEventDispatched(false),
      m_config(registry, connectTimeout, true), 
      m_zk(m_config, NULL, false),
      m_timerEventAdapter(m_timerEventSrc),
      m_zkEventAdapter(m_zk),
      m_shutdown(false),
      m_connected(false),
      m_cachedObjectChangeHandlers(this),
      m_distributedLocks(this)
{
    TRACE(CL_LOG, "FactoryOps");

    /*
     * Link up the event sources.
     */
    m_timerEventAdapter.addListener(&m_externalEventAdapter);
    m_zkEventAdapter.addListener(&m_externalEventAdapter);

    /*
     * Create the clusterlib event dispatch thread (processes only
     * events that are visible to clusterlib clients)
    */
    m_externalEventThread.Create(
        *this, 
        &FactoryOps::dispatchExternalEvents);

    /*
     * Create the timer handler thread.
     */
    m_timerHandlerThread.Create(*this, &FactoryOps::consumeTimerEvents);

    /*
     * Connect to ZK.
     */
    try {
        m_zk.reconnect();
        LOG_INFO(CL_LOG, 
                 "Waiting for connect event from ZooKeeper up to %" PRId64 
                 " msecs, thread: %" PRIu32,
                 connectTimeout,
                 ProcessThreadService::getTid());
        if (m_firstConnect.predWaitUsecs(connectTimeout * 1000) == false) {
            LOG_ERROR(CL_LOG,
                      "FactoryOps: Did not receive connect event from %s in "
                      "time (%" PRId64 " msecs), aborting",
                      m_config.getHosts().c_str(),
                      connectTimeout);
	    throw RepositoryConnectionFailureException(
		"Did not receive connect event in time, aborting");
        }
        LOG_INFO(CL_LOG, 
                 "After wait, m_connected == %d",
                 static_cast<int>(m_connected));
    } catch (zk::ZooKeeperException &e) {
        m_zk.disconnect();
        LOG_FATAL(CL_LOG, 
                  "Failed to connect to Zookeeper (%s)",
                  m_config.getHosts().c_str());
        throw RepositoryInternalsFailureException(e.what());
    }

    /*
     * Register all the clusterlib objects.
     */
    RegisteredNotifyable *regNtp = NULL;
    regNtp = new RegisteredRootImpl(this);
    registerNotifyable(regNtp);
    regNtp = new RegisteredApplicationImpl(this);
    registerNotifyable(regNtp);
    regNtp = new RegisteredGroupImpl(this);
    registerNotifyable(regNtp);
    regNtp = new RegisteredNodeImpl(this);
    registerNotifyable(regNtp);
    regNtp = new RegisteredProcessSlotImpl(this);
    registerNotifyable(regNtp);
    regNtp = new RegisteredDataDistributionImpl(this);
    registerNotifyable(regNtp);
    regNtp = new RegisteredPropertyListImpl(this);
    registerNotifyable(regNtp);
    regNtp = new RegisteredQueueImpl(this);
    registerNotifyable(regNtp);
    
    /*
     * After all other initializations, get/create the root object.
     */
    getNotifyable(NULL,
                  ClusterlibStrings::REGISTERED_ROOT_NAME, 
                  ClusterlibStrings::ROOT,
                  CREATE_IF_NOT_FOUND);
}

void
FactoryOps::registerNotifyable(RegisteredNotifyable *regNtp)
{
    {
        WriteLocker l(&m_registeredNotifyableMapRdWrLock);
        
        map<string, RegisteredNotifyable *>::const_iterator 
            registerNotifyableMapIt =
            m_registeredNotifyableMap.find(regNtp->registeredName());
        if (registerNotifyableMapIt != m_registeredNotifyableMap.end()) {
            ostringstream oss;
            oss << "registerNotifyable: Already has object registeredName="
                << regNtp->registeredName() << " in m_registeredNotifyableMap";
            throw InvalidArgumentsException(oss.str());
        }
        m_registeredNotifyableMap[regNtp->registeredName()] = regNtp;
    }

    {
        Locker l(&m_cachedNotifyableMapLock);
        map<string, SafeNotifyableMap *>::const_iterator cachedNotifyableMapIt
            = m_cachedNotifyableMap.find(regNtp->registeredName());
        if (cachedNotifyableMapIt != m_cachedNotifyableMap.end()) {
            ostringstream oss;
            oss << "registerNotifyable: Already has object registeredName="
                << regNtp->registeredName() << " in m_cachedNotifyableMap";
            throw InvalidArgumentsException(oss.str());
        }
        SafeNotifyableMap *safeNotifyableMap = new SafeNotifyableMap();
        m_cachedNotifyableMap.insert(
            pair<string, SafeNotifyableMap *>(regNtp->registeredName(), 
                                              safeNotifyableMap));
        regNtp->setSafeNotifyableMap(*safeNotifyableMap);
    }
}

void
FactoryOps::unregisterAllNotifyables()
{
    WriteLocker l(&m_registeredNotifyableMapRdWrLock);
    
    map<string, RegisteredNotifyable *>::iterator registerNotifyableMapIt;
    while (!m_registeredNotifyableMap.empty()) {
        registerNotifyableMapIt = m_registeredNotifyableMap.begin();
        delete registerNotifyableMapIt->second;
        m_registeredNotifyableMap.erase(registerNotifyableMapIt);
    }
}

void
FactoryOps::cleanCachedNotifyableMaps()
{
    Locker l(&m_cachedNotifyableMapLock);

    map<string, SafeNotifyableMap *>::iterator cachedNotifyableMapIt;
    while (!m_cachedNotifyableMap.empty()) {
        cachedNotifyableMapIt = m_cachedNotifyableMap.begin();
        delete cachedNotifyableMapIt->second;
        m_cachedNotifyableMap.erase(cachedNotifyableMapIt);
    }
}

/*
 * Destructor of FactoryOps
 */
FactoryOps::~FactoryOps()
{
    TRACE(CL_LOG, "~FactoryOps");

    /*
     * Zookeeper will not deliver any more events after the end event
     * is propagated to the cache and then to the clients.  All event
     * will be fired until the final ZOO_SESSION_EXPIRED event.  As
     * this cascades through the the event handler threads, they
     * deliver and then exit.
     */
    m_zk.injectEndEvent();

    /*
     * Allow our threads to shut down.
     */
    waitForThreads();

    discardAllPeriodicThreads();

    discardAllClients();
    discardAllRemovedNotifyables();

    unregisterAllNotifyables();
    cleanCachedNotifyableMaps();

    try {
        m_zk.disconnect(true);
    } catch (zk::ZooKeeperException &e) {
        LOG_WARN(CL_LOG,
                 "Got exception during disconnect: %s",
                 e.what());
    }

    /* Clean up any contexts that are being waited on. */
    m_handlerAndContextManager.deleteAllCallbackAndContext();

    /*
     * Unlink the event sources prior to their destructor being called
     * or else events may be propagated to missing listeners through
     * fireEvent().
     */
    m_zk.removeListener(&m_zkEventAdapter);
    m_timerEventSrc.removeListener(&m_timerEventAdapter);
    m_timerEventAdapter.removeListener(&m_externalEventAdapter);
    m_zkEventAdapter.removeListener(&m_externalEventAdapter);
}

void
FactoryOps::waitForThreads()
{
    TRACE(CL_LOG, "waitForThreads");

    m_timerHandlerThread.Join();
    m_externalEventThread.Join();
}

Client *
FactoryOps::createClient()
{
    TRACE(CL_LOG, "createClient");

    /*
     * Ensure we're connected.
     */
    if (!m_connected) {
        LOG_ERROR(CL_LOG, "Cannot create client when disconnected.");
        return NULL;
    }
    
    /*
     * Create the new client and add it to the registry.
     */
    ClientImpl *cp = new ClientImpl(this);
    addClient(cp);
    return cp;
}

Client *
FactoryOps::createJSONRPCResponseClient(Queue *responseQueue,
                                        Queue *completedQueue)
{
    TRACE(CL_LOG, "createJSONRPCResponseClient");

    ClientImpl *client = dynamic_cast<ClientImpl *>(createClient());
    if (client == NULL) {
        throw InconsistentInternalStateException(
            "createJSONRPCResponseClient: Failed to create client");
    }
    client->registerJSONRPCResponseHandler(responseQueue,
                                           completedQueue);
    return client;
}

Client *
FactoryOps::createJSONRPCMethodClient(
    ClusterlibRPCManager *rpcManager)
{
    TRACE(CL_LOG, "createJSONRPCMethodClient");

    ClientImpl *client = dynamic_cast<ClientImpl *>(createClient());
    if (client == NULL) {
        throw InconsistentInternalStateException(
            "createJSONRPCMethodClient: Failed to create client");
    }
    client->registerJSONRPCMethodHandler(rpcManager);
    return client;
}

bool
FactoryOps::isConnected() const
{
    return m_connected;
}

void
FactoryOps::synchronize()
{
    TRACE(CL_LOG, "synchronize");
    int64_t syncEventId = 0;

    /* 
     * Simple algorithm to ensure that each synchronize() called by
     * various clients gets a unique ID.  As long as there are not
     * 2^64 sync operations in progress, this will not be a problem.
     */
    {
        Locker l1(getSyncEventLock());
        if ((m_syncEventId < m_syncEventIdCompleted) ||
            (m_syncEventId == numeric_limits<int64_t>::max())) {
            throw InconsistentInternalStateException(
                "synchronize: sync invariant not maintained");
        }
        /*
         * Reset m_syncEventId and m_syncEventIdCompleted if there are no
         * outstanding syncs and are not at the initial state. 
         */
        if ((m_syncEventId == m_syncEventIdCompleted) && 
            (m_syncEventId != 0)) {
            LOG_DEBUG(CL_LOG,
                      "synchronize: Resetting the sync event id and "
                      "sync event id completed");
            m_syncEventId = 0;
            m_syncEventIdCompleted = 0;
        }
        ++m_syncEventId;
        syncEventId = m_syncEventId;
    }

    string key(ClusterlibStrings::ROOTNODE);
    key.append(ClusterlibStrings::CLUSTERLIB);

    string syncEventKey = 
        NotifyableKeyManipulator::createSyncEventKey(syncEventId);

    /*
     * Pass the sync event ID to the ZooKeeperAdapter.  When the event
     * comes back up to clusterlib, it will find the id and then try
     * to match it to this one to ensure that this sync finished.
     */
    getSyncEventSignalMap()->addRefPredMutexCond(syncEventKey);
    CallbackAndContext *callbackAndContext = 
        getHandlerAndContextManager()->createCallbackAndContext(
            getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::SYNCHRONIZE_CHANGE),
            &syncEventId);

    LOG_DEBUG(CL_LOG, 
              "synchronize: Starting sync with event id (%" PRId64 ")", 
              syncEventId);
    SAFE_CALL_ZK(m_zk.sync(
                     key, 
                     &m_zkEventAdapter, 
                     callbackAndContext),
                 "Could not synchronize with the underlying store %s: %s",
                 "/",
                 false,
                 true);

    /* 
     * Wait for this sync event to be processed through the clusterlib
     * external event thread. 
     */
    getSyncEventSignalMap()->waitUsecsPredMutexCond(syncEventKey, -1);
    getSyncEventSignalMap()->removeRefPredMutexCond(syncEventKey);

    LOG_DEBUG(CL_LOG, 
              "synchronize: event id (%" PRId64 ") Complete", 
              syncEventId);
}

/**********************************************************************/
/* Below this line are the methods of class FactoryOps that provide
 * functionality beyond what Factory needs.  */
/**********************************************************************/

/*
 * Add and remove clients.
 */
void
FactoryOps::addClient(ClientImpl *clp)
{
    TRACE(CL_LOG, "addClient");

    Locker l(getClientsLock());
    
    m_clients.push_back(clp);
}

bool
FactoryOps::removeClient(ClientImpl *clp)
{
    TRACE(CL_LOG, "removeClient");

    Locker l(getClientsLock());
    ClientImplList::iterator clIt = find(m_clients.begin(),
                                         m_clients.end(),
                                         clp);
    if (clIt == m_clients.end()) {
        return false;
    }
    else {
        m_clients.erase(clIt);
        delete clp;
        return true;
    }
}

RegisteredNotifyable *
FactoryOps::getRegisteredNotifyable(const string &registeredName, 
                                    bool throwIfNotFound)
{
    TRACE(CL_LOG, "getRegisteredNotifyable");

    map<string, RegisteredNotifyable *>::const_iterator registeredNotifyableIt;

    ReadLocker l(&m_registeredNotifyableMapRdWrLock);

    registeredNotifyableIt = m_registeredNotifyableMap.find(registeredName);
    if (registeredNotifyableIt == m_registeredNotifyableMap.end()) {
        if (throwIfNotFound) {
            throw InconsistentInternalStateException(
                string("getRegisteredNotifyable: Couldn't find ") + 
                registeredName);
        }
        return NULL;
    }
    else {
        return registeredNotifyableIt->second;
    }
}

void
FactoryOps::runPeriodic(void *param)
{
    TRACE(CL_LOG, "runPeriodic");

    LOG_INFO(CL_LOG,
             "Starting thread with FactoryOps::runPeriodic(), "
             "this: %p, thread: %" PRIu32,
             this,
             ProcessThreadService::getTid());
    
    m_periodicMapLock.acquire();
        
    Periodic *periodic = reinterpret_cast<Periodic *>(param);
    map<Periodic *, CXXThread<FactoryOps> *>::const_iterator periodicMapIt
        = m_periodicMap.find(periodic);
    if (periodicMapIt == m_periodicMap.end()) {
        throw InconsistentInternalStateException(
            "runPeriodic: Couldn't find thread");
    }
    PredMutexCond &predMutexCondRef = 
        periodicMapIt->second->getPredMutexCond();

    m_periodicMapLock.release();
    
    try {
        do {
            LOG_DEBUG(CL_LOG, 
                      "runPeriodic: thread: %" PRIu32 " doing run() "
                      "after waiting %" PRId64,
                      ProcessThreadService::getTid(),
                      periodic->getMsecsFrequency());
            periodic->run();
        } while (predMutexCondRef.predWaitMsecs(periodic->getMsecsFrequency()) 
                 == false);
    }
    catch (Exception e) {
        LOG_ERROR(CL_LOG, 
                  "runPeriodic failed with Exception %s", 
                  e.what());
        throw e;
    }

    LOG_INFO(CL_LOG,
             "Ending thread with FactoryOps::runPeriodic(): "
             "this: %p, thread: %" PRIu32,
             this,
             ProcessThreadService::getTid());
}

void
FactoryOps::registerPeriodicThread(Periodic &periodic)
{
    TRACE(CL_LOG, "registerPeriodicThread");

    Locker l(&m_periodicMapLock);

    map<Periodic *, CXXThread<FactoryOps> *>::const_iterator periodicMapIt =
        m_periodicMap.find(&periodic);
    if (periodicMapIt != m_periodicMap.end()) {
        throw InvalidArgumentsException(
            "registerPeriodicThread: This Periodic object was "
            "already registered");
    }

    m_periodicMap[&periodic] = new CXXThread<FactoryOps>(); 
    m_periodicMap[&periodic]->Create(
        *this, &FactoryOps::runPeriodic, &periodic);
}

bool
FactoryOps::cancelPeriodicThread(Periodic &periodic)
{
    TRACE(CL_LOG, "cancelPeriodicThread");
    
    Locker l(&m_periodicMapLock);

    map<Periodic *, CXXThread<FactoryOps> *>::iterator periodicMapIt =
        m_periodicMap.find(&periodic);
    if (periodicMapIt == m_periodicMap.end()) {
        return false;
    }

    periodicMapIt->second->getPredMutexCond().predSignal();
    periodicMapIt->second->Join();
    m_periodicMap.erase(periodicMapIt);
    delete periodicMapIt->second;

    return true;
}

void
FactoryOps::discardAllClients()
{
    TRACE(CL_LOG, "discardAllClients");

    Locker l(getClientsLock());
    ClientImplList::iterator clIt;
    for (clIt  = m_clients.begin();
         clIt != m_clients.end();
         ++clIt) {
	delete *clIt;
    }
    m_clients.clear();
}

void
FactoryOps::discardAllPeriodicThreads()
{
    TRACE(CL_LOG, "discardAllPeriodicThreads");

    Locker l(&m_periodicMapLock);

    map<Periodic *, CXXThread<FactoryOps> *>::iterator periodicMapIt;
    while (!m_periodicMap.empty()) {
        periodicMapIt = m_periodicMap.begin();
        periodicMapIt->second->getPredMutexCond().predSignal();
        periodicMapIt->second->Join();
        m_periodicMap.erase(periodicMapIt);
        delete periodicMapIt->second;
    }
}

void
FactoryOps::discardAllRemovedNotifyables()
{
    TRACE(CL_LOG, "discardAllRemovedNotifyables");

    Locker l(getRemovedNotifyablesLock());
    set<Notifyable *>::iterator ntpIt;

    int32_t count = 0;
    for (ntpIt = m_removedNotifyables.begin();
         ntpIt != m_removedNotifyables.end();
         ntpIt++) {
        LOG_DEBUG(CL_LOG, 
                  "discardAllRemovedNotifyables: "
                  "Removed key (%s) with %d refs",
                  (*ntpIt)->getKey().c_str(),
                  (*ntpIt)->getRefCount());
	delete *ntpIt;
        count++;
    }
    m_removedNotifyables.clear();

    LOG_INFO(CL_LOG, 
             "discardAllRemovedNotifyables: Cleaned up %d removed notifyables",
             count);
}

/*
 * Dispatch events to all registered clients.
 */
void
FactoryOps::dispatchExternalEvents(void *param)
{
    TRACE(CL_LOG, "dispatchExternalEvents");

    uint32_t eventSeqId = 0;
    LOG_INFO(CL_LOG,
             "Starting thread with FactoryOps::dispatchExternalEvents(), "
             "this: %p, thread: %" PRIu32,
             this,
             ProcessThreadService::getTid());

    try {
        while (m_shutdown == false) { 
            LOG_INFO(CL_LOG,
                     "[%d]: Asking for next event",
                     eventSeqId);
            eventSeqId++;

            /*
             * Get the next event and send it off to the
             * correct handler.
             */
            GenericEvent ge;
            m_externalEventAdapter.getNextEvent(ge);

            LOG_DEBUG(CL_LOG,
                      "[%" PRIu32 ", %p] dispatchExternalEvents() received "
                      "generic event of type: %s",
                      eventSeqId,
                      this,
                      GenericEvent::getTypeString(ge.getType()).c_str());
            
            switch (ge.getType()) {
                case ILLEGALEVENT:
                    LOG_FATAL(CL_LOG, "Illegal event");
                    throw InconsistentInternalStateException(
                        "dispatchExternalEvents: Illegal event");
                case TIMEREVENT:
                    {
                        ClusterlibTimerEvent *tp =
                            (ClusterlibTimerEvent *) ge.getEvent();
                        
                        LOG_DEBUG(CL_LOG,
                                  "Dispatching timer event: %p, id: "
                                  "%d, alarm time: %" PRId64,
                                  tp,
                                  tp->getID(),
                                  tp->getAlarmTime());
                        
                        dispatchTimerEvent(tp);
                    }
                    break;
                case ZKEVENT:
                    {
                        zk::ZKWatcherEvent *zp =
                            (zk::ZKWatcherEvent *) ge.getEvent();
                        
                        LOG_DEBUG(CL_LOG,
                                  "Processing ZK event (type: %s, state: %d, "
                                  "context: %p, path: %s)",
                                  zk::ZooKeeperAdapter::getEventString(
                                      zp->getType()).c_str(),
                                  zp->getState(),
                                  zp->getContext(),
                                  zp->getPath().c_str());
                        
                        if ((zp->getType() == ZOO_SESSION_EVENT) &&
                            (zp->getPath().compare(ClusterlibStrings::SYNC) 
                             != 0)) {
                            dispatchSessionEvent(zp);
                        } 
                        else {
                            dispatchZKEvent(zp);
                        }
                    }
                    break;
                default:
                    LOG_FATAL(CL_LOG,
                              "Illegal event with type %d",
                              ge.getType());
                    throw InconsistentInternalStateException(
                        "dispatchExternalEvents: Unknown event");
            }
        }

        /*
         * After the event loop, we inform all
         * registered clients that there will
         * be no more events coming.
         */
        dispatchEndEvent();

    } catch (zk::ZooKeeperException &zke) {
        LOG_ERROR(CL_LOG, "ZooKeeperException: %s", zke.what());
        dispatchEndEvent();
        throw RepositoryInternalsFailureException(zke.what());
    } catch (Exception &e) {
        dispatchEndEvent();
        throw Exception(e.what());
    } catch (std::exception &stde) {
        LOG_ERROR(CL_LOG, "Unknown exception: %s", stde.what());
        dispatchEndEvent();
        throw Exception(stde.what());
    }

    LOG_INFO(CL_LOG,
             "Ending thread with FactoryOps::dispatchExternalEvents(): "
             "this: %p, thread: %" PRIu32,
             this,
             ProcessThreadService::getTid());
}

void
FactoryOps::dispatchTimerEvent(ClusterlibTimerEvent *tep)
{
    TRACE(CL_LOG, "dispatchTimerEvent");

    if (tep == NULL) {
        m_timerEventQueue.put(NULL);
    } 
    else {
        TimerEventPayload *tp = (TimerEventPayload *) tep->getUserData();
        m_timerEventQueue.put(tp);
    }
}

void
FactoryOps::dispatchZKEvent(zk::ZKWatcherEvent *zp)
{
    TRACE(CL_LOG, "dispatchZKEvent");
    
    if (!zp) {
	throw InvalidArgumentsException("Unexpected NULL ZKWatcherEvent");
    }

    CachedObjectEventHandler *fehp =
        (CachedObjectEventHandler *) zp->getContext();
    UserEventPayload *uep, *uepp;
    ClientImplList::iterator clIt;
    char buf[1024];

    /*
     * Protect against NULL context.
     */
    if (fehp == NULL) {
        snprintf(buf,
                 1024,
                 "type: %d, state: %d, key: %s",
                 zp->getType(),
                 zp->getState(),
                 zp->getPath().c_str());
        throw InconsistentInternalStateException(
            string("Unexpected NULL event context: ") + buf);
    }

    LOG_DEBUG(CL_LOG, 
              "Dispatching ZK event for key (%s)!", 
              zp->getPath().c_str());

    /*
     * Update the cache representation of the clusterlib
     * repository object and get back a prototypical
     * user event payload to send to clients.
     *
     * If the event is ZOO_DELETED_EVENT, then the Notifyable * should
     * be disregarded.
     *
     * If NULL is returned, the event is not propagated
     * to clients.
     */
    uep = updateCachedObject(fehp, zp);
    LOG_DEBUG(CL_LOG, 
              "dispatchZKEvent: If NULL cluster event payload on key %s, "
              "will not propogate to clients, payload is %p",
              zp->getPath().c_str(),
              uep);
    if (uep == NULL) {
        return;
    }

    /*
     * Now dispatch the event to all registered clients in
     * case they have a registered handler for the event on
     * the affected clusterlib repository object.
     */
    {
        Locker l(getClientsLock());

        for (clIt = m_clients.begin();
             clIt != m_clients.end(); 
             clIt++) {
            uepp = new UserEventPayload(*uep);
            LOG_DEBUG(CL_LOG, 
                      "dispatchZKEvent: Sending payload %p to client %p "
                      "on key %s",
                      uep,
                      *clIt,
                      zp->getPath().c_str());
            (*clIt)->sendEvent(uepp);
        }
    }
    delete uep;
}

void
FactoryOps::dispatchSessionEvent(zk::ZKWatcherEvent *zep)
{
    TRACE(CL_LOG, "dispatchSessionEvent");

    LOG_INFO(CL_LOG,
             "dispatchSessionEvent: (type: %d, state: %d (%s), key: %s)",
             zep->getType(), 
             zep->getState(),
             zk::ZooKeeperAdapter::getStateString(zep->getState()).c_str(),
             zep->getPath().c_str());

    if ((zep->getState() == ZOO_ASSOCIATING_STATE) ||
        (zep->getState() == ZOO_CONNECTING_STATE)) {
        /*
         * Not really clear what to do here.
         * For now do nothing.
         */
#ifdef	VERY_VERY_VERBOSE
        LOG_TRACE(CL_LOG, "Do nothing.");
#endif
    } 
    else if (zep->getState() == ZOO_CONNECTED_STATE) {
        /*
         * Mark as connected.
         */
#ifdef	VERY_VERY_VERBOSE
        LOG_TRACE(CL_LOG, "Marked connected.");
#endif
        m_connected = true;

        /*
         * Notify the FactoryOps constructor that the connection to
         * Zookeeper has been made.
         */
        m_firstConnect.predSignal();
    }
    else if (zep->getState() == ZOO_EXPIRED_SESSION_STATE) {
        /*
         * We give up on SESSION_EXPIRED.
         */
#ifdef	VERY_VERY_VERBOSE
        LOG_WARN(CL_LOG, "Giving up.");
#endif
        m_shutdown = true;
        m_connected = false;
    }
    else {
        LOG_ERROR(CL_LOG,
                 "Session event with unknown state "
                 "(type: %d, state: %d)",
                 zep->getType(), 
                 zep->getState());
        throw InconsistentInternalStateException(
            "dispatchSessionEvent: Unknown state");
    }
}

bool
FactoryOps::dispatchEndEvent()
{
    TRACE(CL_LOG, "dispatchEndEvent");

    ClientImplList::iterator clIt;

    /*
     * If the end event was already dispatched, don't do
     * it again.
     */
    {
        Locker l(getEndEventLock());

        if (m_endEventDispatched) {
            LOG_WARN(CL_LOG,
                     "Attempt to dispatch END event more than once!");
            return false;
        }
        m_endEventDispatched = true;
    }

    /*
     * Send a terminate signal to the timer
     * event handler thread.
     */
    dispatchTimerEvent(NULL);

    /*
     * Send a terminate signal to all registered
     * client-specific user event handler threads.
     */
    UserEventPayload *uepp = NULL;
    UserEventPayload uep(NotifyableKeyManipulator::createRootKey(), 
                         EN_ENDEVENT);

    Locker l(getClientsLock());
    for (clIt = m_clients.begin();
         clIt != m_clients.end();
         clIt++) {
        uepp = new UserEventPayload(uep);
        (*clIt)->sendEvent(uepp);
    }

    return true;
}

/*
 * Consume timer events. Run the timer event handlers.
 * This runs in a special thread owned by the factory.
 */
void
FactoryOps::consumeTimerEvents(void *param)
{
    TRACE(CL_LOG, "consumeTimerEvents");

    TimerEventPayload *tepp;

    LOG_INFO(CL_LOG,
             "Starting thread with FactoryOps::consumeTimerEvents(), "
             "this: %p, thread: %" PRIu32,
             this,
             ProcessThreadService::getTid());

    try {
        for (;;) {
            m_timerEventQueue.take(tepp);

            /*
             * If we received the terminate signal,
             * then exit from the loop.
             */
            if (tepp == NULL) {
                LOG_INFO(CL_LOG,
                         "Received terminate signal, finishing loop");
                break;
            }

            /*
             * Dispatch the event to its handler, if the
             * event hadn't been cancelled.
             */
            if (!tepp->cancelled()) {
                tepp->getHandler()->handleTimerEvent(tepp->getId(),
                                                     tepp->getData());
            }

            LOG_INFO(CL_LOG,
                     "Serviced timer %d, handler %p, client data %p",
                     tepp->getId(), 
                     tepp->getHandler(),
                     tepp->getData());

            /*
             * Deallocate the payload object.
             */
            {
                Locker l(getTimersLock());

                m_timerRegistry.erase(tepp->getId());
                delete tepp;
            }
        }

    } catch (zk::ZooKeeperException &zke) {
        LOG_ERROR(CL_LOG, "ZooKeeperException: %s", zke.what());
        throw RepositoryInternalsFailureException(zke.what());
    } catch (Exception &e) {
        throw Exception(e.what());
    } catch (std::exception &stde) {
        LOG_ERROR(CL_LOG, "Unknown exception: %s", stde.what());
        throw Exception(stde.what());
    }        

    LOG_INFO(CL_LOG,
             "Ending thread with FactoryOps::consumeTimerEvents(): "
             "this: %p, thread: %" PRIu32,
             this,
             ProcessThreadService::getTid());
}

NotifyableImpl *
FactoryOps::getNotifyable(NotifyableImpl *parent,
                          const string &registeredNotifyableName, 
                          const string &name,
                          AccessType accessType)
{
    TRACE(CL_LOG, "getNotifyable");

    LOG_DEBUG(CL_LOG, 
              "getNotifyable: parent '%p', registeredNotifyableName '%s', "
              "name '%s' accessType '%s'",
              parent,
              registeredNotifyableName.c_str(),
              name.c_str(),
              getAccessTypeString(accessType).c_str());

    map<string, RegisteredNotifyable *>::const_iterator registeredNotifyableIt;
    {
        ReadLocker l(&m_registeredNotifyableMapRdWrLock);
        registeredNotifyableIt = 
            m_registeredNotifyableMap.find(registeredNotifyableName);
    }

    /* Get the registered notifyable for the appropriate member functions */
    if (registeredNotifyableIt == m_registeredNotifyableMap.end()) {
        ostringstream oss;
        oss << "getNotifyable: Tried to get a notifyable of type (" 
            << registeredNotifyableName << ") that does not exist.";
        throw InconsistentInternalStateException(oss.str());
    }
    RegisteredNotifyable *registeredNtp = registeredNotifyableIt->second;
    if (!registeredNtp->isValidName(name)) {
        ostringstream oss;
        oss << "getNotifyable: Name (" << name << ") is invalid for type "
            << registeredNotifyableName;
        throw InvalidArgumentsException(oss.str());
    }
    
    string notifyableKey = registeredNtp->generateKey(
        ((parent == NULL) ? "" : parent->getKey()),
        name);
    SafeNotifyableMap *safeNotifyableMap = 
        registeredNtp->getSafeNotifyableMap();

    /* Try to find the notifyable in its proper map cache. */
    NotifyableImpl *ntp = NULL;
    {
        Locker l(&safeNotifyableMap->getLock());
        ntp = safeNotifyableMap->getNotifyable(notifyableKey);
        if (ntp != NULL) {
            ntp->incrRefCount();
        }
    }

    if ((ntp != NULL) || (accessType == CACHED_ONLY)) {
        return ntp;
    }

    if (parent != NULL) {
        getOps()->getDistributedLocks()->acquire(
            parent, 
            ClusterlibStrings::NOTIFYABLELOCK);
    }
    
    ntp = registeredNtp->loadNotifyableFromRepository(
        name, notifyableKey, parent);
    if ((ntp == NULL) && (accessType == CREATE_IF_NOT_FOUND)) {
        registeredNtp->createRepositoryObjects(name, notifyableKey);
        ntp = registeredNtp->loadNotifyableFromRepository(
            name, notifyableKey, parent);
        if (ntp == NULL) {
            ostringstream oss;
            oss << "getNotifyable: Couldn't load notifyable with name="
                << name << ",key=" << notifyableKey;
            throw InconsistentInternalStateException(oss.str());
        }
    }

    if (ntp != NULL) {
        ntp->setSafeNotifyableMap(*safeNotifyableMap);

        Locker l(&safeNotifyableMap->getLock());

        safeNotifyableMap->uniqueInsert(*ntp);
        ntp->initialize();
    }
    
    if (parent != NULL) {
        getOps()->getDistributedLocks()->release(
            parent,
            ClusterlibStrings::NOTIFYABLELOCK);
    }

    return ntp;
}

bool
FactoryOps::isValidKey(const vector<string> &registeredNameVec,
                       const string &key)
{
    TRACE(CL_LOG, "isValidKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isValidKey(registeredNameVec, components, -1);
}

bool
FactoryOps::isValidKey(const vector<string> &registeredNameVec,
                       const vector<string> &components, 
                       int32_t elements)
{
    TRACE(CL_LOG, "isValidKey");

    ReadLocker l(&m_registeredNotifyableMapRdWrLock);

    if (registeredNameVec.empty()) {
        map<string, RegisteredNotifyable *>::const_iterator 
            registeredNotifyableIt;
        for (registeredNotifyableIt = m_registeredNotifyableMap.begin();
             registeredNotifyableIt != m_registeredNotifyableMap.end();
             ++registeredNotifyableIt) {
            if (registeredNotifyableIt->second->isValidKey(
                    components, elements)) {
                return true;
            }
        }

        return false;
    }

    RegisteredNotifyable *regNtp = NULL;
    vector<string>::const_iterator registeredNameVecIt;
    for (registeredNameVecIt = registeredNameVec.begin();
         registeredNameVecIt != registeredNameVec.end();
         ++registeredNameVecIt) {
        regNtp = getOps()->getRegisteredNotifyable(*registeredNameVecIt);
        if (regNtp != NULL) {
            if (regNtp->isValidKey(components, elements)) {
                return true;
            }
        }
    }

    return false;
}

string
FactoryOps::getNotifyableKeyFromKey(const string &key)
{
    TRACE(CL_LOG, "getNotifyableKeyFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));

    if (static_cast<int32_t>(components.size()) < 
        ClusterlibInts::ROOT_COMPONENTS_COUNT) {
        return string();
    }

    /* 
     * Check if this key already matches a possible Zookeeper node
     * that represents a Notifyable.  Also, strip off one path section
     * and check again if that fails.  
     *
     * The current layout of Clusterlib objects in Zookeeper limits
     * any object to having only one layer beneath them as part of
     * that object.  If that policy changes, this function will have
     * to change.
     */
    if (isValidKey(vector<string>(), components)) {
        LOG_DEBUG(CL_LOG,
                  "getNotifyableKeyFromKey: From key (%s), returned same key)",
                  key.c_str());
        return key;
    }
    if (isValidKey(vector<string>(), components, components.size() -1)) {
        string res = key;
        size_t keySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR);
        if (keySeparator == string::npos) {
            LOG_ERROR(CL_LOG,
                      "getNotifyableKeyFromKey: Couldn't find key "
                      "separator in key (%s)", key.c_str());
            throw InconsistentInternalStateException(
                "getNotifyableKeyFromKey: Couldn't find key separator");
        }
        res.erase(keySeparator);
        LOG_DEBUG(CL_LOG,
                  "getNotifyableKeyFromKey: From key (%s), "
                  "returned stripped key (%s))",
                  key.c_str(),
                  res.c_str());

        return res;
    }

    return string();
}

NotifyableImpl *
FactoryOps::getNotifyableFromKey(
    const vector<string> &registeredNameVec,
    const string &key, 
    AccessType accessType)
{
    TRACE(CL_LOG, "getNotifyableFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    LOG_DEBUG(CL_LOG, "getNotifyableFromKey: key %s", key.c_str());
    return getNotifyableFromComponents(
        registeredNameVec, components, -1, accessType);
}

NotifyableImpl *
FactoryOps::getNotifyableFromComponents(
    const vector<string> &registeredNameVec,
    const vector<string> &components,
    int32_t elements,
    AccessType accessType)
{
    TRACE(CL_LOG, "getNotifyableFromComponents");

    NotifyableImpl *ntp = NULL;
    RegisteredNotifyable *regNtp = NULL;

    if (elements == -1) {
        elements = components.size();
    }

    ReadLocker l(&m_registeredNotifyableMapRdWrLock);
    
    if (registeredNameVec.empty()) {
        map<string, RegisteredNotifyable *>::const_iterator 
            registeredNotifyableIt;
        for (registeredNotifyableIt = m_registeredNotifyableMap.begin();
             registeredNotifyableIt != m_registeredNotifyableMap.end();
             ++registeredNotifyableIt) {
            ntp = registeredNotifyableIt->second->getObjectFromComponents(
                components, elements, accessType);
            if (ntp != NULL) {
                LOG_DEBUG(
                    CL_LOG,
                    "getObjectFromComponents: Found notifyable in "
                    "RegisteredNotifyable name = '%s' ",
                    registeredNotifyableIt->second->registeredName().c_str());
                return ntp;
            }
        }
        
        return NULL;
    }
    else {
        vector<string>::const_iterator registeredNameVecIt;
        for (registeredNameVecIt = registeredNameVec.begin();
             registeredNameVecIt != registeredNameVec.end();
             ++registeredNameVecIt) {
            regNtp = getOps()->getRegisteredNotifyable(*registeredNameVecIt);
            if (regNtp != NULL) {
                ntp = regNtp->getObjectFromComponents(
                    components, elements, accessType);
                if (ntp != NULL) {
                    LOG_DEBUG(CL_LOG,
                              "getObjectFromComponents: Found notifyable in "
                              "RegisteredNotifyable name = '%s' ",
                              regNtp->registeredName().c_str());
                    return ntp;
                }
            }
        }

        return NULL;
    }
}

void 
FactoryOps::removeCachedNotifyable(NotifyableImpl *ntp)
{
    TRACE(CL_LOG, "removeCachedNotifyable");
    
    Locker l(&ntp->getSafeNotifyableMap()->getLock());
    {

        LOG_DEBUG(CL_LOG, 
                  "removeCachedNotifyable: state changing to REMOVED "
                  "for Notifyable %s",
                  ntp->getKey().c_str());

        /* One way transition to removed, this is fine. */
        if (ntp->getState() == Notifyable::REMOVED) {
            throw InvalidMethodException(
                string("removeCachedNotifyable: Tried to remove 2x ") +
                ntp->getKey().c_str());
        }
        
        Locker l3(getRemovedNotifyablesLock());
        set<Notifyable *>::const_iterator it = 
            getRemovedNotifyables()->find(ntp);
        if (it != getRemovedNotifyables()->end()) {
            throw InconsistentInternalStateException(
                string("RemoveNotifyableFromCacheByKey: Notifyable for key ") +
                ntp->getKey() + 
                " is already in removed notifyables" +
                " set!");
        }
        getRemovedNotifyables()->insert(ntp);

        ntp->setState(Notifyable::REMOVED);
    }

    ntp->getSafeNotifyableMap()->erase(*ntp);
}

Mutex *
FactoryOps::getClientsLock() 
{
    TRACE(CL_LOG, "getClientsLock");
    return &m_clLock; 
}

Mutex *
FactoryOps::getRemovedNotifyablesLock() 
{
    TRACE(CL_LOG, "getRemovedNotifyablesLock");
    return &m_removedNotifyablesLock; 
}

Mutex *
FactoryOps::getTimersLock() 
{ 
    TRACE(CL_LOG, "getTimersLock");
    return &m_timerRegistryLock; 
}

Mutex *
FactoryOps::getSyncEventLock() 
{ 
    TRACE(CL_LOG, "getSyncEventLock");
    return &m_syncEventLock; 
}

Cond *
FactoryOps::getSyncEventCond() 
{ 
    TRACE(CL_LOG, "getSyncEventCond");
    return &m_syncEventCond; 
}

Mutex *
FactoryOps::getEndEventLock() 
{
    TRACE(CL_LOG, "getEndEventLock");
    return &m_endEventLock; 
}

NameList
FactoryOps::getChildrenNames(
    const string &notifyableKey,
    CachedObjectChangeHandlers::CachedObjectChange change)
{
    TRACE(CL_LOG, "getChildrenNames");

    NameList list;
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            notifyableKey,
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(change)),
        m_zk.getNodeChildren(notifyableKey, list),
        change,
        notifyableKey,
        "Reading the value of %s failed: %s",
        notifyableKey.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin();
         nlIt != list.end();
         ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(notifyableKey.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}

NotifyableList 
FactoryOps::getNotifyableList(NotifyableImpl *parent,
                              const std::string &registeredNotifyableName,
                              const NameList &nameList,
                              AccessType accessType)
{
    TRACE(CL_LOG, "getNotifyableList");
    
    NotifyableList list;
    NotifyableImpl *ntp = NULL;
    NameList::const_iterator nameListIt;
    for (nameListIt = nameList.begin(); 
         nameListIt != nameList.end(); 
         ++nameListIt) {
        ntp = getNotifyable(
            parent, registeredNotifyableName, *nameListIt, accessType);
        if (ntp != NULL) {
            list.push_back(ntp);
        }
    }

    return list;
}

/*
 * Register a timer handler.
 */
TimerId
FactoryOps::registerTimer(TimerEventHandler *handler,
                          uint64_t afterMsecs,
                          ClientData data)
{
    Locker l(getTimersLock());
    TimerEventPayload *tepp =
        new TimerEventPayload(afterMsecs, handler, data);
    TimerId id = m_timerEventSrc.scheduleAfter(afterMsecs, tepp);

    tepp->updateTimerId(id);
    m_timerRegistry[id] = tepp;

    return id;
}

/*
 * Cancel a timer.
 */
bool
FactoryOps::cancelTimer(TimerId id)
{
    TRACE(CL_LOG, "cancelTimer");

    Locker l(getTimersLock());
    TimerRegistry::const_iterator timerIt = m_timerRegistry.find(id);

    if (timerIt == m_timerRegistry.end()) {
        return false;
    }

    timerIt->second->cancel();
    if (m_timerEventSrc.cancelAlarm(id)) {
        return true;
    }
    return false;
}

UserEventPayload *
FactoryOps::updateCachedObject(CachedObjectEventHandler *fehp,
                               zk::ZKWatcherEvent *ep)
{
    TRACE(CL_LOG, "updateCachedObject");

    if (fehp == NULL) {
        throw InvalidArgumentsException("NULL CachedObjectEventHandler!");
    }
    if (ep == NULL) {
        throw InvalidArgumentsException("NULL watcher event!");
    }

    int32_t etype = ep->getType();

    LOG_INFO(CL_LOG,
             "updateCachedObject: (%p, %p, %s, %s)",
             fehp,
             ep,
             zk::ZooKeeperAdapter::getEventString(etype).c_str(),
             ep->getPath().c_str());

    /*
     * Based on the path and etype, several things need to happen.  At
     * no point in the execution loop can this function hold a
     * distributed lock.  It may hold local locks to coordinate with
     * user threads.
     *
     * 1. Get the derived path of the Notifyable that the event path refers to.
     * 2. Get the Notifyable from the derived path in the cache if it exists 
     *    and update it with the appropriate CachedObjectEventHandler.
     * 3. Pass the derived path and event as a payload in the return.
     *
     * There are exceptions where getting the notifyable is not
     * required.  They are described in the comment below.
     *
     * Note: Getting the NotifyableImpl * for the derived path is best
     * effort.  The NotifyableImpl may have already been removed and
     * in that case, should return NULL.  If the event is
     * ZOO_DELETED_EVENT , there is no guarantee to return the correct
     * Notifyable since it should have been removed.  Still need to
     * try to retrieve it in the event of ZOO_DELETED_EVENT, since if
     * it exists, it needs to be removed.
     */
    string notifyablePath;
    NotifyableImpl *ntp = NULL;
    string cachedObjectPath = ep->getPath();

    /* There are two cases where the getting the Notifyable is not
     * necessary.
     *
     * 1.  A sync event.
     * 2.  A lock node deleted event.
     */
    if (ep->getPath().compare(ClusterlibStrings::SYNC) == 0) {
        CallbackAndContext *callbackAndContext = 
            reinterpret_cast<CallbackAndContext *>(fehp);
        int64_t syncEventId = 
            *(reinterpret_cast<int64_t *>(callbackAndContext->context));
        cachedObjectPath = 
            NotifyableKeyManipulator::createSyncEventKey(syncEventId);
        fehp = reinterpret_cast<CachedObjectEventHandler *>(
            callbackAndContext->callback);
        getHandlerAndContextManager()->deleteCallbackAndContext(
            callbackAndContext);
        ntp = NULL;
    }
    else if ((ep->getPath().find(ClusterlibStrings::PARTIALLOCKNODE) != 
          string::npos) && (etype == ZOO_DELETED_EVENT)) {
        notifyablePath = ep->getPath();
        ntp = NULL;
    }
    else {
        try {
            notifyablePath = getNotifyableKeyFromKey(ep->getPath());
            /*
             * Only try and affect notifyables in our cache, since
             * otherwise distributed locks must be held.  This will
             * hold the sync lock for notifyables for a short time.
             */
            ntp = getNotifyableFromKey(
                vector<string>(), notifyablePath, CACHED_ONLY); 
            
            LOG_DEBUG(CL_LOG, 
                      "updateCachedObject: Got Notifyable key (%s) from "
                      "path (%s)",
                      notifyablePath.c_str(),
                      ep->getPath().c_str());
            
            if (notifyablePath.empty()) {
                throw InconsistentInternalStateException(
                    "updateCachedObject: Returned an empty path from the "
                    "original path (" + ep->getPath() + 
                    ") and is unrelated to any Notifyable!");
            }
            if (ntp == NULL) {
                LOG_DEBUG(CL_LOG, 
                          "updateCachedObject: Returned no notifyable from "
                          "the original path (%s) with notifyablePath (%s)" ,
                          ep->getPath().c_str(), 
                          notifyablePath.c_str());
            }
        }
        catch (RepositoryInternalsFailureException &reife) {
            LOG_WARN(CL_LOG, 
                     "updateCachedObject: NotifyableImpl with key %s "
                     "no longer exists",
                     notifyablePath.c_str());
        }
        catch (ObjectRemovedException &ore) {
            LOG_WARN(CL_LOG,
                     "updateCachedObject: NotifyableImpl with key %s "
                     "was removed",
                     notifyablePath.c_str());
        }
    }

    /*
     * Invoke the object handler. It will update the cache. It
     * will also return the kind of user-level event that this
     * repository event represents.
     */
    Event e = fehp->deliver(ntp, etype, cachedObjectPath);
    if (ntp != NULL) {
        if (dynamic_cast<Root *>(ntp) == NULL) {
            ntp->releaseRef();
        }
    }

    LOG_DEBUG(CL_LOG, 
              "updateCachedObject: Returning payload for event %s (%d) for "
              "key '%s'",
              UserEventHandler::getEventsString(e).c_str(),
              e,
              notifyablePath.c_str());
              
    if (e == EN_NOEVENT) {
        return NULL;
    }

    return new UserEventPayload(notifyablePath, e);
}

void
FactoryOps::establishNotifyableStateChange(NotifyableImpl *ntp)
{
    string ready;
       
    if (ntp == NULL) {
        throw InvalidArgumentsException(
            "establishNotifyableStateChange: with NULL NotifyableImpl *");
    }

    SAFE_CALLBACK_ZK(
        m_zk.getNodeData(
            ntp->getKey(),
            ready,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::CURRENT_STATE_CHANGE)),
        ,
        CachedObjectChangeHandlers::CURRENT_STATE_CHANGE,
        ntp->getKey(),
        "Reading the value of %s failed: %s",
        ntp->getKey().c_str(),
        true,
        true);
}

void
FactoryOps::setIdResponse(const string &id, JSONValue::JSONObject response)
{
    TRACE(CL_LOG, "setIdResponse");

    Locker l(&m_idResponseMapMutex);
    map<string, JSONValue::JSONObject>::const_iterator idResponseMapIt =
        m_idResponseMap.find(id);
    if (idResponseMapIt != m_idResponseMap.end()) {
        throw InconsistentInternalStateException(
            string("setIdResponse: Response ") + 
            JSONCodec::encode(m_idResponseMap[id]) + 
            string(" already exists for id ") + id);
    }
    m_idResponseMap[id] = response;
}

JSONValue::JSONObject
FactoryOps::getIdResponse(const string &id)
{
    TRACE(CL_LOG, "getIdResponse");

    Locker l(&m_idResponseMapMutex);
    map<string, JSONValue::JSONObject>::iterator idResponseMapIt =
        m_idResponseMap.find(id);
    if (idResponseMapIt == m_idResponseMap.end()) {
        throw InconsistentInternalStateException(
            string("getIdResponse: Response for id ") + id + 
            string(" cannot be found"));
    }
    JSONValue::JSONObject retObj = idResponseMapIt->second;
    m_idResponseMap.erase(idResponseMapIt);
    return retObj;
}

};	/* End of 'namespace clusterlib' */
