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

  protected:
    /*
     * Make the factory a friend.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    ClusterClient(Factory *f, 
                  ::zk::ZooKeeperAdapter *zk)
        : mp_f(f),
          mp_zk(zk)
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
     * The factory instance we're using.
     */
    Factory *mp_f;

    /*
     * The ZooKeeper adapter instance we're using.
     */
    ::zk::ZooKeeperAdapter *mp_zk;

    /*
     * All the applications.
     */
    ApplicationMap m_apps;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENT_H_ */
