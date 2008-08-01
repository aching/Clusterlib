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
    virtual bool CheckHealth() = 0;

    /*
     * Get/set the ClusterServer object that
     * this health checker is associated
     * with.
     */
    ClusterServer *getServer() { return mp_server; }
    void setServer(ClusterServer *s) { mp_server = s; }

  private:
    /*
     * Store the ClusterServer this health
     * checker is associated with.
     */
    ClusterServer *mp_server;
};

};	/* End of 'namespace clusterlib' */
#endif	/* !_HEALTHCHECKER_H_ */
