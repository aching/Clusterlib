/*
 * client.h --
 *
 * Include file for client side types. Include this file if you're only writing
 * a pure clusterlib client. If you are creating a server (a node that is in a
 * group inside some app using clusterlib) then you need to include the server-
 * side functionality: server/clusterserver.h.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef _CLUSTERCLIENT_H_
#define _CLUSTERCLIENT_H_

namespace clusterlib
{

class ClusterClient
{
  public:
    /*
     * Retrieve an application.
     */
    Application *getApplication(const string &appName)
        throw(ClusterException);

    /*
     * Retrieve a map of all applications.
     */
    ApplicationMap *getApplications() throw(ClusterException);

    /*
     * Get the input queue of this client.
     */
    ClientEventQueue *getQueue() { return &m_queue; }

  protected:
    /*
     * Make the factory a friend.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    ClusterClient(FactoryOps *f)
        : mp_f(f)
    {
        m_apps.clear();
    }

  private:
    /*
     * Make the default constructor private so
     * noone can call it.
     */
    ClusterClient()
    {
        throw ClusterException("Someone called the ClusterClient "
                               "default constructor!");
    }

  private:
    /*
     * The factory delegate instance we're using.
     */
    FactoryOps *mp_f;

    /*
     * All the applications.
     */
    ApplicationMap m_apps;

    /*
     * The blocking queue for delivering notifications
     * to this client.
     */
    ClientEventQueue m_queue;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENT_H_ */
