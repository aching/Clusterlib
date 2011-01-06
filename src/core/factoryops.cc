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

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib {

FactoryOps::FactoryOps(const string &registry, int64_t msecConnectTimeout)
    : m_syncEventId(0),
      m_syncEventIdCompleted(0),
      m_endEventDispatched(false),
      m_config(registry, msecConnectTimeout, true), 
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
     * Connect to ZK using the user-defined timeout.  Add one more
     * second to allow zookeeper_init() to complete and return.
     */
    try {
        m_zk.reconnect();
        LOG_INFO(CL_LOG, 
                 "Waiting for connect event from ZooKeeper up to %" PRId64 
                 " msecs from %s, thread: %" PRId32,
                 msecConnectTimeout,
                 m_config.getHosts().c_str(),
                 ProcessThreadService::getTid());
        if (m_firstConnect.predWaitUsecs((msecConnectTimeout + 1) * 1000) 
            == false) {
            LOG_ERROR(CL_LOG,
                      "FactoryOps: Did not receive connect event from %s in "
                      "time (%" PRId64 " msecs), aborting",
                      m_config.getHosts().c_str(),
                      msecConnectTimeout);
	    throw RepositoryConnectionFailureException(
		"Did not receive connect event in time, aborting");
        }
        LOG_INFO(CL_LOG, 
                 "After wait, m_connected == %d",
                 static_cast<int>(m_connected));
    } 
    catch (zk::ZooKeeperException &e) {
        m_zk.disconnect();
        LOG_FATAL(CL_LOG, 
                  "Failed to connect to Zookeeper (%s)",
                  m_config.getHosts().c_str());
        throw RepositoryInternalsFailureException(e.what());
    }

    /*
     * Register the HashRange objects.
     */
    registerHashRange(UnknownHashRange());
    registerHashRange(Uint64HashRange());

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
    getNotifyable(shared_ptr<NotifyableImpl>(),
                  CLString::REGISTERED_ROOT_NAME, 
                  CLStringInternal::ROOT_NAME,
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
    
    map<string, RegisteredNotifyable *>::iterator registeredNotifyableMapIt;
    while (!m_registeredNotifyableMap.empty()) {
        registeredNotifyableMapIt = m_registeredNotifyableMap.begin();
        delete registeredNotifyableMapIt->second;
        m_registeredNotifyableMap.erase(registeredNotifyableMapIt);
    }
}

void
FactoryOps::registerHashRange(const HashRange &hashRange)
{
    TRACE(CL_LOG, "registerHashRange");
    
    WriteLocker l(&m_registeredHashRangeMapRdWrLock);
    
    map<string, HashRange *>::const_iterator registeredHashRangeMapIt = 
        m_registeredHashRangeMap.find(hashRange.getName());
    if (registeredHashRangeMapIt != m_registeredHashRangeMap.end()) {
        ostringstream oss;
        oss << "registerHashRange: Already has HashRange with name="
            << hashRange.getName() << " in m_registeredHashRangeMap";
        throw InvalidArgumentsException(oss.str());
    }
    m_registeredHashRangeMap[hashRange.getName()] = &(hashRange.create());
}

void
FactoryOps::unregisterAllHashRanges()
{
    WriteLocker l(&m_registeredHashRangeMapRdWrLock);

    map<string, HashRange *>::iterator registeredHashRangeMapIt;
    while (!m_registeredHashRangeMap.empty()) {
        registeredHashRangeMapIt = m_registeredHashRangeMap.begin();
        delete registeredHashRangeMapIt->second;
        m_registeredHashRangeMap.erase(registeredHashRangeMapIt);
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

    unregisterAllNotifyables();

    unregisterAllHashRanges();

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
FactoryOps::createJSONRPCResponseClient(
    const shared_ptr<Queue> &responseQueueSP,
    const shared_ptr<Queue> &completedQueueSP)
{
    TRACE(CL_LOG, "createJSONRPCResponseClient");

    ClientImpl *client = dynamic_cast<ClientImpl *>(createClient());
    if (client == NULL) {
        throw InconsistentInternalStateException(
            "createJSONRPCResponseClient: Failed to create client");
    }
    client->registerJSONRPCResponseHandler(responseQueueSP,
                                           completedQueueSP);
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

    string key(CLStringInternal::ROOT_ZNODE);
    key.append(CLStringInternal::CLUSTERLIB);

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

zk::ZooKeeperAdapter *
FactoryOps::getRepository()
{
    return &m_zk;
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

HashRange &
FactoryOps::getHashRange(const string &name)
{
    TRACE(CL_LOG, "getHashRange");

    map<string, HashRange *>::const_iterator registeredHashRangeMapIt;

    ReadLocker l(&m_registeredHashRangeMapRdWrLock);

    registeredHashRangeMapIt = m_registeredHashRangeMap.find(name);
    if (registeredHashRangeMapIt == m_registeredHashRangeMap.end()) {
        registeredHashRangeMapIt = 
            m_registeredHashRangeMap.find(UnknownHashRange::name());
        if (registeredHashRangeMapIt == m_registeredHashRangeMap.end()) {
            throw InconsistentInternalStateException(
                "getHashRange: No such HashRange with name = unknown exists!");
        }
    }

    return registeredHashRangeMapIt->second->create();
}

void
FactoryOps::runPeriodic(void *param)
{
    TRACE(CL_LOG, "runPeriodic");

    LOG_INFO(CL_LOG,
             "Starting thread with FactoryOps::runPeriodic(), "
             "this: %p, thread: %" PRId32,
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
                      "runPeriodic: thread: %" PRId32 " doing run() "
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
             "this: %p, thread: %" PRId32,
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
    
    map<Periodic *, CXXThread<FactoryOps> *>::iterator periodicMapIt;

    {
        Locker l(&m_periodicMapLock);

        periodicMapIt = m_periodicMap.find(&periodic);
    }

    if (periodicMapIt == m_periodicMap.end()) {
        return false;
    }

    periodicMapIt->second->getPredMutexCond().predSignal();
    periodicMapIt->second->Join();

    {
        Locker l(&m_periodicMapLock);

        delete periodicMapIt->second;
        m_periodicMap.erase(periodicMapIt);
    }

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

    map<Periodic *, CXXThread<FactoryOps> *>::iterator periodicMapIt;
    m_periodicMapLock.acquire();
    while (!m_periodicMap.empty()) {
        periodicMapIt = m_periodicMap.begin();
        m_periodicMapLock.release();
        
        periodicMapIt->second->getPredMutexCond().predSignal();
        periodicMapIt->second->Join();

        m_periodicMapLock.acquire();
        delete periodicMapIt->second;
        m_periodicMap.erase(periodicMapIt);
    }
    m_periodicMapLock.release();
}

FactoryOps *
FactoryOps::getOps() 
{
    return this;
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
             "this: %p, thread: %" PRId32,
             this,
             ProcessThreadService::getTid());

    try {
        while (m_shutdown == false) { 
            LOG_DEBUG(CL_LOG,
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
                            (zp->getPath().compare(CLStringInternal::SYNC) 
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
             "this: %p, thread: %" PRId32,
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

    LOG_DEBUG(CL_LOG,
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
             "this: %p, thread: %" PRId32,
             this,
             ProcessThreadService::getTid());

    try {
        for (;;) {
            tepp = m_timerEventQueue.take();

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

            LOG_DEBUG(CL_LOG,
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
             "this: %p, thread: %" PRId32,
             this,
             ProcessThreadService::getTid());
}

bool
FactoryOps::getNotifyableWaitMsecs(
    const shared_ptr<NotifyableImpl> parentSP,
    const string &registeredNotifyableName, 
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<NotifyableImpl> *pNotifyableSP)
{
    TRACE(CL_LOG, "getNotifyableWaitMsecs");

    LOG_DEBUG(CL_LOG, 
              "getNotifyableWaitMsecs: parent '%p', registeredNotifyableName "
              "'%s', name '%s' accessType '%s' msecTimeout '%" PRId64 
              "', pNotifyableSP '%p'",
              parentSP.get(),
              registeredNotifyableName.c_str(),
              name.c_str(),
              accessTypeToString(accessType).c_str(),
              msecTimeout,
              pNotifyableSP);

    if (pNotifyableSP == NULL) {
        throw InvalidArgumentsException(
            "getNotifyableWaitMsecs: NULL pNotifyableSP");
    }

    pNotifyableSP->reset();
    map<string, RegisteredNotifyable *>::const_iterator registeredNotifyableIt;
    {
        ReadLocker l(&m_registeredNotifyableMapRdWrLock);

        registeredNotifyableIt = 
            m_registeredNotifyableMap.find(registeredNotifyableName);
    }

    /* Get the registered notifyable for the appropriate member functions */
    if (registeredNotifyableIt == m_registeredNotifyableMap.end()) {
        ostringstream oss;
        oss << "getNotifyableWaitMsecs: Tried to get a notifyable of type (" 
            << registeredNotifyableName << ") that does not exist.";
        throw InconsistentInternalStateException(oss.str());
    }
    RegisteredNotifyable *registeredNotifyable = 
        registeredNotifyableIt->second;
    if (!registeredNotifyable->isValidName(name)) {
        ostringstream oss;
        oss << "getNotifyableWaitMsecs: Name (" << name 
            << ") is invalid for type " << registeredNotifyableName;
        throw InvalidArgumentsException(oss.str());
    }
    
    LOG_DEBUG(CL_LOG, 
              "getNotifyableWaitMsecs: Using registeredNotifyable=%s",
              registeredNotifyable->registeredName().c_str());

    string notifyableKey = registeredNotifyable->generateKey(
        ((parentSP == NULL) ? "" : parentSP->getKey()),
        name);
    SafeNotifyableMap *safeNotifyableMap = 
        registeredNotifyable->getSafeNotifyableMap();

    /* Try to find the notifyable in its proper map cache. */
    {
        Locker l(&safeNotifyableMap->getLock());

        *pNotifyableSP = safeNotifyableMap->getNotifyable(notifyableKey);
    }

    if ((*pNotifyableSP != NULL) || (accessType == CACHED_ONLY)) {
        return true;
    }

    /*
     * Need to lock the parent if it is being LOAD_FROM_REPOSITORY or
     * CREATE_IF_NOT_FOUND.  However, if the minimum lock is already
     * held, there is no need to do this.
     */
    bool hasMinimumLock = false;
    if (parentSP != NULL) {
        DistributedLockType distributedLockType = DIST_LOCK_SHARED;
        if (accessType == CREATE_IF_NOT_FOUND) {
            distributedLockType = DIST_LOCK_EXCL;
        }
        DistributedLockType ownerDistributedLockType = DIST_LOCK_INIT;
        hasMinimumLock = parentSP->hasLock(CLString::CHILD_LOCK, 
                                           &ownerDistributedLockType);
        if (hasMinimumLock) {
            if ((distributedLockType == DIST_LOCK_EXCL) &&
                (ownerDistributedLockType != DIST_LOCK_EXCL)) {
                ostringstream oss;
                oss << "getNotifyableWaitMsecs: Notiyable to be retrieved "
                    << "with Notiyable key=" << notifyableKey << " has parent="
                    << parentSP->getKey() << " that is already locked with "
                    << distributedLockTypeToString(ownerDistributedLockType)
                    << " but needs "
                    << distributedLockTypeToString(distributedLockType);
                throw InvalidMethodException(oss.str());
            }
        }
        else {
            hasMinimumLock = getOps()->getDistributedLocks()->acquireWaitMsecs(
                msecTimeout,
                dynamic_pointer_cast<Notifyable>(parentSP),
                CLString::CHILD_LOCK,
                distributedLockType);
            if (false == hasMinimumLock) {
                return false;
            }
        }
    }
    
    *pNotifyableSP = registeredNotifyable->loadNotifyableFromRepository(
        name, notifyableKey, parentSP);
    if ((*pNotifyableSP == NULL) && (accessType == CREATE_IF_NOT_FOUND)) {
        registeredNotifyable->createRepositoryObjects(name, notifyableKey);
        *pNotifyableSP = registeredNotifyable->loadNotifyableFromRepository(
            name, notifyableKey, parentSP);
        if (*pNotifyableSP == NULL) {
            ostringstream oss;
            oss << "getNotifyableWaitMsecs: Couldn't load notifyable "
                << "with name=" << name << ", key=" << notifyableKey;
            throw InconsistentInternalStateException(oss.str());
        }
    }

    if (*pNotifyableSP != NULL) {
        (*pNotifyableSP)->setSafeNotifyableMap(*safeNotifyableMap);

        Locker l(&safeNotifyableMap->getLock());

        safeNotifyableMap->uniqueInsert(*pNotifyableSP);
        (*pNotifyableSP)->initialize();
    }
    
    if ((true == hasMinimumLock) && (NULL != parentSP)) {
        getOps()->getDistributedLocks()->release(
            parentSP,
            CLString::CHILD_LOCK);
    }

    return true;
}

shared_ptr<NotifyableImpl> 
FactoryOps::getNotifyable(
    const shared_ptr<NotifyableImpl> &parentSP,
    const string &registeredNotifyableName, 
    const string &name,
    AccessType accessType)
{
    shared_ptr<NotifyableImpl> notifyableSP;
    getNotifyableWaitMsecs(parentSP,
                           registeredNotifyableName,
                           name,
                           accessType,
                           -1,
                           &notifyableSP);
    return notifyableSP;
}

bool
FactoryOps::isValidKey(const vector<string> &registeredNameVec,
                       const string &key)
{
    TRACE(CL_LOG, "isValidKey");

    vector<string> components;
    split(components, key, is_any_of(CLString::KEY_SEPARATOR));
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
    split(components, key, is_any_of(CLString::KEY_SEPARATOR));

    if (static_cast<int32_t>(components.size()) < 
        CLNumericInternal::ROOT_COMPONENTS_COUNT) {
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
        size_t keySeparator = res.rfind(CLString::KEY_SEPARATOR);
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

bool
FactoryOps::getNotifyableFromKeyWaitMsecs(
    const vector<string> &registeredNameVec,
    const string &key, 
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<NotifyableImpl> *pNotifyableSP)
{
    TRACE(CL_LOG, "getNotifyableFromKeyWaitMsecs");

    vector<string> components;
    split(components, key, is_any_of(CLString::KEY_SEPARATOR));
    LOG_DEBUG(CL_LOG, "getNotifyableFromKey: key %s", key.c_str());
    return getNotifyableFromComponents(registeredNameVec, 
                                       components, 
                                       -1, 
                                       accessType, 
                                       msecTimeout, 
                                       pNotifyableSP);
}

shared_ptr<NotifyableImpl> 
FactoryOps::getNotifyableFromKey(
    const vector<string> &registeredNameVec,
    const string &key, 
    AccessType accessType)
{
    shared_ptr<NotifyableImpl> notifyableSP;
    getNotifyableFromKeyWaitMsecs(
        registeredNameVec, key, accessType, -1, &notifyableSP);
    return notifyableSP;
}

bool
FactoryOps::getNotifyableFromComponents(
    const vector<string> &registeredNameVec,
    const vector<string> &components,
    int32_t elements,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<NotifyableImpl> *pNotifyableSP)
{
    TRACE(CL_LOG, "getNotifyableFromComponents");

    if (pNotifyableSP == NULL) {
        throw InvalidArgumentsException(
            "getObjectFromComponents: NULL pNotifyableSP");
    }
    pNotifyableSP->reset();

    if (elements == -1) {
        elements = components.size();
    }

    ReadLocker l(&m_registeredNotifyableMapRdWrLock);
    
    bool success = false;
    if (registeredNameVec.empty()) {
        map<string, RegisteredNotifyable *>::const_iterator 
            registeredNotifyableIt;
        for (registeredNotifyableIt = m_registeredNotifyableMap.begin();
             registeredNotifyableIt != m_registeredNotifyableMap.end();
             ++registeredNotifyableIt) {
            success = registeredNotifyableIt->second->getObjectFromComponents(
                components, 
                elements, 
                accessType, 
                msecTimeout, 
                pNotifyableSP);
            if ((success == true) && (*pNotifyableSP != NULL)) {
                LOG_DEBUG(
                    CL_LOG,
                    "getObjectFromComponents: Found notifyable in "
                    "RegisteredNotifyable name = '%s' ",
                    registeredNotifyableIt->second->registeredName().c_str());
                return true;
            }
        }
    }
    else {
        RegisteredNotifyable *pRegNotifyable = NULL;
        vector<string>::const_iterator registeredNameVecIt;
        for (registeredNameVecIt = registeredNameVec.begin();
             registeredNameVecIt != registeredNameVec.end();
             ++registeredNameVecIt) {
            pRegNotifyable = 
                getOps()->getRegisteredNotifyable(*registeredNameVecIt);
            if (pRegNotifyable != NULL) {
                success = pRegNotifyable->getObjectFromComponents(
                    components, 
                    elements, 
                    accessType, 
                    msecTimeout, 
                    pNotifyableSP);
                if ((success == true) && (*pNotifyableSP != NULL)) {
                    LOG_DEBUG(
                        CL_LOG,
                        "getObjectFromComponents: Found notifyable in "
                        "RegisteredNotifyable name = '%s' with name = '%s'",
                        pRegNotifyable->registeredName().c_str(),
                        components.back().c_str());
                    return true;
                }
            }
        }
    }

    pNotifyableSP->reset();
    return false;
}

void 
FactoryOps::removeCachedNotifyable(
    const shared_ptr<NotifyableImpl> &notifyableSP)
{
    TRACE(CL_LOG, "removeCachedNotifyable");
    
    Locker l(&notifyableSP->getSafeNotifyableMap()->getLock());
    {
        LOG_DEBUG(CL_LOG, 
                  "removeCachedNotifyable: state changing to REMOVED "
                  "for Notifyable %s",
                  notifyableSP->getKey().c_str());

        /* One way transition to removed, this is fine. */
        if (notifyableSP->getState() == Notifyable::REMOVED) {
            throw InvalidMethodException(
                string("removeCachedNotifyable: Tried to remove 2x ") +
                notifyableSP->getKey().c_str());
        }
        
        notifyableSP->setState(Notifyable::REMOVED);
    }

    notifyableSP->getSafeNotifyableMap()->erase(notifyableSP);
}

const Mutex &
FactoryOps::getClientsLock() const 
{
    TRACE(CL_LOG, "getClientsLock");
    return m_clLock; 
}

const Mutex &
FactoryOps::getTimersLock() const 
{ 
    TRACE(CL_LOG, "getTimersLock");
    return m_timerRegistryLock; 
}

const Mutex &
FactoryOps::getSyncEventLock() const 
{ 
    TRACE(CL_LOG, "getSyncEventLock");
    return m_syncEventLock; 
}

const Cond &
FactoryOps::getSyncEventCond() const
{ 
    TRACE(CL_LOG, "getSyncEventCond");
    return m_syncEventCond; 
}

const Mutex &
FactoryOps::getEndEventLock() const 
{
    TRACE(CL_LOG, "getEndEventLock");
    return m_endEventLock; 
}

NameList
FactoryOps::getChildrenNames(
    const string &notifyableKey,
    CachedObjectChangeHandlers::CachedObjectChange change)
{
    TRACE(CL_LOG, "getChildrenNames");

    NameList nameList;
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            notifyableKey,
            nameList,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(change)),
        m_zk.getNodeChildren(notifyableKey, nameList),
        change,
        notifyableKey,
        "Reading the value of %s failed: %s",
        notifyableKey.c_str(),
        true,
        true);
    
    for (NameList::iterator nameListIt = nameList.begin();
         nameListIt != nameList.end();
         ++nameListIt) {
        /*
         * Remove the key prefix
         */
        *nameListIt = nameListIt->substr(
            notifyableKey.length() + 
            CLString::KEY_SEPARATOR.length());
    }

    return nameList;
}

bool
FactoryOps::getNotifyableListWaitMsecs(
    const shared_ptr<NotifyableImpl> &parentSP,
    const string &registeredNotifyableName,
    const NameList &nameList,
    AccessType accessType,
    int64_t msecTimeout,
    NotifyableList *pNotifyableList)
{
    TRACE(CL_LOG, "getNotifyableListWaitMsecs");
    
    shared_ptr<NotifyableImpl> notifyableSP;
    NameList::const_iterator nameListIt;
    bool completed = false;
    for (nameListIt = nameList.begin(); 
         nameListIt != nameList.end(); 
         ++nameListIt) {
        completed = getNotifyableWaitMsecs(parentSP, 
                                           registeredNotifyableName, 
                                           *nameListIt, 
                                           accessType, 
                                           msecTimeout, 
                                           &notifyableSP);
        if ((completed == true) && (notifyableSP != NULL)) {
            pNotifyableList->push_back(
                dynamic_pointer_cast<Notifyable>(notifyableSP));
        }
    }

    return completed;
}

NotifyableList 
FactoryOps::getNotifyableList(const shared_ptr<NotifyableImpl> &parentSP,
                              const string &registeredNotifyableName,
                              const NameList &nameList,
                              AccessType accessType)
{
    NotifyableList notifyableList;
    getNotifyableListWaitMsecs(parentSP, 
                               registeredNotifyableName, 
                               nameList, 
                               accessType,
                               -1, 
                               &notifyableList);
    return notifyableList;
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

    LOG_DEBUG(CL_LOG,
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
    shared_ptr<NotifyableImpl> notifyableSP;
    string cachedObjectPath = ep->getPath();

    /* There are two cases where the getting the Notifyable is not
     * necessary.
     *
     * 1.  A sync event.
     * 2.  A lock node deleted event.
     */
    if (ep->getPath().compare(CLStringInternal::SYNC) == 0) {
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
    }
    else if ((ep->getPath().find(CLStringInternal::PARTIAL_LOCK_NODE) != 
          string::npos) && (etype == ZOO_DELETED_EVENT)) {
        notifyablePath = ep->getPath();
    }
    else {
        try {
            notifyablePath = getNotifyableKeyFromKey(ep->getPath());
            /*
             * Only try and affect notifyables in our cache, since
             * otherwise distributed locks must be held.  This will
             * hold the sync lock for notifyables for a short time.
             */
            notifyableSP = getNotifyableFromKey(
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
            if (notifyableSP == NULL) {
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
    Event e = fehp->deliver(notifyableSP, etype, cachedObjectPath);

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

}	/* End of 'namespace clusterlib' */
