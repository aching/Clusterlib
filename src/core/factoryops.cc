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
    : mp_root(NULL),
      m_syncEventId(0),
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
                 "Waiting for connect event from ZooKeeper up to %Ld msecs",
                 connectTimeout);
        if (m_firstConnect.predWaitUsecs(connectTimeout * 1000) == false) {
            LOG_ERROR(CL_LOG,
                      "FactoryOps: Did not receive connect event from %s in "
                      "time (%Ld msecs), aborting",
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
     * After all other initializations, get the root object.
     */
    getRoot();
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

    discardAllClients();
    discardAllDataDistributions();
    discardAllPropertyLists();
    discardAllQueues();
    discardAllApplications();
    discardAllGroups();
    discardAllNodes();
    discardAllRemovedNotifyables();
    delete mp_root;

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

/*
 * Wait for all my threads.
 */
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
    try {
        ClientImpl *cp = new ClientImpl(this);
        addClient(cp);
        return cp;
    } catch (Exception &e) {
	LOG_WARN(CL_LOG, 
                 "Couldn't create client because: %s", 
                 e.what());
        return NULL;
    }
}

Client *
FactoryOps::createJSONRPCResponseClient(Queue *responseQueue,
                                        Queue *completedQueue)
{
    TRACE(CL_LOG, "createJSONRPCResponseClient");

    ClientImpl *client = dynamic_cast<ClientImpl *>(createClient());
    assert(client);
    client->registerJSONRPCResponseHandler(responseQueue,
                                           completedQueue);
    return client;
}

Client *
FactoryOps::createJSONRPCMethodClient(Queue *recvQueue,
                                      Queue *completedQueue,
                                      ::json::rpc::JSONRPCManager *rpcManager)
{
    TRACE(CL_LOG, "createJSONRPCMethodClient");

    ClientImpl *client = dynamic_cast<ClientImpl *>(createClient());
    assert(client);
    client->registerJSONRPCMethodHandler(recvQueue,
                                         completedQueue,
                                         rpcManager);
    return client;
}


bool
FactoryOps::isConnected() const
{
    return m_connected;
}

/*
 * Try to synchronize with the underlying data store.
 */
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
              "synchronize: Starting sync with event id (%lld)", 
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

    LOG_DEBUG(CL_LOG, "synchronize: event id (%lld) Complete", syncEventId);
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

void
FactoryOps::removeClient(ClientImpl *clp)
{
    TRACE(CL_LOG, "removeClient");

    Locker l(getClientsLock());
    ClientImplList::iterator clIt = find(m_clients.begin(),
                                         m_clients.end(),
                                         clp);

    if (clIt == m_clients.end()) {
        return;
    }
    m_clients.erase(clIt);
}

void
FactoryOps::discardAllClients()
{
    TRACE(CL_LOG, "discardAllClients");

    Locker l(getClientsLock());
    ClientImplList::iterator clIt;
    for (clIt  = m_clients.begin();
         clIt != m_clients.end();
         clIt++) {
	delete *clIt;
    }
    m_clients.clear();
}

/*
 * Methods to clean up storage used by a FactoryOps.
 */
void
FactoryOps::discardAllDataDistributions()
{
    TRACE(CL_LOG, "discardAllDataDistributions");

    Locker l(getDataDistributionsLock());
    NotifyableImplMap::iterator distIt;

    for (distIt = m_dists.begin();
         distIt != m_dists.end();
         distIt++) {
        LOG_DEBUG(CL_LOG, 
                  "discardAllDataDistributions: Removed key (%s) with %d refs",
                  distIt->second->getKey().c_str(),
                  distIt->second->getRefCount());

	delete distIt->second;
    }
    m_dists.clear();
}
void
FactoryOps::discardAllPropertyLists()
{
    TRACE(CL_LOG, "discardAllPropertyLists");

    Locker l(getPropertyListLock());
    NotifyableImplMap::iterator propListIt;

    for (propListIt = m_propLists.begin();
         propListIt != m_propLists.end();
         propListIt++) {
        LOG_DEBUG(CL_LOG, 
                  "discardAllPropertyLists: Removed key (%s) with %d refs",
                  propListIt->second->getKey().c_str(),
                  propListIt->second->getRefCount());

	delete propListIt->second;
    }
    m_propLists.clear();
}
void
FactoryOps::discardAllQueues()
{
    TRACE(CL_LOG, "discardAllQueues");

    Locker l(getQueueLock());
    NotifyableImplMap::iterator queueIt;

    for (queueIt = m_queues.begin();
         queueIt != m_queues.end();
         queueIt++) {
        LOG_DEBUG(CL_LOG, 
                  "discardAllQueues: Removed key (%s) with %d refs",
                  queueIt->second->getKey().c_str(),
                  queueIt->second->getRefCount());

	delete queueIt->second;
    }
    m_queues.clear();
}
void
FactoryOps::discardAllApplications()
{
    TRACE(CL_LOG, "discardAllApplications");

    Locker l(getApplicationsLock());
    NotifyableImplMap::iterator aIt;

    for (aIt = m_apps.begin();
         aIt != m_apps.end();
         aIt++) {
        LOG_DEBUG(CL_LOG, 
                  "discardAllApplications: Removed key (%s) with %d refs",
                  aIt->second->getKey().c_str(),
                  aIt->second->getRefCount());
	delete aIt->second;
    }
    m_apps.clear();
}
void
FactoryOps::discardAllGroups()
{
    TRACE(CL_LOG, "discardAllGroups");

    Locker l(getGroupsLock());
    NotifyableImplMap::iterator gIt;

    for (gIt = m_groups.begin();
         gIt != m_groups.end();
         gIt++) {
        LOG_DEBUG(CL_LOG, 
                  "discardAllGroups: Removed key (%s) with %d refs",
                  gIt->second->getKey().c_str(),
                  gIt->second->getRefCount());
	delete gIt->second;
    }
    m_groups.clear();
}
void
FactoryOps::discardAllNodes()
{
    TRACE(CL_LOG, "discardAllNodes");

    Locker l(getNodesLock());
    NotifyableImplMap::iterator nIt;

    for (nIt = m_nodes.begin();
         nIt != m_nodes.end();
         nIt++) {
        LOG_DEBUG(CL_LOG, 
                  "discardAllNodes: Removed key (%s) with %d refs",
                  nIt->second->getKey().c_str(),
                  nIt->second->getRefCount());
	delete nIt->second;
    }
    m_nodes.clear();
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
    LOG_DEBUG(CL_LOG,
              "Starting thread with FactoryOps::dispatchExternalEvents(), "
              "this: 0x%x, thread: 0x%x",
              (int32_t) this,
              (uint32_t) pthread_self());

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
                      "[%d, 0x%x] dispatchExternalEvents() received "
                      "generic event of type: %s",
                      eventSeqId,
                      (unsigned int) this,
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
                                  "Dispatching timer event: 0x%x, id: "
                                  "%d, alarm time: %lld",
                                  (unsigned int) tp,
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
                                  "context: 0x%x, path: %s)",
                                  zk::ZooKeeperAdapter::getEventString(
                                      zp->getType()).c_str(),
                                  zp->getState(),
                                  (unsigned int) zp->getContext(),
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

        LOG_DEBUG(CL_LOG,
                  "Ending thread with FactoryOps::dispatchExternalEvents(): "
                  "this: 0x%x, thread: 0x%x",
                  (int32_t) this,
                  (uint32_t) pthread_self());
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
}

/*
 * Dispatch a timer event.
 */
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

/*
 * Dispatch a ZK event.
 */
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
              "will not propogate to clients, payload is %u",
              zp->getPath().c_str(),
              (uint32_t) uep);
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
                      "dispatchZKEvent: Sending payload %u to client %u "
                      "on key %s",
                      (uint32_t) uep,
                      (uint32_t) (*clIt),
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

    LOG_DEBUG(CL_LOG,
              "Starting thread with FactoryOps::consumeTimerEvents(), "
              "this: 0x%x, thread: 0x%x",
              (int32_t) this,
              (uint32_t) ::pthread_self());

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
                     "Serviced timer %d, handler 0x%x, client data 0x%x",
                     tepp->getId(), 
                     (int) tepp->getHandler(),
                     (int) tepp->getData());

            /*
             * Deallocate the payload object.
             */
            {
                Locker l(getTimersLock());

                m_timerRegistry.erase(tepp->getId());
                delete tepp;
            }
        }

        LOG_DEBUG(CL_LOG,
                  "Ending thread with FactoryOps::consumeTimerEvents(): "
                  "this: 0x%x, thread: 0x%x",
                  (int32_t) this,
                  (uint32_t) pthread_self());
    } catch (zk::ZooKeeperException &zke) {
        LOG_ERROR(CL_LOG, "ZooKeeperException: %s", zke.what());
        throw RepositoryInternalsFailureException(zke.what());
    } catch (Exception &e) {
        throw Exception(e.what());
    } catch (std::exception &stde) {
        LOG_ERROR(CL_LOG, "Unknown exception: %s", stde.what());
        throw Exception(stde.what());
    }        
}

/*
 * Methods to retrieve entities (or create them) from the
 * FactoryOps's cache.
 */
RootImpl *
FactoryOps::getRoot()
{
    TRACE(CL_LOG, "getRoot");

    if (mp_root != NULL) {
        return mp_root;
    }
    
    string key = NotifyableKeyManipulator::createRootKey();
    string applications = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::APPLICATIONS;
    
    SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                 "Could not create key %s: %s",
                 key.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(applications, "", 0),
                 "Could not create key %s: %s",
                 applications.c_str(),
                 true,
                 true);
    
    Locker l(getRootLock());
    mp_root = new RootImpl(this, key, "root");
    return mp_root;
}

/*
 * Methods to retrieve entities (or create them) from the
 * FactoryOps's cache.
 */
ApplicationImpl *
FactoryOps::getApplication(const string &name, bool create)
{
    TRACE(CL_LOG, "getApplication");
    
    /* 
     * Can accept the DEFAULT_CLI_APPLICATION (different than other names)
     */
    if ((name.compare(ClusterlibStrings::DEFAULT_CLI_APPLICATION)) &&
        (!NotifyableKeyManipulator::isValidNotifyableName(name))) {
        LOG_WARN(CL_LOG,
                 "getApplication: illegal application name %s",
                 name.c_str());
        if (create == true) {
            throw InvalidArgumentsException("getApplication: illegal name");
        }
        return NULL;
    }

    string key = NotifyableKeyManipulator::createApplicationKey(name);

    {
        Locker l(getApplicationsLock());
        NotifyableImplMap::const_iterator appIt = m_apps.find(key);

        if (appIt != m_apps.end()) {
            appIt->second->incrRefCount();
            return dynamic_cast<ApplicationImpl *>(appIt->second);
        }
    }

    ApplicationImpl *app = loadApplication(name, key);
    if (app != NULL) {
        return app;
    }
    if (create == true) {
        /*
         * Use a distributed lock on the parent to prevent another thread
         * from interfering with creation or removal.
         */
        getOps()->getDistributedLocks()->acquire(
            getRoot(), 
            ClusterlibStrings::NOTIFYABLELOCK);

        app = loadApplication(name, key);
        if (app == NULL) {
            app = createApplication(name, key);
        }

        getOps()->getDistributedLocks()->release(
            getRoot(),
            ClusterlibStrings::NOTIFYABLELOCK);
        return app;
    }

    LOG_WARN(CL_LOG,
             "getApplication: application %s not found nor created",
             name.c_str());

    return NULL;
}

DataDistributionImpl *
FactoryOps::getDataDistribution(const string &name,
                                GroupImpl *parentGroup,
                                bool create)
{
    TRACE(CL_LOG, "getDataDistribution");

    if (!NotifyableKeyManipulator::isValidNotifyableName(name)) {
        LOG_WARN(CL_LOG,
                 "getDataDistribution: Illegal data distribution name %s",
                 name.c_str());

        if (create == true) {
            throw InvalidArgumentsException(
                "getDataDistribution: illegal name");
        }
        return NULL;
    }

    if (parentGroup == NULL) {
        LOG_WARN(CL_LOG, "NULL parent group");
        throw InvalidArgumentsException("getDataDistribution: NULL parent");
    }

    string key = NotifyableKeyManipulator::createDataDistributionKey(
        parentGroup->getKey(),
        name);

    {
        Locker l(getDataDistributionsLock());
        NotifyableImplMap::const_iterator distIt = 
            m_dists.find(key);

        if (distIt != m_dists.end()) {
            distIt->second->incrRefCount();
            return dynamic_cast<DataDistributionImpl *>(distIt->second);
        }
    }

    DataDistributionImpl *dist = loadDataDistribution(name, 
                                                      key, 
                                                      parentGroup);
    if (dist != NULL) {
        return dist;
    }
    if (create == true) {
        /*
         * Use a distributed lock on the parent to prevent another thread
         * from interfering with creation or removal.
         */
        getOps()->getDistributedLocks()->acquire(
            parentGroup,
            ClusterlibStrings::NOTIFYABLELOCK);

        dist = loadDataDistribution(name, 
                                    key, 
                                    parentGroup);
        if (dist == NULL) {
            dist = createDataDistribution(name, key, "", parentGroup); 
        }

        getOps()->getDistributedLocks()->release(
            parentGroup,
            ClusterlibStrings::NOTIFYABLELOCK);
        return dist;
    }

    LOG_WARN(CL_LOG,
             "getDataDistribution: data distribution %s not found "
             "nor created",
             name.c_str());

    return NULL;
}

PropertyListImpl *
FactoryOps::getPropertyList(const string &name,
                            Notifyable *parent,
                            bool create)
{
    TRACE(CL_LOG, "getPropertyList");

    /* 
     * Can accept the DEFAULTPROPERTYLIST (different than other names)
     */
    if ((name.compare(ClusterlibStrings::DEFAULTPROPERTYLIST)) &&
        (!NotifyableKeyManipulator::isValidNotifyableName(name))) {
        LOG_WARN(CL_LOG,
                 "getPropertyList: Illegal property list name %s",
                 name.c_str());

        if (create == true) {
            throw InvalidArgumentsException("getPropertyList: illegal name");
        }
        return NULL;
    }

    if (parent == NULL) {
        LOG_ERROR(CL_LOG, "getPropertyList: NULL parent");                 
        throw InvalidArgumentsException("getPropertyList: NULL parent");
    }

    string key = 
        NotifyableKeyManipulator::createPropertyListKey(parent->getKey(),
                                                        name);
    
    {
        Locker l(getPropertyListLock());
        NotifyableImplMap::const_iterator propIt = m_propLists.find(key);

        if (propIt != m_propLists.end()) {
            propIt->second->incrRefCount();
            return dynamic_cast<PropertyListImpl *>(propIt->second);
        }
    }

    PropertyListImpl *propList = loadPropertyList(name, key, parent);
    if (propList != NULL) {
        return propList;
    }
    if (create == true) {
        /*
         * Use a distributed lock on the parent to prevent another thread
         * from interfering with creation or removal.
         */
        getOps()->getDistributedLocks()->acquire(
            parent,
            ClusterlibStrings::NOTIFYABLELOCK);
        
        propList = loadPropertyList(name, key, parent);
        if (propList == NULL) {
            propList = createPropertyList(name, key, parent);
        }

        getOps()->getDistributedLocks()->release(
            parent,
            ClusterlibStrings::NOTIFYABLELOCK);
        return propList;
    }

    LOG_WARN(CL_LOG,
             "getPropertyList: could not find nor create "
             "propertyList for %s",
             parent->getKey().c_str());

    return NULL;
}

QueueImpl *
FactoryOps::getQueue(const string &name,
                     Notifyable *parent,
                     bool create)
{
    TRACE(CL_LOG, "getQueue");

    if ((name.compare(ClusterlibStrings::DEFAULT_RECV_QUEUE) != 0) &&
        (name.compare(ClusterlibStrings::DEFAULT_RESP_QUEUE) != 0) &&
        (name.compare(ClusterlibStrings::DEFAULT_COMPLETED_QUEUE) != 0) &&
        !NotifyableKeyManipulator::isValidNotifyableName(name)) {
        LOG_WARN(CL_LOG,
                 "getQueue: Illegal queue name %s",
                 name.c_str());

        if (create == true) {
            throw InvalidArgumentsException("getQueue: illegal name");
        }
        return NULL;
    }

    if (parent == NULL) {
        LOG_ERROR(CL_LOG, "getQueue: NULL parent");                 
        throw InvalidArgumentsException("getQueue: NULL parent");
    }

    string key = 
        NotifyableKeyManipulator::createQueueKey(parent->getKey(),
                                                 name);
    
    {
        Locker l(getQueueLock());
        NotifyableImplMap::const_iterator propIt = m_queues.find(key);

        if (propIt != m_queues.end()) {
            propIt->second->incrRefCount();
            return dynamic_cast<QueueImpl *>(propIt->second);
        }
    }

    QueueImpl *queue = loadQueue(name, key, parent);
    if (queue != NULL) {
        return queue;
    }
    if (create == true) {
        /*
         * Use a distributed lock on the parent to prevent another thread
         * from interfering with creation or removal.
         */
        getOps()->getDistributedLocks()->acquire(
            parent,
            ClusterlibStrings::NOTIFYABLELOCK);
        
        queue = loadQueue(name, key, parent);
        if (queue == NULL) {
            queue = createQueue(name, key, parent);
        }

        getOps()->getDistributedLocks()->release(
            parent,
            ClusterlibStrings::NOTIFYABLELOCK);
        return queue;
    }

    LOG_WARN(CL_LOG,
             "getQueue: could not find nor create "
             "queue for %s",
             parent->getKey().c_str());

    return NULL;
}

GroupImpl *
FactoryOps::getGroup(const string &groupName,
                     GroupImpl *parentGroup,
                     bool create)
{
    TRACE(CL_LOG, "getGroup");

    if (!NotifyableKeyManipulator::isValidNotifyableName(groupName)) {
        LOG_WARN(CL_LOG,
                 "getGroup: Illegal group name %s",
                 groupName.c_str());
        if (create == true) {
            throw InvalidArgumentsException("getGroup: illegal name");
        }
        return NULL;
    }

    if (parentGroup == NULL) {
        LOG_WARN(CL_LOG, "getGroup: NULL parent group");
        throw InvalidArgumentsException("getGroup: NULL parent");
    }
    string key = 
        NotifyableKeyManipulator::createGroupKey(parentGroup->getKey(),
                                                 groupName);

    {
        Locker l(getGroupsLock());
        NotifyableImplMap::const_iterator groupIt = m_groups.find(key);

        if (groupIt != m_groups.end()) {
            groupIt->second->incrRefCount();
            return dynamic_cast<GroupImpl *>(groupIt->second);
        }
    }

    GroupImpl *group = loadGroup(groupName, key, parentGroup);
    if (group != NULL) {
        return group;
    }
    if (create == true) {
        /*
         * Use a distributed lock on the parent to prevent another thread
         * from interfering with creation or removal.
         */
        getOps()->getDistributedLocks()->acquire(
            parentGroup,
            ClusterlibStrings::NOTIFYABLELOCK);

        group = loadGroup(groupName, key, parentGroup);
        if (group == NULL) {
            group = createGroup(groupName, key, parentGroup);
        }

        getOps()->getDistributedLocks()->release(
            parentGroup,
            ClusterlibStrings::NOTIFYABLELOCK);
        return group;
    }

    LOG_WARN(CL_LOG,
             "getGroup: group %s not found nor created",
             groupName.c_str());

    return NULL;
}

NodeImpl *
FactoryOps::getNode(const string &nodeName,
                    GroupImpl *parentGroup,
                    bool create)
{
    TRACE(CL_LOG, "getNode");

    if (!NotifyableKeyManipulator::isValidNotifyableName(nodeName)) {
        LOG_WARN(CL_LOG,
                 "getNode: Illegal node name %s",
                 nodeName.c_str());
        if (create == true) {
            throw InvalidArgumentsException("getNode: illegal name");
        }
        return NULL;
    }

    if (parentGroup == NULL) {
        LOG_ERROR(CL_LOG, "getNode: NULL parent group");
        throw InvalidArgumentsException("getNode: NULL parent");
    }
    string key = NotifyableKeyManipulator::createNodeKey(parentGroup->getKey(),
                                                         nodeName);

    {
        Locker l(getNodesLock());
        NotifyableImplMap::const_iterator nodeIt = m_nodes.find(key);

        if (nodeIt != m_nodes.end()) {
            LOG_DEBUG(CL_LOG, 
                      "getNode: Found %s/%s in local cache.",
                      parentGroup->getKey().c_str(),
                      nodeName.c_str());
            nodeIt->second->incrRefCount();
            return dynamic_cast<NodeImpl *>(nodeIt->second);
        }
    }

    NodeImpl *node = loadNode(nodeName, key, parentGroup);
    if (node != NULL) {
        return node;
    }
    if (create == true) {
        /*
         * Use a distributed lock on the parent to prevent another thread
         * from interfering with creation and removal.
         */
        getOps()->getDistributedLocks()->acquire(
            parentGroup,
            ClusterlibStrings::NOTIFYABLELOCK);

        node = loadNode(nodeName, key, parentGroup);
        if (node == NULL) {
            node = createNode(nodeName, key, parentGroup);
        }
        
        getOps()->getDistributedLocks()->release(
            parentGroup,
            ClusterlibStrings::NOTIFYABLELOCK);
        return node;
    }

    LOG_WARN(CL_LOG,
             "getNode: node %s not found nor created",
             nodeName.c_str());

    return NULL;
}

ProcessSlotImpl *
FactoryOps::getProcessSlot(const string &processSlotName,
                    NodeImpl *parentNode,
                    bool create)
{
    TRACE(CL_LOG, "getPropertySlot");

    if (!NotifyableKeyManipulator::isValidNotifyableName(processSlotName)) {
        LOG_WARN(CL_LOG,
                 "getPropertySlot: Illegal processslot name %s",
                 processSlotName.c_str());
        if (create == true) {
            throw InvalidArgumentsException("getProcessSlot: illegal name");
        }
        return NULL;
    }

    if (parentNode == NULL) {
        LOG_ERROR(CL_LOG, "getProcessSlot: NULL parent node");
        throw InvalidArgumentsException("getProcessSlot: NULL parent");
    }
    string key = NotifyableKeyManipulator::createProcessSlotKey(
        parentNode->getKey(),
        processSlotName);

    {
        Locker l(getProcessSlotsLock());
        NotifyableImplMap::const_iterator processSlotIt 
            = m_processSlots.find(key);

        if (processSlotIt != m_processSlots.end()) {
            LOG_DEBUG(CL_LOG, 
                      "getProcessSlot: Found %s/%s in local cache.",
                      parentNode->getKey().c_str(),
                      processSlotName.c_str());
            processSlotIt->second->incrRefCount();
            return dynamic_cast<ProcessSlotImpl *>(processSlotIt->second);
        }
    }

    ProcessSlotImpl *processSlot = loadProcessSlot(processSlotName, 
                                                   key, 
                                                   parentNode);
    if (processSlot != NULL) {
        return processSlot;
    }
    if (create == true) {
        /*
         * Use a distributed lock on the parent to prevent another thread
         * from interfering with creation and removal.
         */
        getOps()->getDistributedLocks()->acquire(
            parentNode,
            ClusterlibStrings::NOTIFYABLELOCK);

        processSlot = loadProcessSlot(processSlotName, key, parentNode);
        if (processSlot == NULL) {
            processSlot = createProcessSlot(processSlotName, key, parentNode);
        }
        
        getOps()->getDistributedLocks()->release(
            parentNode,
            ClusterlibStrings::NOTIFYABLELOCK);
        return processSlot;
    }

    LOG_WARN(CL_LOG,
             "getProcessSlot: process slot %s not found nor created",
             processSlotName.c_str());

    return NULL;
}

/*
 * Update the fields of a distribution in the clusterlib repository.
 */
void
FactoryOps::updateDataDistribution(const string &distKey,
                                   const string &shards,
                                   int32_t version,
                                   int32_t &finalVersion)
{
    TRACE(CL_LOG, "updateDataDistribution");
    
    string snode = 
	distKey +
        ClusterlibStrings::KEYSEPARATOR +
	ClusterlibStrings::SHARDS;
    Stat stat;
    bool exists = false;

    /*
     * Update the shards.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(snode)),
                 "Could not determine whether key %s exists: %s",
                 snode.c_str(),
                 false,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(snode, shards, 0),
                     "Creation of %s failed: %s",
                     snode.c_str(),
                     true,
                     true);
    }

    SAFE_CALL_ZK(m_zk.setNodeData(snode, shards, version, &stat),
                 "Setting of %s failed: %s",
                 snode.c_str(),
                 false,
                 true);

    SAFE_CALLBACK_ZK(
        m_zk.nodeExists(
            snode,
            &m_zkEventAdapter, 
            getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::SHARDS_CHANGE)),
        ,
        CachedObjectChangeHandlers::SHARDS_CHANGE,
        snode,
        "Reestablishing watch on value of %s failed: %s",
        snode.c_str(),
        true,
        true);

    finalVersion = stat.version;
}

/*
 * Update the propertyList of a notifyable object in the clusterlib repository
 */
void
FactoryOps::updatePropertyList(const string &propListKey,
                             const string &propListValue,
                             int32_t version,
                             int32_t &finalVersion)
{
    TRACE(CL_LOG, "updatePropertyList");

    string kvnode =
        propListKey +
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::KEYVAL;

    Stat stat;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(kvnode)),
                 "Could not determine whether key %s exists: %s",
                 kvnode.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(kvnode, propListValue, 0),
                     "Creation of %s failed: %s",
                     kvnode.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(kvnode,
                                  propListValue, 
                                  version, 
                                  &stat),
                 "Setting of %s failed: %s",
                 kvnode.c_str(),
                 false,
                 true);
    SAFE_CALLBACK_ZK(
        m_zk.nodeExists(
            kvnode,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROPERTYLIST_VALUES_CHANGE)),
        ,
        CachedObjectChangeHandlers::PROPERTYLIST_VALUES_CHANGE,
        kvnode,
        "Reestablishing watch on value of %s failed: %s",
        propListKey.c_str(),
        false,
        true);  
    
    finalVersion = stat.version;
}

/*
 * Update the client state field of a node.
 */
void
FactoryOps::updateNodeClientState(const string &nodeKey,
                                  const string &cs)
{
    TRACE(CL_LOG, "updateNodeClientState");

    string csKey = 
        nodeKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(csKey)),
                 "Could not determine whether key %s exists: %s",
                 csKey.c_str(),
                 true,
                 true);

    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(csKey, cs, 0),
                     "Creation of %s failed: %s",
                     csKey.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(csKey, cs),
                 "Setting of %s failed: %s",
                 csKey.c_str(),
                 false,
                 true);

    SAFE_CALLBACK_ZK(
        m_zk.nodeExists(
            csKey,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::NODE_CLIENT_STATE_CHANGE)),
        ,
        CachedObjectChangeHandlers::NODE_CLIENT_STATE_CHANGE,
        csKey,
        "Reestablishing watch on value of %s failed: %s",
        csKey.c_str(),
        false,
        true);
}

/*
 * Update the client state description field for
 * a node.
 */
void
FactoryOps::updateNodeClientStateDesc(const string &nodeKey,
                                      const string &desc)
{
    TRACE(CL_LOG, "updateNodeClientStateDesc");

    string csKey = 
        nodeKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTSTATEDESC;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(csKey)),
                 "Could not determine whether key %s exists: %s",
                 csKey.c_str(),
                 true,
                 true);

    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(csKey, desc, 0),
                     "Creation of %s failed: %s",
                     csKey.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(csKey, desc),
                 "Setting of %s failed: %s",
                 csKey.c_str(),
                 false,
                 true);

    /*
     * NO WATCHER -- do we need one?
     */
}

/*
 * Update the master state field of a node.
 */
void
FactoryOps::updateNodeMasterSetState(const string &nodeKey,
                                     const string &ms)
{
    TRACE(CL_LOG, "updateNodeMasterSetState");

    string msKey = 
        nodeKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::MASTERSETSTATE;
    bool exists = false;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(msKey)),
                 "Could not determine whether key %s exists: %s",
                 msKey.c_str(),
                 true,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(m_zk.createNode(msKey, ms, 0),
                     "Creation of %s failed: %s",
                     msKey.c_str(),
                     true,
                     true);
    }
    SAFE_CALL_ZK(m_zk.setNodeData(msKey, ms),
                 "Setting of %s failed: %s",
                 msKey.c_str(),
                 true,
                 true);
    SAFE_CALLBACK_ZK(
        m_zk.nodeExists(
            msKey,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::NODE_MASTER_SET_STATE_CHANGE)),
        ,
        CachedObjectChangeHandlers::NODE_MASTER_SET_STATE_CHANGE,
        msKey,
        "Reestablishing watch on value of %s failed: %s",
        msKey.c_str(),
        false,
        true);
}

/*
 * Get (& potentially load) cache objects given a key.
 */

NotifyableImpl *
FactoryOps::getNotifyableFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getNotifyableFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    LOG_DEBUG(CL_LOG, "getNotifyableFromKey: key %s", key.c_str());
    return getNotifyableFromComponents(components, -1, create);
}
NotifyableImpl *
FactoryOps::getNotifyableFromComponents(const vector<string> &components,
                                        int32_t elements,
                                        bool create)
{
    TRACE(CL_LOG, "getNotifyableFromComponents");

    if (elements == -1) {
        elements = components.size();
    }

    NotifyableImpl *ntp = NULL;    
    int32_t clusterObjectElements = elements;

    LOG_DEBUG(CL_LOG, "getNotifyableFromComponents: elements %d", elements);

    ntp = getRootFromComponents(components,
                                clusterObjectElements);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found Root");
        return ntp;
    }
    ntp = getApplicationFromComponents(components,
                                       clusterObjectElements, 
                                       create);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found Application");
        return ntp;
    }
    ntp = getPropertyListFromComponents(components,
                                        clusterObjectElements, 
                                        create);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found PropertyList");
        return ntp;
    }
    ntp = getQueueFromComponents(components,
                                 clusterObjectElements, 
                                 create);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found Queue");
        return ntp;
    }
    ntp = getDataDistributionFromComponents(components,
                                            clusterObjectElements, 
                                            create);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found DataDistribution");
        return ntp;
    }
    ntp = getGroupFromComponents(components,
                                 clusterObjectElements, 
                                 create);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found Group");
        return ntp;
    }
    ntp = getNodeFromComponents(components, 
                                clusterObjectElements, 
                                create);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found Node");
        return ntp;
    }
    ntp = getProcessSlotFromComponents(components, 
                                       clusterObjectElements, 
                                       create);
    if (ntp) {
        LOG_DEBUG(CL_LOG, 
                  "getNotifyableFromComponents: found ProcessSlot");
        return ntp;
    }
    
    return NULL;
}

RootImpl *
FactoryOps::getRootFromKey(const string &key)
{
    TRACE(CL_LOG, "getRootFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getRootFromComponents(components, -1);
}
RootImpl *
FactoryOps::getRootFromComponents(const vector<string> &components, 
                                  int32_t elements)
{
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isRootKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getRootFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getRootFromComponents: root name = %s", 
              components.at(elements - 1).c_str());

    return getRoot();
}

ApplicationImpl *
FactoryOps::getApplicationFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getApplicationFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getApplicationFromComponents(components, -1, create);
}
ApplicationImpl *
FactoryOps::getApplicationFromComponents(const vector<string> &components, 
                                         int32_t elements,
                                         bool create)
{
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isApplicationKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getApplicationFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getApplicationFromComponents: application name = %s", 
              components.at(elements - 1).c_str());

    return getApplication(components.at(elements - 1), create);
}

DataDistributionImpl *
FactoryOps::getDataDistributionFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getDataDistributionFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getDataDistributionFromComponents(components, -1, create);
}
DataDistributionImpl *
FactoryOps::getDataDistributionFromComponents(
    const vector<string> &components, 
    int32_t elements,
    bool create)
{
    TRACE(CL_LOG, "getDataDistributionFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isDataDistributionKey(components, 
                                                         elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getDataDistributionFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    GroupImpl *parent = getGroupFromComponents(components,
                                               parentGroupCount,
                                               create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getDataDistributionFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getDataDistributionFromComponents: parent key = %s, "
              "distribution name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    parent->releaseRef();
    return getDataDistribution(components.at(elements - 1),
                               parent,
                               create);
}

PropertyListImpl *
FactoryOps::getPropertyListFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getPropertyListFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getPropertyListFromComponents(components, -1, create);
}
PropertyListImpl *
FactoryOps::getPropertyListFromComponents(const vector<string> &components, 
                                          int32_t elements,
                                          bool create)
{
    TRACE(CL_LOG, "getPropertyListFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

   if (!NotifyableKeyManipulator::isPropertyListKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getPropertyListFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    Notifyable *parent = getNotifyableFromComponents(components,
                                                     parentGroupCount,
                                                     create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getPropertyListFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getPropertyListFromComponents: parent key = %s, "
              "property list name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    if (dynamic_cast<Root *>(parent) == NULL) {
        parent->releaseRef();
    }
    return getPropertyList(components.at(elements - 1),
                         parent,
                         create);
}

QueueImpl *
FactoryOps::getQueueFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getQueueFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getQueueFromComponents(components, -1, create);
}
QueueImpl *
FactoryOps::getQueueFromComponents(const vector<string> &components, 
                                   int32_t elements,
                                   bool create)
{
    TRACE(CL_LOG, "getQueueFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

   if (!NotifyableKeyManipulator::isQueueKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getQueueFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    Notifyable *parent = getNotifyableFromComponents(components,
                                                     parentGroupCount,
                                                     create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getQueueFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getQueueFromComponents: parent key = %s, "
              "property list name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    if (dynamic_cast<Root *>(parent) == NULL) {
        parent->releaseRef();
    }
    return getQueue(components.at(elements - 1),
                         parent,
                         create);
}

GroupImpl *
FactoryOps::getGroupFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getGroupFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getGroupFromComponents(components, -1, create);
}
GroupImpl *
FactoryOps::getGroupFromComponents(const vector<string> &components, 
                                   int32_t elements,
                                   bool create)
{
    TRACE(CL_LOG, "getGroupFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /* 
     * Could be either an Application or a Group, need to check both.
     * If is a Group and not an Application, then it must have a
     * parent that is either an Application or Group.  If the parent
     * is a Group, this function will call itself recursively.
     */
    if (NotifyableKeyManipulator::isApplicationKey(components, elements)) {
        return getApplicationFromComponents(components, elements, create);
    }
    
    if (!NotifyableKeyManipulator::isGroupKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getGroupFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    GroupImpl *parent = getGroupFromComponents(components,
                                               parentGroupCount,
                                               create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getGroupFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }
    
    LOG_DEBUG(CL_LOG, 
              "getGroupFromComponents: parent key = %s, group name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    parent->releaseRef();
    return getGroup(components.at(elements - 1),
                   parent,
                   create);
}

NodeImpl *
FactoryOps::getNodeFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getNodeFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getNodeFromComponents(components, -1, create);
}
NodeImpl *
FactoryOps::getNodeFromComponents(const vector<string> &components, 
                                  int32_t elements,
                                  bool create)
{
    TRACE(CL_LOG, "getNodeFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isNodeKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getNodeFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    GroupImpl *parent = getGroupFromComponents(components,
                                               parentGroupCount,
                                               create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getNodeFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getNodeFromComponents: parent key = %s, node name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    parent->releaseRef();
    return getNode(components.at(elements - 1),
                   parent,
                   create);
}

ProcessSlotImpl *
FactoryOps::getProcessSlotFromKey(const string &key, bool create)
{
    TRACE(CL_LOG, "getProcessSlotFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getProcessSlotFromComponents(components, -1, create);
}
ProcessSlotImpl *
FactoryOps::getProcessSlotFromComponents(const vector<string> &components, 
                                         int32_t elements,
                                         bool create)
{
    TRACE(CL_LOG, "getProcessSlotFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!NotifyableKeyManipulator::isProcessSlotKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getProcessSlotFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }
    
    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }
    NodeImpl *parent = getNodeFromComponents(components,
                                             parentGroupCount,
                                             create);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getProcessSlotFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getProcessSlotFromComponents: parent key = %s, "
              "process slot name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    parent->releaseRef();
    return getProcessSlot(components.at(elements - 1),
                          parent,
                          create);
}

/*
 * Entity loading from ZooKeeper. Also add it to the global cache.
 */
ApplicationImpl *
FactoryOps::loadApplication(const string &name,
                            const string &key)
{
    TRACE(CL_LOG, "loadApplication");

    vector<string> zkNodes;
    ApplicationImpl *app;
    bool exists = false;
    Locker l(getApplicationsLock());

    NotifyableImplMap::const_iterator appIt = m_apps.find(key);
    if (appIt != m_apps.end()) {
        appIt->second->incrRefCount();
        return dynamic_cast<ApplicationImpl *>(appIt->second);
    }

    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    string groups = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::GROUPS;
    string dists = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::DISTRIBUTIONS;
    string nodes =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::NODES;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(groups)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadApplication: Application with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 groups.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(dists)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadApplication: Application with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 dists.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(nodes)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadApplication: Application with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 nodes.c_str());
        return NULL;
    }

    app = new ApplicationImpl(this, 
                              key, 
                              name, 
                              getRoot());
    app->initializeCachedRepresentation();

    /*
     * Set up handler for changes in Notifyable state.
     */
    establishNotifyableStateChange(app);

    m_apps[key] = app;

    return app;
}
    
DataDistributionImpl *
FactoryOps::loadDataDistribution(const string &distName,
                                 const string &distKey,
                                 GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "loadDataDistribution");

    DataDistributionImpl *dist;
    bool exists = false;

    /*
     * Ensure that we have a cached object for this data
     * distribution in the cache.
     */
    Locker l(getDataDistributionsLock());

    NotifyableImplMap::const_iterator distIt = 
        m_dists.find(distKey);
    if (distIt != m_dists.end()) {
        distIt->second->incrRefCount();
        return dynamic_cast<DataDistributionImpl *>(distIt->second);
    }

    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    string shards = 
        distKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::SHARDS;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(distKey)),
                 "Could not determine whether key %s exists: %s",
                 distKey.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(shards)),
                 "Could not determine whether key %s exists: %s",
                 distKey.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadGroup: Group with key %s is not fully constructed "
                 "(%s missing)",
                 distKey.c_str(),
                 shards.c_str());
        return NULL;
    }

    dist = new DataDistributionImpl(this,
                                    distKey,
                                    distName,
                                    parentGroup);
    dist->initializeCachedRepresentation();

    /*                                                                         
     * Set up handler for changes in Notifyable state.
     */
    establishNotifyableStateChange(dist);

    m_dists[distKey] = dist;

    return dist;
}

string
FactoryOps::loadShards(const string &key, int32_t &version)
{
    Stat stat;
    string snode =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::SHARDS;
    string res;

    version = 0;
    SAFE_CALLBACK_ZK(
        m_zk.getNodeData(
            snode,
            res,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::SHARDS_CHANGE),
            &stat),
        m_zk.getNodeData(snode, res, NULL, NULL, &stat),
        CachedObjectChangeHandlers::SHARDS_CHANGE,
        snode,
        "Loading shards from %s failed: %s",
        snode.c_str(),
        false,
        true);
    version = stat.version;
    return res;
}

PropertyListImpl *
FactoryOps::loadPropertyList(const string &propListName, 
                             const string &propListKey,
                             Notifyable *parent)
{
    TRACE(CL_LOG, "PropertyList");

    PropertyListImpl *propList;
    bool exists = false;
    Locker l(getPropertyListLock());

    NotifyableImplMap::const_iterator propIt =
        m_propLists.find(propListKey);
    if (propIt != m_propLists.end()) {
        propIt->second->incrRefCount();
        return dynamic_cast<PropertyListImpl *>(propIt->second);
    }

    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(propListKey)),
                 "Could not determine whether key %s exists: %s",
                 propListKey.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }

    propList = new PropertyListImpl(this,
                               propListKey,
                               propListName,
                               dynamic_cast<NotifyableImpl *>(parent));
    propList->initializeCachedRepresentation();
    
    /*
     * Set up handler for changes in Notifyable state.
     */
    establishNotifyableStateChange(propList);

    m_propLists[propListKey] = propList;

    return propList;
}

QueueImpl *
FactoryOps::loadQueue(const string &queueName, 
                      const string &queueKey,
                      Notifyable *parent)
{
    TRACE(CL_LOG, "Queue");

    QueueImpl *queue;
    bool exists = false;
    Locker l(getQueueLock());

    NotifyableImplMap::const_iterator queueIt =
        m_queues.find(queueKey);
    if (queueIt != m_queues.end()) {
        queueIt->second->incrRefCount();
        return dynamic_cast<QueueImpl *>(queueIt->second);
    }

    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    SAFE_CALL_ZK((exists = m_zk.nodeExists(queueKey)),
                 "Could not determine whether key %s exists: %s",
                 queueKey.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }

    queue = new QueueImpl(this,
                          queueKey,
                          queueName,
                          dynamic_cast<NotifyableImpl *>(parent));
    queue->initializeCachedRepresentation();
    
    /*
     * Set up handler for changes in Notifyable state.
     */
    establishNotifyableStateChange(queue);

    m_queues[queueKey] = queue;

    return queue;
}

string
FactoryOps::loadKeyValMap(const string &key, int32_t &version)
{
    Stat stat;

    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    string kvnode =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::KEYVAL;

    string kv;
    SAFE_CALLBACK_ZK(
        m_zk.getNodeData(
            kvnode,
            kv,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROPERTYLIST_VALUES_CHANGE),
            &stat),
        m_zk.getNodeData(kvnode, kv, NULL, NULL, &stat),
        CachedObjectChangeHandlers::PROPERTYLIST_VALUES_CHANGE,
        kvnode,
        "Reading the value of %s failed: %s",
        key.c_str(),
        false,
        true);

    version = stat.version;

    return kv;
}

GroupImpl *
FactoryOps::loadGroup(const string &groupName,
                      const string &groupKey,
                      GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "loadGroup");

    GroupImpl *group;
    bool exists = false;
    Locker l(getGroupsLock());

    NotifyableImplMap::const_iterator groupIt = m_groups.find(groupKey);
    if (groupIt != m_groups.end()) {
        groupIt->second->incrRefCount();
        return dynamic_cast<GroupImpl *>(groupIt->second);
    }

    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    string nodes = 
        groupKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::NODES;
    string groups =
        groupKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::GROUPS;
    string dists =
        groupKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::DISTRIBUTIONS;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(groupKey)),
                 "Could not determine whether key %s exists: %s",
                 groupKey.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(nodes)),
                 "Could not determine whether key %s exists: %s",
                 groupKey.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadGroup: Group with key %s is not fully constructed "
                 "(%s missing)",
                 groupKey.c_str(),
                 nodes.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(groups)),
                 "Could not determine whether key %s exists: %s",
                 groupKey.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadGroup: Group with key %s is not fully constructed "
                 "(%s missing)",
                 groupKey.c_str(),
                 groups.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(dists)),
                 "Could not determine whether key %s exists: %s",
                 groupKey.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadGroup: Group with key %s is not fully constructed "
                 "(%s missing)",
                 groupKey.c_str(),
                 groups.c_str());
        return NULL;
    }

    group = new GroupImpl(this,
                          groupKey,
                          groupName,
                          parentGroup);
    group->initializeCachedRepresentation();

    /*
     * Set up handler for changes in Notifyable state.
     */
    establishNotifyableStateChange(group);

    m_groups[groupKey] = group;

    return group;
}

NodeImpl *
FactoryOps::loadNode(const string &name,
                     const string &key,
                     GroupImpl *group)
{
    TRACE(CL_LOG, "loadNode");

    NodeImpl *node;
    bool exists = false;
    Locker l(getNodesLock());

    NotifyableImplMap::const_iterator nodeIt = m_nodes.find(key);
    if (nodeIt != m_nodes.end()) {
        nodeIt->second->incrRefCount();
        return dynamic_cast<NodeImpl *>(nodeIt->second);
    }
    
    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    string cs = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTSTATE;
    string ms = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::MASTERSETSTATE;
    string cv = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTVERSION;
    string up =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::PROCESSSLOTSUSAGE;
    string mp =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::PROCESSSLOTSMAX;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(cs)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadNode: Node with key %s is not fully constructed "
                 "(%s missing)",
                 key.c_str(),
                 cs.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(ms)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadNode: Node with key %s is not fully constructed "
                 "(%s missing)",
                 key.c_str(),
                 ms.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(cv)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadNode: Node with key %s is not fully constructed "
                 "(%s missing)",
                 key.c_str(),
                 cv.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(up)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadNode: Node with key %s is not fully constructed "
                 "(%s missing)",
                 key.c_str(),
                 up.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(mp)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadNode: Node with key %s is not fully constructed "
                 "(%s missing)",
                 key.c_str(),
                 mp.c_str());
        return NULL;
    }

    node = new NodeImpl(this, key, name, group);
    node->initializeCachedRepresentation();

    /*
     * Set up handler for changes in Notifyable state.
     */
    establishNotifyableStateChange(node);

    m_nodes[key] = node;

    return node;
}

ProcessSlotImpl *
FactoryOps::loadProcessSlot(const string &name,
                            const string &key,
                            NodeImpl *node)
{
    TRACE(CL_LOG, "loadProcessSlot");

    ProcessSlotImpl *processSlot;
    bool exists = false;
    Locker l(getProcessSlotsLock());

    NotifyableImplMap::const_iterator processSlotIt = m_processSlots.find(key);
    if (processSlotIt != m_processSlots.end()) {
        processSlotIt->second->incrRefCount();
        return dynamic_cast<ProcessSlotImpl *>(processSlotIt->second);
    }
    
    /* 
     * Make sure that all the Zookeeper nodes exist that are part of
     * this object.
     */
    string pv = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTPORTVEC;
    string ea = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTEXECARGS;
    string rea = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTRUNNINGEXECARGS;
    string pid = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTPID;
    string ds = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTDESIREDSTATE;
    string cs = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTCURRENTSTATE;
    string res = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTRESERVATION;

    SAFE_CALL_ZK((exists = m_zk.nodeExists(key)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(pv)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadProcessSlot: ProcessSlot with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 pv.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(ea)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadProcessSlot: ProcessSlot with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 ea.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(rea)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadProcessSlot: ProcessSlot with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 rea.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(pid)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadProcessSlot: ProcessSlot with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 pid.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(ds)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadProcessSlot: ProcessSlot with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 ds.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(cs)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadProcessSlot: ProcessSlot with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 cs.c_str());
        return NULL;
    }
    SAFE_CALL_ZK((exists = m_zk.nodeExists(res)),
                 "Could not determine whether key %s exists: %s",
                 key.c_str(),
                 false,
                 true);
    if (!exists) {
        LOG_WARN(CL_LOG, 
                 "loadProcessSlot: ProcessSlot with key %s is not fully "
                 "constructed (%s missing)",
                 key.c_str(),
                 res.c_str());
        return NULL;
    }

    processSlot = new ProcessSlotImpl(this, key, name, node);
    processSlot->initializeCachedRepresentation();

    /*
     * Set up handler for changes in Notifyable state.
     */
    establishNotifyableStateChange(processSlot);

    m_processSlots[key] = processSlot;

    return processSlot;
}

/*
 * Entity creation in ZooKeeper.
 */
ApplicationImpl *
FactoryOps::createApplication(const string &name, const string &key)
{
    TRACE(CL_LOG, "createApplication");
    
    string groups = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::GROUPS;
    string dists = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::DISTRIBUTIONS;
    string nodes =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::NODES;

    /*
     * Create the application data structure.
     */
    SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                 "Could not create key %s: %s",
                 key.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(groups, "", 0),
                 "Could not create key %s: %s",
                 groups.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(dists, "", 0),
                 "Could not create key %s: %s",
                 dists.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(nodes, "", 0),
                 "Could not create key %s: %s",
                 nodes.c_str(),
                 true,
                 true);

    /*
     * Load the application object, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    return loadApplication(name, key);
}

DataDistributionImpl *
FactoryOps::createDataDistribution(const string &distName,
                                   const string &distKey,
                                   const string &marshalledShards,
                                   GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "createDataDistribution");

    string shards = 
        distKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::SHARDS;

    SAFE_CALL_ZK(m_zk.createNode(distKey, "", 0),
                 "Could not create key %s: %s",
                 distKey.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(shards, "", 0),
                 "Could not create key %s: %s",
                 shards.c_str(),
                 true,
                 true);
    
    /*
     * Load the distribution, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    return loadDataDistribution(distName, distKey, parentGroup);
}

PropertyListImpl *
FactoryOps::createPropertyList(const string &propListName,
                               const string &propListKey,
                               Notifyable *parent) 
{
    TRACE(CL_LOG, "createPropertyList");

    string kv = 
        propListKey + 
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::KEYVAL;

    SAFE_CALL_ZK(m_zk.createNode(propListKey, "", 0),
                 "Could not create key %s: %s",
                 propListKey.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(kv, "", 0),
                 "Could not create key %s: %s",
                 kv.c_str(),
                 true,
                 true);
    
    /*
     * Load the propertyList, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    return loadPropertyList(propListName, propListKey, parent);
}

QueueImpl *
FactoryOps::createQueue(const string &queueName,
                        const string &queueKey,
                        Notifyable *parent) 
{
    TRACE(CL_LOG, "createQueue");

    SAFE_CALL_ZK(m_zk.createNode(queueKey, "", 0),
                 "Could not create key %s: %s",
                 queueKey.c_str(),
                 true,
                 true);
    
    /*
     * Load the queue, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    return loadQueue(queueName, queueKey, parent);
}

GroupImpl *
FactoryOps::createGroup(const string &groupName, 
                        const string &groupKey,
                        GroupImpl *parentGroup)
{
    TRACE(CL_LOG, "createGroup");

    string nodes = 
        groupKey + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::NODES;
    string groups =
        groupKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::GROUPS;
    string dists =
        groupKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::DISTRIBUTIONS;

    SAFE_CALL_ZK(m_zk.createNode(groupKey, "", 0),
                 "Could not create key %s: %s",
                 groupKey.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(nodes, "", 0),
                 "Could not create key %s: %s",
                 nodes.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(groups, "", 0),
                 "Could not create key %s: %s",
                 groups.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(dists, "", 0),
                 "Could not create key %s: %s",
                 dists.c_str(),
                 true,
                 true);
    
    /*
     * Load the group, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    return loadGroup(groupName, groupKey, parentGroup);
}

NodeImpl *
FactoryOps::createNode(const string &name,
                       const string &key, 
                       GroupImpl *group)
{
    TRACE(CL_LOG, "createNode");

    string cs = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTSTATE;
    string ms = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::MASTERSETSTATE;
    string cv = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::CLIENTVERSION;
    string up =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::PROCESSSLOTSUSAGE;
    string mp =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::PROCESSSLOTSMAX;

    SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                 "Could not create key %s: %s",
                 key.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(cs, "", 0),
                 "Could not create key %s: %s",
                 cs.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(ms, "", 0),
                 "Could not create key %s: %s",
                 ms.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(cv, ClusterlibStrings::CLUSTERLIBVERSION, 0),
                 "Could not create key %s: %s",
                 cv.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(up, "", 0),
                 "Could not create key %s: %s",
                 up.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(mp, "", 0),
                 "Could not create key %s: %s",
                 up.c_str(),
                 true,
                 true);

    /*
     * Load the node, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    return loadNode(name, key, group);
}

ProcessSlotImpl *
FactoryOps::createProcessSlot(const string &name,
                              const string &key, 
                              NodeImpl *node)
{
    TRACE(CL_LOG, "createProcessSlot");

    string pv = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTPORTVEC;
    string ea = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTEXECARGS;
    string rea = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTRUNNINGEXECARGS;
    string pid = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTPID;
    string ds = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTDESIREDSTATE;
    string cs = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTCURRENTSTATE;
    string res = 
        key + 
        ClusterlibStrings::KEYSEPARATOR + 
        ClusterlibStrings::PROCESSSLOTRESERVATION;

    SAFE_CALL_ZK(m_zk.createNode(key, "", 0),
                 "Could not create key %s: %s",
                 key.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(pv, "", 0),
                 "Could not create key %s: %s",
                 pv.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(ea, 
                                 ProcessSlotImpl::createDefaultExecArgs(),
                                 0),
                 "Could not create key %s: %s",
                 ea.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(rea, 
                                 ProcessSlotImpl::createDefaultExecArgs(),
                                 0),
                 "Could not create key %s: %s",
                 rea.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(pid, "", 0),
                 "Could not create key %s: %s",
                 pid.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(ds, "", 0),
                 "Could not create key %s: %s",
                 ds.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(cs, "", 0),
                 "Could not create key %s: %s",
                 cs.c_str(),
                 true,
                 true);
    SAFE_CALL_ZK(m_zk.createNode(res, "", 0),
                 "Could not create key %s: %s",
                 res.c_str(),
                 true,
                 true);

    /*
     * Load the process slot, which will load the data,
     * establish all the event notifications, and add the
     * object to the cache.
     */
    return loadProcessSlot(name, key, node);
}

void
FactoryOps::removeApplication(ApplicationImpl *app)
{
    TRACE(CL_LOG, "removeApplication");

    SAFE_CALL_ZK(m_zk.deleteNode(app->getKey(), true),
                 "Could not delete key %s: %s",
                 app->getKey().c_str(),
                 false,
                 true);
}

void
FactoryOps::removeDataDistribution(DataDistributionImpl *dist)
{
    TRACE(CL_LOG, "removeDataDistribution");    

    SAFE_CALL_ZK(m_zk.deleteNode(dist->getKey(), true),
                 "Could not delete key %s: %s",
                 dist->getKey().c_str(),
                 false,
                 true);
}

void
FactoryOps::removePropertyList(PropertyListImpl *propList)
{
    TRACE(CL_LOG, "removePropertyList");    

    SAFE_CALL_ZK(m_zk.deleteNode(propList->getKey(), true),
                 "Could not delete key %s: %s",
                 propList->getKey().c_str(),
                 false,
                 true);
}

void
FactoryOps::removeQueue(QueueImpl *queue)
{
    TRACE(CL_LOG, "removeQueue");    

    SAFE_CALL_ZK(m_zk.deleteNode(queue->getKey(), true),
                 "Could not delete key %s: %s",
                 queue->getKey().c_str(),
                 false,
                 true);
}

void
FactoryOps::removeGroup(GroupImpl *group)
{
    TRACE(CL_LOG, "removeGroup");    

    SAFE_CALL_ZK(m_zk.deleteNode(group->getKey(), true),
                 "Could not delete key %s: %s",
                 group->getKey().c_str(),
                 false,
                 true);
}

void
FactoryOps::removeNode(NodeImpl *node)
{
    TRACE(CL_LOG, "removeNode");

    SAFE_CALL_ZK(m_zk.deleteNode(node->getKey(), true),
                 "Could not delete key %s: %s",
                 node->getKey().c_str(),
                 false,
                 true);
}

void
FactoryOps::removeProcessSlot(ProcessSlotImpl *processSlot)
{
    TRACE(CL_LOG, "removeProcessSlot");

    SAFE_CALL_ZK(m_zk.deleteNode(processSlot->getKey(), true),
                 "Could not delete key %s: %s",
                 processSlot->getKey().c_str(),
                 false,
                 true);
}

void 
FactoryOps::removeNotifyableFromCacheByKey(const string &key)
{
    TRACE(CL_LOG, "removeNotifyableFromCacheByKey");

    Mutex *ntpMapLock = NULL;
    NotifyableImplMap *ntpMap = NULL;

    vector<string> components;
    NotifyableKeyManipulator::splitNotifyableKey(key, components);

    if (NotifyableKeyManipulator::isRootKey(components)) {
        throw InvalidMethodException(
            "RemoveNotifyableFromCacheByKey: Cannot remove root!");
    }
    else if (NotifyableKeyManipulator::isApplicationKey(components)) {
        ntpMapLock = getApplicationsLock();
        ntpMap = &m_apps;
    }
    else if (NotifyableKeyManipulator::isGroupKey(components)) {
        ntpMapLock = getGroupsLock();
        ntpMap = &m_groups;
    }
    else if (NotifyableKeyManipulator::isNodeKey(components)) {
        ntpMapLock = getNodesLock();
        ntpMap = &m_nodes;
    }
    else if (NotifyableKeyManipulator::isProcessSlotKey(components)) {
        ntpMapLock = getProcessSlotsLock();
        ntpMap = &m_processSlots;
    }
    else if (NotifyableKeyManipulator::isPropertyListKey(components)) {
        ntpMapLock = getPropertyListLock();
        ntpMap = &m_propLists;
    }
    else if (NotifyableKeyManipulator::isQueueKey(components)) {
        ntpMapLock = getQueueLock();
        ntpMap = &m_queues;
    }
    else if (NotifyableKeyManipulator::isDataDistributionKey(components)) {
        ntpMapLock = getDataDistributionsLock();
        ntpMap = &m_dists;
    }
    else {
        throw InvalidArgumentsException(
            string("RemoveNotifyableFromCacheByKey:: Cannot find map for ") + 
            key);
    }

    Locker l1(ntpMapLock);
    NotifyableImplMap::iterator ntpMapIt = ntpMap->find(key);
    if (ntpMapIt == ntpMap->end()) {
        throw InconsistentInternalStateException(
            string("RemoveNotifyableFromCacheByKey: Cannot find Notifyable ") +
            key + string(" in the map"));
    }

    {
        /* Can not hold the sync lock while removing! */
        Locker l2(ntpMapIt->second->getSyncLock());
        LOG_DEBUG(CL_LOG, 
                  "RemoveNotifyableFromCacheByKey: state changed to REMOVED "
                  "for Notifyable %s",
                  key.c_str());
        if (ntpMapIt->second->getState() == Notifyable::REMOVED) {
            throw InvalidMethodException(
                string("RemoveNotifyableFromCacheByKey: Tried to remove 2x ") +
                key.c_str());
        }
        
        Locker l3(getRemovedNotifyablesLock());
        set<Notifyable *>::const_iterator it = 
            getRemovedNotifyables()->find(ntpMapIt->second);
        if (it != getRemovedNotifyables()->end()) {
            throw InconsistentInternalStateException(
                string("RemoveNotifyableFromCacheByKey: Notifyable for key ") +
                ntpMapIt->second->getKey() + 
                " is already in removed notifyables" +
                " set!");
        }
        getRemovedNotifyables()->insert(ntpMapIt->second);

        ntpMapIt->second->setState(Notifyable::REMOVED);
    }

    ntpMap->erase(ntpMapIt);
}

bool
FactoryOps::isNodeConnected(const string &nodeKey, string &id, int64_t &msecs)
{
    TRACE(CL_LOG, "isNodeConnected");

    string ckey =
        nodeKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CONNECTED;
    string encodedJsonObj;
    bool exists = false;

    /* 
     * Set the event handler to fire when the node changes (or is
     * created) 
     */
    SAFE_CALLBACK_ZK(
        (exists = m_zk.nodeExists(
            ckey,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::NODE_CONNECTION_CHANGE))),
        (exists = m_zk.nodeExists(ckey)),
        CachedObjectChangeHandlers::NODE_CONNECTION_CHANGE,
        ckey,
        "Could not determine whether key %s is connected: %s",
        nodeKey.c_str(),
        false,
        true);
    if (exists) {
        SAFE_CALL_ZK((exists = m_zk.getNodeData(
                          ckey,
                          encodedJsonObj)),
                     "Could not determine whether key %s is connected: %s",
                     ckey.c_str(),
                     false,
                     true);
        if (exists) {
            JSONValue jsonValue = JSONCodec::decode(encodedJsonObj);
            JSONValue::JSONObject jsonObj = 
                jsonValue.get<JSONValue::JSONObject>();
            JSONValue::JSONObject::const_iterator jsonObjIt;
            jsonObjIt = 
                jsonObj.find(ClusterlibStrings::JSONOBJECTKEY_CONNECTEDID);
            if (jsonObjIt != jsonObj.end()) {
                id = jsonObjIt->second.get<JSONValue::JSONString>();
            }
            else {
                throw InconsistentInternalStateException(
                    string("isNodeConnected: Cannot find id in value ") + 
                    encodedJsonObj);
            }
            jsonObjIt = jsonObj.find(ClusterlibStrings::JSONOBJECTKEY_TIME);
            if (jsonObjIt != jsonObj.end()) {
                msecs = jsonObjIt->second.get<JSONValue::JSONInteger>();
            }
            else {
                throw InconsistentInternalStateException(
                    string("isNodeConnected: Cannot find time in value ") + 
                    encodedJsonObj);
            }
        }
    }

    return exists;
}
string
FactoryOps::getNodeClientState(const string &nodeKey)
{
    TRACE(CL_LOG, "getNodeClientState");

    string ckey =
        nodeKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CLIENTSTATE;
    string res;

    SAFE_CALLBACK_ZK(
        m_zk.getNodeData(
            ckey,
            res,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::NODE_CLIENT_STATE_CHANGE)),
        m_zk.getNodeData(ckey, res),
        CachedObjectChangeHandlers::NODE_CLIENT_STATE_CHANGE,
        ckey,
        "Could not read node %s client state: %s",
        nodeKey.c_str(),
        false,
        true);

    return res;
}
int32_t
FactoryOps::getNodeMasterSetState(const string &nodeKey)
{
    TRACE(CL_LOG, "getNodeMasterSetState");

    string ckey =
        nodeKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::MASTERSETSTATE;
    string res;

    SAFE_CALLBACK_ZK(
        m_zk.getNodeData(
            ckey,
            res,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::NODE_MASTER_SET_STATE_CHANGE)),
        m_zk.getNodeData(ckey, res),
        CachedObjectChangeHandlers::NODE_MASTER_SET_STATE_CHANGE,
        ckey,
        "Could not read node %s master set state: %s",
        nodeKey.c_str(),
        false,
        true);
    
    return ::atoi(res.c_str());
}

/*
 * Create a node's connected state.
 */
bool
FactoryOps::createConnected(const string &key, const string &id)
{
    TRACE(CL_LOG, "createConnected");

    string ckey =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CONNECTED;
        
    try {
        /*
         * Create the node.
         */
        JSONValue::JSONObject jsonObj;
        jsonObj[ClusterlibStrings::JSONOBJECTKEY_CONNECTEDID] = id;
        jsonObj[ClusterlibStrings::JSONOBJECTKEY_TIME] = 
            TimerService::getCurrentTimeMsecs();
        string encodedJsonObj = JSONCodec::encode(jsonObj);
        LOG_DEBUG(CL_LOG,
                  "createConnected: Key (%s) object (%s)",
                  ckey.c_str(),
                  encodedJsonObj.c_str());
        SAFE_CALL_ZK(m_zk.createNode(ckey, encodedJsonObj, ZOO_EPHEMERAL),
                     "Could not create %s CONNECTED subnode -- "
                     "already exists: %s",
                     ckey.c_str(),
                     true,
                     true);
        /*
         * Establish the watch.
         */
        SAFE_CALLBACK_ZK(
            m_zk.nodeExists(
                ckey,
                &m_zkEventAdapter,
                getCachedObjectChangeHandlers()->
                getChangeHandler(
                    CachedObjectChangeHandlers::NODE_CONNECTION_CHANGE)),
            ,
            CachedObjectChangeHandlers::NODE_CONNECTION_CHANGE,
            ckey,
            "Could not establish exists watch for znode %s: %s",
            ckey.c_str(),
            false,
            true);
        
        return true;
    } catch (zk::ZooKeeperException &e) {
        return false;
    }
}

/*
 * Actively remove a node's connected state.
 */
void
FactoryOps::removeConnected(const string &key)
{
    TRACE(CL_LOG, "removeConnected");

    string ckey =
        key +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CONNECTED;

    /*
     * Try to remove the node's connected state.
     */
    SAFE_CALL_ZK(m_zk.deleteNode(ckey, false),
                 "Could not delete key %s: %s",
                 ckey.c_str(),
                 false,
                 true);
}

Mutex *
FactoryOps::getClientsLock() 
{
    TRACE(CL_LOG, "getClientsLock");
    return &m_clLock; 
}

Mutex *
FactoryOps::getPropertyListLock() 
{
    TRACE(CL_LOG, "getPropertyListLock");
    return &m_propListsLock; 
}

Mutex *
FactoryOps::getQueueLock() 
{
    TRACE(CL_LOG, "getQueueLock");
    return &m_queuesLock; 
}

Mutex *
FactoryOps::getDataDistributionsLock() 
{ 
    TRACE(CL_LOG, "getDataDistributionsLock");
    return &m_distsLock; 
}
   
Mutex *
FactoryOps::getRootLock() 
{ 
    TRACE(CL_LOG, "getRootLock");
    return &m_rootLock; 
}

Mutex *
FactoryOps::getApplicationsLock() 
{ 
    TRACE(CL_LOG, "getApplicationsLock");
    return &m_appsLock; 
}

Mutex *
FactoryOps::getGroupsLock() 
{ 
    TRACE(CL_LOG, "getGroupsLock");
    return &m_groupsLock; 
}

Mutex *
FactoryOps::getNodesLock() 
{ 
    TRACE(CL_LOG, "getNodesLock");
    return &m_nodesLock; 
}

Mutex *
FactoryOps::getProcessSlotsLock() 
{ 
    TRACE(CL_LOG, "getProcessSlotsLock");
    return &m_processSlotsLock;
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

/*
 * Get all entity names within a collection: all applications,
 * all groups or distributions within an application, or all
 * nodes within a group.
 */
NameList
FactoryOps::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    NameList list;
    string key =
        ClusterlibStrings::ROOTNODE +
        ClusterlibStrings::CLUSTERLIB +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CLUSTERLIBVERSION +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::ROOT +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::APPLICATIONS;

    list.clear();
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            key, 
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::APPLICATIONS_CHANGE)),
        m_zk.getNodeChildren(key, list),
        CachedObjectChangeHandlers::APPLICATIONS_CHANGE,
        key,
        "Reading the value of %s failed: %s",
        key.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin(); nlIt != list.end(); ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(key.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getGroupNames(GroupImpl *group)
{
    TRACE(CL_LOG, "getGroupNames");

    if (group == NULL) {
        throw InvalidArgumentsException("NULL group in getGroupNames");
    }

    NameList list;
    string key =
        group->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::GROUPS;

    list.clear();
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            key,
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::GROUPS_CHANGE)),
        m_zk.getNodeChildren(key, list),
        CachedObjectChangeHandlers::GROUPS_CHANGE,
        key,
        "Reading the value of %s failed: %s",
        key.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin();
         nlIt != list.end();
         ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(key.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getDataDistributionNames(GroupImpl *group)
{
    TRACE(CL_LOG, "getDataDistributionNames");

    if (group == NULL) {
        throw InvalidArgumentsException(
            "NULL group in getDataDistributionNames");
    }

    NameList list;
    string key =
        group->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::DISTRIBUTIONS;

    list.clear();
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            key,
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::DATADISTRIBUTIONS_CHANGE)),
        m_zk.getNodeChildren(key, list),
        CachedObjectChangeHandlers::DATADISTRIBUTIONS_CHANGE,
        key,
        "Reading the value of %s failed: %s",
        key.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin();
         nlIt != list.end();
         ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(key.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getNodeNames(GroupImpl *group)
{
    TRACE(CL_LOG, "getNodeNames");

    if (group == NULL) {
        throw InvalidArgumentsException("NULL group in getNodeNames");
    }

    NameList list;
    string key =
        group->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::NODES;

    list.clear();
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            key,
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::NODES_CHANGE)),
        m_zk.getNodeChildren(key, list),
        CachedObjectChangeHandlers::NODES_CHANGE,
        key,
        "Reading the value of %s failed: %s",
        key.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin();
         nlIt != list.end();
         ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(key.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getProcessSlotNames(NodeImpl *node)
{
    TRACE(CL_LOG, "getNodeNames");

    if (node == NULL) {
        throw InvalidArgumentsException("NULL node in getProcessSlotNames");
    }

    NameList list;
    string key =
        node->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::PROCESSSLOTS;

    list.clear();
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            key,
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::PROCESSSLOTS_CHANGE)),
        m_zk.getNodeChildren(key, list),
        CachedObjectChangeHandlers::PROCESSSLOTS_CHANGE,
        key,
        "Reading the value of %s failed: %s",
        key.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin();
         nlIt != list.end();
         ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(key.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getPropertyListNames(NotifyableImpl *ntp)
{
    TRACE(CL_LOG, "getPropertyListNames");

    if (ntp == NULL) {
        throw InvalidArgumentsException(
            "NULL notifyables in getPropertyListNames");
    }

    NameList list;
    string key =
        ntp->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::PROPERTYLISTS;

    list.clear();
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            key,
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROPERTYLIST_VALUES_CHANGE)),
        m_zk.getNodeChildren(key, list),
        CachedObjectChangeHandlers::PROPERTYLIST_VALUES_CHANGE,
        key,
        "Reading the value of %s failed: %s",
        key.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin();
         nlIt != list.end();
         ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(key.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}
NameList
FactoryOps::getQueueNames(NotifyableImpl *ntp)
{
    TRACE(CL_LOG, "getQueueNames");

    if (ntp == NULL) {
        throw InvalidArgumentsException(
            "NULL notifyables in getQueueNames");
    }

    NameList list;
    string key =
        ntp->getKey() +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::QUEUES;

    list.clear();
    SAFE_CALLBACK_ZK(
        m_zk.getNodeChildren(
            key,
            list,
            &m_zkEventAdapter,
            getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::QUEUE_CHILD_CHANGE)),
        m_zk.getNodeChildren(key, list),
        CachedObjectChangeHandlers::QUEUE_CHILD_CHANGE,
        key,
        "Reading the value of %s failed: %s",
        key.c_str(),
        true,
        true);
    
    for (NameList::iterator nlIt = list.begin();
         nlIt != list.end();
         ++nlIt) {
        /*
         * Remove the key prefix
         */
        *nlIt = nlIt->substr(key.length() + 
                             ClusterlibStrings::KEYSEPARATOR.length());
    }

    return list;
}

NotifyableList
FactoryOps::getChildren(Notifyable *ntp)
{
    /*
     * NotifyableImpl * could be any subclass, so start with the
     * simplest and try the most complicated.  If new subclasses of
     * NotifyableImpl are added to clusterlib, they will need to also
     * be checked for here.
     */
    NotifyableList ntList;
    
    PropertyList *propList = dynamic_cast<PropertyList *>(ntp);
    if (propList != NULL) {
        LOG_DEBUG(CL_LOG, 
                  "getChildren: %s is a PropertyList", 
                  ntp->getKey().c_str());
        return ntList;
    }
  
    /*
     * Any other NotifyableImpl subclass (except Root) can have a
     * PropertyList child
     */
    try {
        propList = ntp->getPropertyList();
        if (propList != NULL) {
            LOG_DEBUG(CL_LOG, 
                      "getChildren: found PropertyList %s for %s", 
                      propList->getKey().c_str(),
                      ntp->getKey().c_str());
            ntList.push_back(propList);
        }
    } 
    catch (InvalidMethodException &e) {
        /*
         * If this is not a root object then throw an exception.
         * Otherwise, we expect Root to get an exception here.
         */
        if (dynamic_cast<Root *>(ntp) == NULL) {
            throw InvalidMethodException(
                string("getChildren: getPropertyList failed: ") + e.what());
        }
        LOG_DEBUG(CL_LOG, "getChildren: Root doesn't have PropertyList");
    }

    DataDistribution *dist = dynamic_cast<DataDistribution *>(ntp);
    if (dist != NULL) {
        LOG_DEBUG(CL_LOG, 
                  "getChildren: %s is a DataDistribution", 
                  ntp->getKey().c_str());
        return ntList;
    }
  
    Queue *queue = dynamic_cast<Queue *>(ntp);
    if (queue != NULL) {
        LOG_DEBUG(CL_LOG, 
                  "getChildren: %s is a Queue", 
                  ntp->getKey().c_str());
        return ntList;
    }
  
    ProcessSlot *processSlot = dynamic_cast<ProcessSlot *>(ntp);
    if (processSlot != NULL) {
        LOG_DEBUG(CL_LOG, "getChildren: %s is a ProcessSlot", 
                  ntp->getKey().c_str());
        return ntList;
    }

    NameList::iterator nameListIt;
    Notifyable *tempNtp = NULL;
    Node *node = dynamic_cast<Node *>(ntp);
    if (node != NULL) {
        LOG_DEBUG(CL_LOG, "getChildren: %s is a Node", ntp->getKey().c_str());
        NameList nameList = node->getQueueNames();
        for (nameListIt = nameList.begin();
             nameListIt != nameList.end(); 
             nameListIt++) {
            tempNtp = node->getQueue(*nameListIt);
            if (tempNtp == NULL) {
                LOG_WARN(CL_LOG, 
                         "getChildren: Shouldn't get NULL queue for %s"
                         " if I have the lock",
                         nameListIt->c_str());
            }
            else {
                LOG_DEBUG(CL_LOG, 
                          "getChildren: found Queue %s for %s", 
                          tempNtp->getKey().c_str(),
                          ntp->getKey().c_str());
                ntList.push_back(tempNtp);
            }
        }

        nameList = node->getProcessSlotNames();
        for (nameListIt = nameList.begin(); 
             nameListIt != nameList.end(); 
             nameListIt++) {
            tempNtp = node->getProcessSlot(*nameListIt);
            if (tempNtp == NULL) {
                LOG_WARN(CL_LOG, 
                         "getChildren: Shouldn't get NULL process slot for %s"
                         " if I have the lock",
                         nameListIt->c_str());
            }
            else {
                LOG_DEBUG(CL_LOG, 
                          "getChildren: found Process Slot %s for %s", 
                          tempNtp->getKey().c_str(),
                          ntp->getKey().c_str());
                ntList.push_back(tempNtp);
            }
        }

        return ntList;
    }

    Group *group = dynamic_cast<Group *>(ntp);
    if (group != NULL) {
        LOG_DEBUG(CL_LOG, "getChildren: %s is a Group", ntp->getKey().c_str());
        NameList nameList = group->getNodeNames();
        for (nameListIt = nameList.begin(); 
             nameListIt != nameList.end(); 
             nameListIt++) {
            tempNtp = group->getNode(*nameListIt);
            if (tempNtp == NULL) {
                LOG_WARN(CL_LOG, 
                         "getChildren: Shouldn't get NULL node for %s"
                         " if I have the lock",
                         nameListIt->c_str());
            }
            else {
                LOG_DEBUG(CL_LOG, 
                          "getChildren: found Node %s for %s", 
                          tempNtp->getKey().c_str(),
                          ntp->getKey().c_str());
                ntList.push_back(tempNtp);
            }
        }

        nameList = group->getGroupNames();
        for (nameListIt = nameList.begin(); 
             nameListIt != nameList.end(); 
             nameListIt++) {
            tempNtp = group->getGroup(*nameListIt);
            if (tempNtp == NULL) {
                LOG_WARN(CL_LOG, 
                         "getChildren: Shouldn't get NULL group for %s"
                         " if I have the lock",
                         nameListIt->c_str());
            }
            else {
                LOG_DEBUG(CL_LOG, 
                          "getChildren: found Group %s for %s", 
                          tempNtp->getKey().c_str(),
                          ntp->getKey().c_str());
                ntList.push_back(tempNtp);
            }
        }

        nameList = group->getDataDistributionNames();
        for (nameListIt = nameList.begin(); 
             nameListIt != nameList.end(); 
             nameListIt++) {
            tempNtp = group->getDataDistribution(*nameListIt);
            if (tempNtp == NULL) {
                LOG_WARN(CL_LOG, 
                         "getChildren: Shouldn't get NULL data distribution "
                         "for %s if I have the lock",
                         nameListIt->c_str());
            }
            else {
                LOG_DEBUG(CL_LOG, 
                          "getChildren: found DataDistribution %s for %s", 
                          tempNtp->getKey().c_str(),
                          ntp->getKey().c_str());
                ntList.push_back(tempNtp);
            }
        }

        nameList = group->getQueueNames();
        for (nameListIt = nameList.begin();
             nameListIt != nameList.end(); 
             nameListIt++) {
            tempNtp = group->getQueue(*nameListIt);
            if (tempNtp == NULL) {
                LOG_WARN(CL_LOG, 
                         "getChildren: Shouldn't get NULL queue for %s"
                         " if I have the lock",
                         nameListIt->c_str());
            }
            else {
                LOG_DEBUG(CL_LOG, 
                          "getChildren: found Queue %s for %s", 
                          tempNtp->getKey().c_str(),
                          ntp->getKey().c_str());
                ntList.push_back(tempNtp);
            }
        }

        return ntList;
    }

    Root *root = dynamic_cast<Root *>(ntp);
    if (root != NULL) {
        LOG_DEBUG(CL_LOG, "getChildren: %s is a Root", ntp->getKey().c_str());
        NameList nameList = root->getApplicationNames();
        for (nameListIt = nameList.begin(); 
             nameListIt != nameList.end(); 
             nameListIt++) {
            tempNtp = root->getApplication(*nameListIt);
            if (tempNtp == NULL) {
                LOG_WARN(CL_LOG, 
                         "getChildren: Shouldn't get NULL application for %s"
                         " if I have the lock",
                         nameListIt->c_str());
            }
            else {
                LOG_DEBUG(CL_LOG, 
                          "getChildren: found Application %s for %s", 
                          tempNtp->getKey().c_str(),
                          ntp->getKey().c_str());
                ntList.push_back(tempNtp);
            }
        }

        return ntList;
    }

    throw InconsistentInternalStateException(
        "getChildren: Unknown NotifyableImpl subclass");
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

/*
 * Update the cached representation of a clusterlib repository object and
 * generate the prototypical user event payload to send to registered
 * clients.
 */
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
             "updateCachedObject: (0x%x, 0x%x, %s, %s)",
             (int) fehp,
             (int) ep,
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
            notifyablePath = NotifyableKeyManipulator::getNotifyableKeyFromKey(
                    ep->getPath());
            ntp = getNotifyableFromKey(notifyablePath); 
            
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
        if (!NotifyableKeyManipulator::isRootKey(ntp->getKey())) {
            ntp->releaseRef();
        }
    }

    LOG_DEBUG(CL_LOG, 
              "updateCachedObject: Returning payload for event %d for key %s",
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
                CachedObjectChangeHandlers::NOTIFYABLE_STATE_CHANGE)),
        ,
        CachedObjectChangeHandlers::NOTIFYABLE_STATE_CHANGE,
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

