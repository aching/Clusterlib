/*
 * healthchecker.h --
 *
 * Interface for objects that check health.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_HEALTHCHECKER_H_
#define	_CL_HEALTHCHECKER_H_

namespace clusterlib {

/**
 * \brief Represents a health report of a cluster node.
 */
class HealthReport
{
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
        : m_msecsPerCheckIfHealthy(60*1000),
          m_msecsPerCheckIfUnhealthy(15*1000),
          m_msecsAllowedPerHealthCheck(60*1000) {}

    /*
     * Destructor.
     */
    virtual ~HealthChecker() {}

    /**
     * Must be supplied by sub-classes.
     * 
     * @return the HealthReport after checking the health.
     */
    virtual HealthReport checkHealth() = 0;

    /**
     * Get the amount of time in between health checks if healthy.  It
     * is safe to call this even after the health checker has been
     * registered.
     *
     * @return the amount of time in milliseconds to wait in between
     * health checks if the last check reported healthy.
     */
    int64_t getMsecsPerCheckIfHealthy() 
    {
        return m_msecsPerCheckIfHealthy;
    }

    /**
     * Set the amount of time in between health checks if healthy.  It
     * is safe to call this even after the health checker has been
     * registered.
     *
     * @param msecsPerCheckIfHealthy the amount of time in
     * milliseconds to wait in between health checks if the last check
     * reported healthy.
     */
    void setMsecsPerCheckIfHealthy(int64_t msecsPerCheckIfHealthy)
    {
        Locker l1(getMutex());
        if (msecsPerCheckIfHealthy > 0) {
            m_msecsPerCheckIfHealthy = msecsPerCheckIfHealthy;
        }
        else {
            throw InvalidArgumentsException(
                "setMsecsPerCheckIfHealthy: msecsPerCheckIfHealthy <= 0");
        }
    }

    /**
     * Get the amount of time in between health checks if unhealthy.  It
     * is safe to call this even after the health checker has been
     * registered.
     *
     * @return the amount of time in milliseconds to wait in between
     * health checks if the last check reported unhealthy.
     */
    int64_t getMsecsPerCheckIfUnhealthy() 
    {
        Locker l1(getMutex());
        return m_msecsPerCheckIfUnhealthy;
    }

    /**
     * Set the amount of time in between health checks if unhealthy.  It
     * is safe to call this even after the health checker has been
     * registered.
     *
     * @param msecsPerCheckIfUnhealthy the amount of time in
     * milliseconds to wait in between health checks if the last check
     * reported unhealthy.
     */
    void setMsecsPerCheckIfUnhealthy(int64_t msecsPerCheckIfUnhealthy)
    {
        Locker l1(getMutex());
        if (msecsPerCheckIfUnhealthy > 0) {
            m_msecsPerCheckIfUnhealthy = msecsPerCheckIfUnhealthy;
        }
        else {
            throw InvalidArgumentsException(
                "setMsecsPerCheckIfUnhealthy: msecsPerCheckIfUnhealthy <= 0");
        }
    }

    /**
     * Get the amount of time to wait for checkHealth() to complete.
     * It is safe to call this even after the health checker has been
     * registered.
     *
     * @return the amount of time in milliseconds to wait for
     * checkHealth() to complete.
     */
    int64_t getMsecsAllowedPerHealthCheck() 
    {
        Locker l1(getMutex());
        return m_msecsAllowedPerHealthCheck;
    }

    /**
     * Set the amount of time to wait for checkHealth() to complete.
     * It is safe to call this even after the health checker has been
     * registered.
     *
     * @param msecsAllowedPerHealthCheck the amount of time in
     * milliseconds to wait for checkHealth() to complete.
     */
    void setMsecsAllowedPerHealthCheck(int64_t msecsAllowedPerHealthCheck) 
    {
        Locker l1(getMutex());
        if (msecsAllowedPerHealthCheck > 0) {
            m_msecsAllowedPerHealthCheck = msecsAllowedPerHealthCheck;
        }
        else {
            throw InvalidArgumentsException(
                "getMsecsAllowedPerHealthCheck: "
                "msecsAllowedPerHealthCheck <= 0");
        }
    }

  private:
    Mutex *getMutex() { return &m_mutex; }

  private:
    /**
     * Lock to guarantee consistent internal state
     */
    Mutex m_mutex;

    /**
     * Milliseconds to wait to in between checks if the last check was
     * healthy.
     */
    int64_t m_msecsPerCheckIfHealthy;

    /**
     * Milliseconds to wait to in between checks if the last check was
     * unhealthy.
     */
    int64_t m_msecsPerCheckIfUnhealthy;

    /**
     * Milliseconds to allow a 'checkHealth()' to wait before declared
     * failed.
     */
    int64_t m_msecsAllowedPerHealthCheck;
};

}	/* End of 'namespace clusterlib' */
#endif	/* !_CL_HEALTHCHECKER_H_ */
