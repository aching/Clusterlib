/*
 * healthchecker.h --
 *
 * Interface for objects that check health.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_HEALTHCHECKER_H_
#define	_HEALTHCHECKER_H_

namespace clusterlib
{

/**
 * \brief Represents a health report of a cluster node.
 */
class HealthReport {
    public:
        
        /**
         * \brief All possible health states of a cluster node.
         */
        enum HealthState {
            HS_HEALTHY = 0,
            HS_UNHEALTHY
        };
        
        /**
         * \brief Constructs a health report.
         * 
         * @param healthState the health state
         * @param desc the optional description of the health state
         */
        HealthReport(HealthState healthState, const std::string &desc = "") : 
            m_healthState(healthState), m_stateDescription(desc) {}
        
        /**
         * \brief Returns current health state.
         * 
         * @return current health state
         */
        HealthState getHealthState() const 
        {
            return m_healthState;
        }
        
        /**
         * \brief Returns the current health state's description, if any.
         * 
         * @return health state's descrription
         */
        const std::string getStateDescription() const 
        {
            return m_stateDescription;
        }
        
    private:
        
        /**
         * Whether healthy or not.
         */
        HealthState m_healthState;
        
        /**
         * Provides more detailed description of {@link m_healthState}.
         */
        std::string m_stateDescription;
};

class HealthChecker
{
  public:
    /*
     * Constructor.
     */
    HealthChecker()
        : mp_server(NULL)
    {
    }

    /*
     * Destructor.
     */
    virtual ~HealthChecker() {}

    /*
     * Must be supplied by sub-classes.
     */
    virtual HealthReport checkHealth() = 0;

    /*
     * Get/set the Server object that
     * this health checker is associated
     * with.
     */
    Server *getServer() { return mp_server; }
    void setServer(Server *s) { mp_server = s; }

  private:
    /*
     * Store the Server this health
     * checker is associated with.
     */
    Server *mp_server;
};

};	/* End of 'namespace clusterlib' */
#endif	/* !_HEALTHCHECKER_H_ */
