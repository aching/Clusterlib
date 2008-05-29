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
  protected:
    /*
     * Make the factory a friend.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    ClusterClient(Factory *f, ZooKeeperAdapter *zk)
        : mp_f(f),
          mp_zk(zk)
    {
    }

  private:
    /*
     * Make the default constructor private so
     * noone can call it.
     */
    ClusterClient()
    {
        throw ClusterException("Someone called the default constructor!");
    }

  private:
    /*
     * The ZooKeeper adapter instance we're using.
     */
    ZooKeeperAdapter *mp_zk;

    /*
     * The factory instance we're using.
     */
    Factory *mp_f;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENT_H_ */
