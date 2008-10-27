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

class Server
    : public virtual Client
{
  public:
    /*
     * Retrieve the node object for "my" node.
     */
    Node *getMyNode() { return mp_node; }

    /*
     * Retrieve the names of the node,
     * group, and application.
     */
    string getNodeName() { return m_nodeName; }
    string getGroupName() { return m_grpName; }
    string getAppName() { return m_appName; }

    /*
     * Is this server managed?
     */
    bool isManaged()
    {
        return (m_flags & SF_MANAGED) ? true : false;
    }

  protected:
    /*
     * Make the Factory class a friend.
     */
    friend class Factory;

    /*
     * Constructor used by Factory.
     */
    Server(FactoryOps *f,
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
    Server() : Client(NULL)
    {
        throw ClusterException("Someone called the Server "
                               "default constructor!");
    }

    /*
     * Make the destructor private also.
     */
    ~Server() {}

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
     * The node that represents "my node".
     */
    Node *mp_node;

    /*
     * The thread running the health checker.
     */
    CXXThread<HealthChecker> m_checkerThread;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERSERVER_H_ */
