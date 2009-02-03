/*
 * clusterserver.cc --
 *
 * Implementation of the Server class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlib.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{

#define VAL(str) #str
    
// execute the given code or die if it times out
#define LIVE_OR_DIE(code, timeout)  \
    {   \
        LOG_DEBUG( CL_LOG, "Setting up a bomb to go off in %d ms if '%s' deadlocks...", timeout, VAL(code) );  \
        TimerId timerId = m_timer.scheduleAfter( timeout, VAL(code) );   \
        try {   \
            code;   \
            m_timer.cancelAlarm( timerId ); \
        } catch (std::exception &e) {   \
            m_timer.cancelAlarm( timerId ); \
            LOG_ERROR( CL_LOG, \
                        "An exception while executing '%s': %s",  \
                        VAL(code), e.what() ); \
        } catch (...) { \
            m_timer.cancelAlarm( timerId ); \
            LOG_ERROR( CL_LOG, \
                        "Unable to execute '%s', unknown exception", VAL(code) );  \
        }   \
        LOG_DEBUG( CL_LOG, "...disarming the bomb" );  \
    }


/*
 * Constructor.
 */
Server::Server(FactoryOps *f,
               const string &appName,
               const string &grpName,
               const string &nodeName,
               HealthChecker *healthChecker,
               ServerFlags flags)
    : Client(f),
      mp_f(f),
      m_appName(appName),
      m_grpName(grpName),
      m_nodeName(nodeName),
      m_checkFrequencyHealthy(1000*60), // Default healthy check of a minute
      m_checkFrequencyUnhealthy(1000*15), // Default unhealthy check of 15 secs
      m_heartBeatMultiple(2.0),
      m_heartBeatCheckPeriod(1000*60),
      m_healthCheckerTerminating(false),
      m_healthCheckingEnabled(true),
      mp_healthChecker(healthChecker),
      m_flags(flags)
{
    mp_node = mp_f->getNode(m_appName,
                            m_grpName,
                            m_nodeName,
                            (m_flags & SF_MANAGED) == SF_MANAGED,
                            (m_flags & SF_CREATEREG) == SF_CREATEREG);
    if (mp_node == NULL) {
        throw ClusterException("Could not find or create node!");
    }

    if (healthChecker != NULL) {
	m_checkerThread.Create(*this, &Server::checkHealth);
    }
}

Server::~Server()
{
    m_mutex.Acquire();
    m_healthCheckerTerminating = true;
    LOG_INFO(CL_LOG, "Sending signal to stop health checking...");
    m_cond.Signal();
    m_mutex.Release();
    m_checkerThread.Join();
}

/*****************************************************************************/
/* Health management.                                                        */
/*****************************************************************************/

void
Server::registerHealthChecker(HealthChecker *healthChecker)
{
    TRACE( CL_LOG, "registerHealthChecker" );

    /*
     * DCL pattern.
     */
    if (healthChecker != mp_healthChecker) {
        m_mutex.Acquire();
        if (healthChecker != mp_healthChecker) {
            mp_healthChecker = healthChecker;
            m_cond.Signal();
        }
        m_mutex.Release();
    }
}

/*
 * Return the various heartbeat periods and multiples.
 */
int
Server::getHeartBeatPeriod()
{
    return m_checkFrequencyHealthy;
}

int
Server::getUnhealthyHeartBeatPeriod()
{
    return m_checkFrequencyUnhealthy;
}

double
Server::getHeartBeatMultiple() 
{
    return m_heartBeatMultiple;
}

int
Server::getHeartBeatCheckPeriod()
{
    return m_heartBeatCheckPeriod;
}

/*
 * Check the health. But don't actually do the health check if the period
 * is less than 0. In that case just wait for the length of the last valid
 * period and then check the setting of m_checkFrequency again.
 */
void
Server::checkHealth()
{
    TRACE( CL_LOG, "checkHealth" );
    
    int lastPeriod = m_checkFrequencyHealthy;   /* Initially.   */
    int curPeriod = m_checkFrequencyHealthy;    /* Initially.   */

    while (!m_healthCheckerTerminating) {
        LOG_DEBUG( CL_LOG, "About to check health" );

        /*
         * First of all check the health, if health checking is turned on.
         */
        HealthReport report(HealthReport::HS_UNHEALTHY);
        m_mutex.Acquire();
        if ((mp_healthChecker != NULL) && 
            (curPeriod > 0) &&
            m_healthCheckingEnabled) 
        {
            try {
                //schedule an abort in 10 mins to prevent from a deadlock
#if 0 // AC - Events aren't ready yet
                LIVE_OR_DIE( report = mp_healthChecker->checkHealth(), 10 * 60 * 1000 );
#else
		report = mp_healthChecker->checkHealth();		
#endif
                LOG_DEBUG( CL_LOG, "Health report - state: %d, description: %s",
                           report.getHealthState(), report.getStateDescription().c_str() );
            } catch (std::exception &e) {
                LOG_ERROR( CL_LOG, "Caught exception: %s", e.what() );
                report = HealthReport( HealthReport::HS_UNHEALTHY, e.what() );
            } catch (...) {
                LOG_ERROR(
                    CL_LOG,
                    "Caught unknown exception, assuming unhealthy state" );
                report = HealthReport( HealthReport::HS_UNHEALTHY );
            }
            //check if m_healthCheckingEnabled is still true
            if (m_healthCheckingEnabled) {
#if 0 // AC - No statistics I'm aware of yet
                mp_clusterStats->incrHealthReports( 
                    report.getHealthState() == HealthReport::HS_HEALTHY, 
                    1 
                );
#endif
                //schedule an abort in 2 mins to prevent from a deadlock
#if 0 // AC - Events aren't ready yet
                LIVE_OR_DIE( setHealthy( report ), 2 * 60 * 1000 );
#else
		setHealthy(report);
#endif
            } else {
                LOG_WARN( CL_LOG, "m_healthCheckingEnabled has changed while checking the health. Ignoring the last report" );
            }
        }

        /*
         * Decide whether to use the CLUSTER_HEARTBEAT_HEALTHY or
         * CLUSTER_HEARTBEAT_UNHEALTHY heartbeat frequency.
         */
        if (report.getHealthState() == HealthReport::HS_HEALTHY) {
            curPeriod = m_checkFrequencyHealthy;
        } else {
            curPeriod = m_checkFrequencyUnhealthy;
        }

        /*
         * We don't care whether wait times out or not.
         */
        int waitTime;
        if (curPeriod > 0) {
            lastPeriod = curPeriod;
            waitTime = curPeriod;
        } else {
            waitTime = lastPeriod;
        }
        LOG_DEBUG( CL_LOG,
                   "About to wait %d msec before next health check...",
                   waitTime );
        m_cond.Wait( m_mutex, waitTime );
        LOG_DEBUG( CL_LOG, "...awoken!" );
        
        m_mutex.Release();
    }
}

void
Server::enableHealthChecking(bool enabled)
{
    TRACE( CL_LOG, "enableHealthChecking" );

    LOG_INFO( CL_LOG, 
               "enableHealthChecking - enabled: %s",
               enabled ? "true" : "false" );
    
    m_mutex.Acquire();
    if (enabled != m_healthCheckingEnabled) {
        m_healthCheckingEnabled = enabled;   
        m_cond.Signal();
    }
    m_mutex.Release();
}

void
Server::setHealthy(const HealthReport &report) 
{
    string key = mp_f->createNodeKey(getAppName(),
				     getGroupName(),
				     getNodeName(),
				     (m_flags & SF_MANAGED) == SF_MANAGED);
    string value =
        (report.getHealthState() == HealthReport::HS_HEALTHY)
        ? ClusterlibStrings::HEALTHY
        : ClusterlibStrings::UNHEALTHY;
 
    mp_f->updateNodeClientState(key, value);
    mp_f->updateNodeClientStateDesc(key, report.getStateDescription());
}

};	/* End of 'namespace clusterlib' */
