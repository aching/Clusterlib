/*
 * serverimpl.cc --
 *
 * Implementation of the Server class.
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
        } catch (std::exception &e) {   \
            m_timer.cancelAlarm(timerId); \
            LOG_ERROR(CL_LOG, \
                      "An exception while executing '%s': %s",  \
                      VAL(code), e.what()); \
        } catch (...) { \
            m_timer.cancelAlarm(timerId); \
            LOG_ERROR(CL_LOG, \
                      "Unable to execute '%s', unknown exception", VAL(code));  \
        }   \
        LOG_DEBUG(CL_LOG, "...disarming the bomb");  \
    }


/*
 * Constructor.
 */
ServerImpl::ServerImpl(FactoryOps *fp,
                       Group *group,
                       const string &nodeName,
                       HealthChecker *healthChecker,
                       ServerFlags flags)
    : ClientImpl(fp),
      mp_f(fp),
      m_checkFrequencyHealthy(1000*60), // Default healthy check of a minute
      m_checkFrequencyUnhealthy(1000*15), // Default unhealthy check of 15 secs
      m_heartBeatMultiple(2.0),
      m_heartBeatCheckPeriod(1000*60),
      m_healthCheckerTerminating(false),
      m_healthCheckingEnabled(true),
      mp_healthChecker(healthChecker),
      m_flags(flags),
      m_myBid(-1)
{
    mp_node = mp_f->getNode(nodeName,
                            dynamic_cast<GroupImpl *>(group),
                            (m_flags & SF_MANAGED) == SF_MANAGED,
                            (m_flags & SF_CREATEREG) == SF_CREATEREG);
    if (mp_node == NULL) {
        throw InvalidArgumentsException(
            "Could not find or create node!");
    }

    /*
     * Start the health checker thread, even if the caller did not
     * register a health checker during construction. If mp_healthChecker
     * is NULL then the health check is just skipped...
     */
    m_checkerThread.Create(*this, &ServerImpl::checkHealth);
}

ServerImpl::~ServerImpl()
{
    m_mutex.acquire();
    m_healthCheckerTerminating = true;
    LOG_INFO(CL_LOG, "Sending signal to stop health checking...");
    m_cond.signal();
    m_mutex.release();
    m_checkerThread.Join();
}

/*****************************************************************************/
/* Health management.                                                        */
/*****************************************************************************/

void
ServerImpl::registerHealthChecker(HealthChecker *healthChecker)
{
    TRACE( CL_LOG, "registerHealthChecker" );

    /*
     * DCL pattern.
     */
    if (healthChecker != mp_healthChecker) {
        m_mutex.acquire();
        if (healthChecker != mp_healthChecker) {
            mp_healthChecker = healthChecker;
            m_cond.signal();
        }
        m_mutex.release();
    }
}

/*
 * Return the various heartbeat periods and multiples.
 */
int
ServerImpl::getHeartBeatPeriod()
{
    return m_checkFrequencyHealthy;
}

int
ServerImpl::getUnhealthyHeartBeatPeriod()
{
    return m_checkFrequencyUnhealthy;
}

double
ServerImpl::getHeartBeatMultiple() 
{
    return m_heartBeatMultiple;
}

int
ServerImpl::getHeartBeatCheckPeriod()
{
    return m_heartBeatCheckPeriod;
}

/*
 * Check the health. But don't actually do the health check if the period
 * is less than 0. In that case just wait for the length of the last valid
 * period and then check the setting of m_checkFrequency again.
 */
void
ServerImpl::checkHealth(void *param)
{
    TRACE(CL_LOG, "checkHealth");
    
    int32_t lastPeriod = m_checkFrequencyHealthy;   /* Initially.   */
    int32_t curPeriod = m_checkFrequencyHealthy;    /* Initially.   */

    LOG_DEBUG(CL_LOG,
              "Starting thread with ServerImpl::checkHealth(), "
              "this: 0x%x, thread: 0x%x",
              (int32_t) this,
              (uint32_t) pthread_self());

    while (!m_healthCheckerTerminating) {
        LOG_DEBUG(CL_LOG, "About to check health");

        /*
         * First of all check the health, if health checking is turned on.
         */
        HealthReport report(HealthReport::HS_UNHEALTHY);
        m_mutex.acquire();
        if ((mp_healthChecker != NULL) && 
            (curPeriod > 0) &&
            m_healthCheckingEnabled) 
        {
            try {
#if 0 // AC - Events aren't ready yet
                //schedule an abort in 10 mins to prevent from a deadlock
                LIVE_OR_DIE(report = mp_healthChecker->checkHealth(), 10 * 60 * 1000);
#else
		report = mp_healthChecker->checkHealth();		
#endif
                LOG_DEBUG(CL_LOG,
                          "Health report - state: %d, description: %s",
                          report.getHealthState(), report.getStateDescription().c_str());
            } catch (std::exception &e) {
                LOG_ERROR(CL_LOG, "Caught exception: %s", e.what());
                report = HealthReport(HealthReport::HS_UNHEALTHY, e.what());
            } catch (...) {
                LOG_ERROR(CL_LOG,
                          "Caught unknown exception, "
                          "assuming unhealthy state");
                report = HealthReport(HealthReport::HS_UNHEALTHY);
            }
            //check if m_healthCheckingEnabled is still true
            if (m_healthCheckingEnabled) {
#if 0 // AC - Events aren't ready yet
                //schedule an abort in 2 mins to prevent from a deadlock
                LIVE_OR_DIE(setHealthy( report ), 2 * 60 * 1000);
#else
		setHealthy(report);
#endif
            }
            else {
                LOG_WARN(CL_LOG,
                         "m_healthCheckingEnabled has changed while "
                         "checking the health. Ignoring the last report");
            }
        }

        /*
         * Decide whether to use the CLUSTER_HEARTBEAT_HEALTHY or
         * CLUSTER_HEARTBEAT_UNHEALTHY heartbeat frequency.
         */
        if (report.getHealthState() == HealthReport::HS_HEALTHY) {
            curPeriod = m_checkFrequencyHealthy;
        }
        else {
            curPeriod = m_checkFrequencyUnhealthy;
        }

        /*
         * We don't care whether wait times out or not.
         */
        int32_t waitTime;
        if (curPeriod > 0) {
            lastPeriod = curPeriod;
            waitTime = curPeriod;
        }
        else {
            waitTime = lastPeriod;
        }
        LOG_DEBUG(CL_LOG,
                  "About to wait %d msec before next health check...",
                  waitTime);
        m_cond.wait(m_mutex, waitTime);
        LOG_DEBUG(CL_LOG, "...awoken!");
        
        m_mutex.release();
    }
    
    LOG_DEBUG(CL_LOG,
              "Ending thread with ServerImpl::checkHealth(): "
              "this: 0x%x, thread: 0x%x",
              (int32_t) this,
              (uint32_t) pthread_self());
}

void
ServerImpl::enableHealthChecking(bool enabled)
{
    TRACE(CL_LOG, "enableHealthChecking");

    LOG_INFO(CL_LOG, 
             "enableHealthChecking - enabled: %s",
             enabled ? "true" : "false");
    
    m_mutex.acquire();
    if (enabled != m_healthCheckingEnabled) {
        m_healthCheckingEnabled = enabled;   
        m_cond.signal();
    }
    m_mutex.release();
}

void
ServerImpl::setHealthy(const HealthReport &report) 
{
    TRACE(CL_LOG, "setHealthy");

    string value =
        (report.getHealthState() == HealthReport::HS_HEALTHY)
        ? ClusterlibStrings::HEALTHY
        : ClusterlibStrings::UNHEALTHY;
    string key = mp_node->getKey();
 
    mp_f->updateNodeClientState(key, value);
    mp_f->updateNodeClientStateDesc(key, report.getStateDescription());
}

/**********************************************************************/
/* Leadership protocol.                                               */
/**********************************************************************/

/*
 * Attempt to become the leader.
 */
bool
ServerImpl::tryToBecomeLeader()
{
    TRACE(CL_LOG, "tryToBecomeLeader");

    /*
     * Initialization.
     */
    if (m_myBid == -1) {
        m_myBid = mp_f->placeBid(mp_node, this);
    }
    LOG_INFO(CL_LOG, "My leader bid: %lld", m_myBid);

    /*
     * First check if I already that I am or I am not the leader.
     */
    if (amITheLeader()) {
        return true;
    }
    if (mp_f->isLeaderKnown(mp_node)) {
        return false;
    }

    /*
     * If my bid is the lowest, then I'm the leader. Find out.
     */
    return mp_f->tryToBecomeLeader(mp_node, m_myBid);
}

/*
 * Is this server the leader of its group?
 */
bool
ServerImpl::amITheLeader()
{
    TRACE(CL_LOG, "amITheLeader");

    return mp_node->isLeader();
}

/*
 * Give up leadership -- no-op if I'm not the leader of my
 * group, else some other server becomes the leader.
 */
void
ServerImpl::giveUpLeadership()
{
    TRACE(CL_LOG, "giveUpLeadership");

    int64_t myBid = m_myBid;

    /*
     * Revert to not having a leadership bid.
     */
    m_myBid = -1;

    /*
     * If the previous bid was -1, I never bid, so I
     * cannot be the leader.
     */
    if (myBid == -1) {
        return;
    }

    mp_f->giveUpLeadership(mp_node, myBid);
}

};	/* End of 'namespace clusterlib' */
