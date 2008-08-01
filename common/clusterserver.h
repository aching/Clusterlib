/*
 * clusterserver.h --
 *
 * Include file for server side types. Include this file if you are writing
 * an implementation of an application that is managed by clusterlib.
 *
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERSERVER_H_
#define	_CLUSTERSERVER_H_

namespace clusterlib
{

class ClusterServer
    : public ClusterClient
{
  public:
    /*
     * Retrieve the node object for "my" node.
     */
    Node *getMyNode() { return mp_node; }

    /*
     * Retrieve the key for that node object.
     */
    string getMyKey() { return m_key; }

  protected:
    /*
     * Make the Factory class a friend.
     */
    friend class Factory;

    /*
     * Constructor used by Factory.
     */
    ClusterServer(FactoryOps *f,
                  const string &app,
                  const string &grp,
                  const string &node,
                  HealthChecker *checker,
                  ServerFlags flags);

  private:
    /*
     * Make the default constructor private so it
     * cannot be called.
     */
    ClusterServer()
        : ClusterClient(NULL)
    {
        throw ClusterException("Someone called the ClusterServer "
                               "default constructor!");
    }

  private:
    /*
     * The factory delegate instance.
     */
    FactoryOps *mp_f;

    /*
     * The components of my node's key.
     */
    string m_appName;
    string m_grpName;
    string m_nodeName;

    /*
     * The object implementing health checking
     * for this "server".
     */
    HealthChecker *mp_healthChecker;

    /*
     * Flags for this server.
     */
    ServerFlags m_flags;

    /*
     * The key of my node.
     */
    string m_key;

    /*
     * The node that represents "my node".
     */
    Node *mp_node;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERSERVER_H_ */
