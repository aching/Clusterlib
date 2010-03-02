/*
 * nodeimpl.cc --
 *
 * Implementation of the NodeImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace json;

namespace clusterlib
{

#define VAL(str) #str
    
// execute the given code or die if it times out
#define LIVE_OR_DIE(code, timeout)  \
{   \
    LOG_DEBUG(CL_LOG, "Setting up a bomb to go off in %d ms if '%s' deadlocks...", timeout, VAL(code));  \
    TimerId timerId = m_timer.scheduleAfter(timeout, VAL(code));   \
    try {   \
        code;   \
        m_timer.cancelAlarm(timerId); \
    } \
    catch (std::exception &e) {   \
        m_timer.cancelAlarm(timerId); \
        LOG_ERROR(CL_LOG, \
                  "An exception while executing '%s': %s",  \
                  VAL(code), e.what()); \
    } \
    catch (...) { \
        m_timer.cancelAlarm(timerId); \
        LOG_ERROR(CL_LOG, \
                  "Unable to execute '%s', unknown exception", VAL(code));  \
    } \
    LOG_DEBUG(CL_LOG, "...disarming the bomb");  \
}

bool
NodeImpl::isConnected(string *id, int64_t *msecs) 
{
    TRACE(CL_LOG, "isConnected");

    Locker l(getStateMutex());
    LOG_DEBUG(CL_LOG, "isConnected: id=%s,msecs=%Ld", m_connectedId.c_str(), m_connectionTime);
    if (m_connected) {
        if (id) {
            *id = m_connectedId;
        }
        if (msecs) {
            *msecs = m_connectionTime;
        }
    }
    return m_connected; 
}

bool
NodeImpl::initializeConnection(bool force)
{
    TRACE(CL_LOG, "initializeConnection");

    Locker l(getStateMutex());
    if (force) {
        getOps()->removeConnected(getKey());
    }
    bool ret = getOps()->createConnected(getKey(), 
                                         Factory::getHostnamePidTid());
    if (ret) {
        m_connected = true;
        m_connectedId = Factory::getHostnamePidTid();
    }

    return ret;
}

/*****************************************************************************/
/* Health management.                                                        */
/*****************************************************************************/

void
NodeImpl::registerHealthChecker(HealthChecker *healthChecker)
{
    TRACE(CL_LOG, "registerHealthChecker" );

    /*
     * Check to make sure that this client initialized the connection.
     */
    if (!m_connected) {
        throw InvalidArgumentsException(
            "registerHealthChecker: Must be connected to this node prior "
            "to registering a health checker!");
    }
    if (m_connectedId != Factory::getHostnamePidTid()) {
            throw AlreadyConnectedException(
                getKey() +
                ": registerHealthChecker: Node already connected for thread " +
                m_connectedId + " cannot connect with " +
                Factory::getHostnamePidTid());
    }

    if (healthChecker == NULL) {
        throw InvalidArgumentsException(
            "registerHealthChecker: Cannot use a NULL healthChecker");
    }

    if (healthChecker->getMsecsPerCheckIfHealthy() <= 0) {
        throw InvalidArgumentsException(
            "registerHealthChecker: Cannot have a healthy "
            "msec check cycle <= 0");
    }

    if (healthChecker->getMsecsPerCheckIfUnhealthy() <= 0) {
        throw InvalidArgumentsException(
            "registerHealthChecker: Cannot have a unhealthy "
            "msec check cycle <= 0");
    }

    Locker l(getHealthMutex());
    if (mp_healthChecker != NULL) {
        LOG_ERROR(CL_LOG,
                  "registerHealthChecker: Already registered healthChecker "
                  "0x%x",
                  reinterpret_cast<uint32_t>(healthChecker));
        throw InvalidMethodException(
            "registerHealthChecker: Already registered a health checker");
    }

    m_terminateDoHealthChecks = false;
    mp_healthChecker = healthChecker;

    /*
     * Start the health checker thread
     */
    m_doHealthChecksThread.Create(*this, &NodeImpl::doHealthChecks, NULL);
}

void
NodeImpl::unregisterHealthChecker()
{
    TRACE(CL_LOG, "unregisterHealthChecker" );

    getOps()->removeConnected(getKey());

    {
        Locker l(getHealthMutex());
        if (mp_healthChecker == NULL) {
            LOG_ERROR(CL_LOG,
                      "unregisterHealthChecker: No registered healthChecker ");
            
            throw InvalidMethodException(
                "unregisterHealthChecker: No registered health checker");
        }
        m_terminateDoHealthChecks = true;
        getHealthCond()->signal();
    }

    m_doHealthChecksThread.Join();

    Locker l(getHealthMutex());
    mp_healthChecker = NULL;
}

void
NodeImpl::setUseProcessSlots(bool use)
{
    TRACE(CL_LOG, "setUseProcessSlots");

    acquireLock();
    string processSlotsUsageKey = 
        NotifyableKeyManipulator::createProcessSlotsUsageKey(getKey());

    JSONValue::JSONBoolean jsonBool = use;
    string encodedJsonValue = JSONCodec::encode(jsonBool);
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotsUsageKey,
                     encodedJsonValue),
                 "Getting of %s failed: %s",
                 processSlotsUsageKey.c_str(),
                 true,
                 false);
    releaseLock();
}

bool
NodeImpl::getUseProcessSlots()
{
    TRACE(CL_LOG, "getUseProcessSlots");

    string processSlotsUsageKey = 
        NotifyableKeyManipulator::createProcessSlotsUsageKey(getKey());

    string encodedJsonValue;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotsUsageKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOTS_USAGE_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotsUsageKey,
            encodedJsonValue),
        CachedObjectChangeHandlers::PROCESSSLOTS_USAGE_CHANGE,
        processSlotsUsageKey,
        "Reading the value of %s failed: %s",
        processSlotsUsageKey.c_str(),
        true,
        true);
    if (encodedJsonValue.empty()) {
        return false;
    }
    else {        
        return 
            JSONCodec::decode(encodedJsonValue).get<JSONValue::JSONBoolean>();
    }
}

NameList 
NodeImpl::getProcessSlotNames()
{
    TRACE(CL_LOG, "getProcessSlotNames");

    throwIfRemoved();

    return getOps()->getProcessSlotNames(this);
}

ProcessSlot *
NodeImpl::getProcessSlot(const string &name, 
                         bool create)
{
    TRACE(CL_LOG, "getProcessSlot");

    throwIfRemoved();

    return getOps()->getProcessSlot(name, this, create);
}

int32_t
NodeImpl::getMaxProcessSlots()
{
    TRACE(CL_LOG, "getMaxProcessSlots");
    string processSlotsMaxKey = 
        NotifyableKeyManipulator::createProcessSlotsMaxKey(getKey());

    string encodedJsonValue;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeData(
                      processSlotsMaxKey,
                      encodedJsonValue),
                 "Getting of %s failed: %s",
                 processSlotsMaxKey.c_str(),
                 true,
                 false);
    if (encodedJsonValue.empty()) {
        return -1;
    }
    else {        
        return static_cast<int32_t>(JSONCodec::decode(encodedJsonValue).
                                    get<JSONValue::JSONInteger>());
    }
}

void
NodeImpl::setMaxProcessSlots(int32_t maxProcessSlots)
{
    TRACE(CL_LOG, "setMaxProcessSlots");

    acquireLock();
    string processSlotsMaxKey = 
        NotifyableKeyManipulator::createProcessSlotsMaxKey(getKey());

    JSONValue::JSONInteger jsonInt = static_cast<int64_t>(maxProcessSlots);
    string encodedJsonValue = JSONCodec::encode(jsonInt);
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotsMaxKey,
                     encodedJsonValue),
                 "Getting of %s failed: %s",
                 processSlotsMaxKey.c_str(),
                 true,
                 false);
    releaseLock();
}

NodeImpl::~NodeImpl()
{
    /*
     * Shut down health checking if the user forgot to do this.
     */
    bool stopHealthChecker = false;
    {
        Locker l(getHealthMutex());
        if (mp_healthChecker != NULL) {
            stopHealthChecker = true;
        }
    }

    if (stopHealthChecker == true) {
        unregisterHealthChecker();
    }
}

bool
NodeImpl::isHealthy() 
{
    TRACE(CL_LOG, "isHealthy");

    Locker l(getStateMutex());
    LOG_DEBUG(CL_LOG, 
              "isHealthy: Notifyable = (%s), clientState = (%s)",
              getKey().c_str(),
              m_clientState.c_str());

    return (m_clientState == ClusterlibStrings::HEALTHY) ? true : false;
}

void
NodeImpl::doHealthChecks(void *param)
{
    TRACE(CL_LOG, "doHealthChecks");
    
    /*
     * Initialize these to the starting values.
     */
    int64_t lastPeriod = mp_healthChecker->getMsecsPerCheckIfHealthy();
    int64_t curPeriod = mp_healthChecker->getMsecsPerCheckIfUnhealthy();

    if ((lastPeriod <= 0) || (curPeriod <= 0)) {
        throw InvalidMethodException(
            "doHealthChecks: Impossible <= 0 healthy or unhealthy period");
    }

    LOG_DEBUG(CL_LOG,
              "Starting thread with NodeImpl::doHealthChecks(), "
              "this: 0x%x, thread: 0x%x",
              (int32_t) this,
              (uint32_t) pthread_self());

    while (!m_terminateDoHealthChecks) {
        LOG_DEBUG(CL_LOG, "About to check health");

        HealthReport report(HealthReport::HS_UNHEALTHY);
        try {
#if 0 // AC - Events aren't ready yet
            //schedule an abort in 10 mins to prevent from a deadlock
            LIVE_OR_DIE(report = mp_healthChecker->checkHealth(), 
                        mp_healthChecker->getMsecsAllowedPerHealthCheck());
#else
            report = mp_healthChecker->checkHealth();		
#endif
            LOG_DEBUG(CL_LOG,
                      "doHealthChecks: Health report - state: %d, "
                      "description: %s",
                      report.getHealthState(), 
                      report.getStateDescription().c_str());
        } 
        catch (Exception &e) {
            LOG_ERROR(CL_LOG, "Caught exception: %s", e.what());
            report = HealthReport(HealthReport::HS_UNHEALTHY, e.what());
        } 
        catch (...) {
            LOG_ERROR(CL_LOG,
                      "Caught unknown exception, "
                      "assuming unhealthy state");
            report = HealthReport(HealthReport::HS_UNHEALTHY);
        }
            
        /*
         * Set the health in the repository
         */
        string value =
            (report.getHealthState() == HealthReport::HS_HEALTHY)
            ? ClusterlibStrings::HEALTHY
            : ClusterlibStrings::UNHEALTHY; 
        getOps()->updateNodeClientState(getKey(), value);
        getOps()->updateNodeClientStateDesc(getKey(), 
                                            report.getStateDescription());
        
        /*
         * Decide whether to use the CLUSTER_HEARTBEAT_HEALTHY or
         * CLUSTER_HEARTBEAT_UNHEALTHY heartbeat frequency.
         */
        lastPeriod = curPeriod;
        if (report.getHealthState() == HealthReport::HS_HEALTHY) {
            curPeriod = mp_healthChecker->getMsecsPerCheckIfHealthy();
        }
        else {
            curPeriod = mp_healthChecker->getMsecsPerCheckIfUnhealthy();
        }

        getHealthMutex()->acquire();
        LOG_DEBUG(CL_LOG,
                  "About to wait %lld msec before next health check...",
                  curPeriod);
        getHealthCond()->waitUsecs(*getHealthMutex(), 
                                   static_cast<uint64_t>(curPeriod) * 1000);
        LOG_DEBUG(CL_LOG, "...awoken!");

        getHealthMutex()->release();
    }
    
    LOG_DEBUG(CL_LOG,
              "Ending thread with NodeImpl::doHealthChecks(): "
              "this: 0x%x, thread: 0x%x",
              (int32_t) this,
              (uint32_t) pthread_self());
}

void 
NodeImpl::setConnectedAndTime(bool nc, const string &id, int64_t t)
{ 
    Locker l1(getStateMutex());

    LOG_DEBUG(CL_LOG,
              "setConnectedAndTime: connected (%s), id (%s), time (%lld)",
              (nc ? "true" : "false"),
              id.c_str(),
              t);

    m_connected = nc; 
    m_connectedId = id;
    m_connectionTime = t; 
}


/*
 * Initialize the cached representation of this node.
 */
void
NodeImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Ensure that the cache contains all the information
     * about this node, and that all watches are established.
     */
    string connectedId;
    int64_t connectionTime;
        
    Locker l(getStateMutex());
    bool connected = getOps()->isNodeConnected(
        getKey(), connectedId, connectionTime);
    if (connected) {
        setConnectedAndTime(connected, connectedId, connectionTime);
    }
    m_clientState = getOps()->getNodeClientState(
        getKey());
    m_masterSetState = getOps()->getNodeMasterSetState(
        getKey());
}

void
NodeImpl::removeRepositoryEntries()
{
    getOps()->removeNode(this);
}

};	/* End of 'namespace clusterlib' */
