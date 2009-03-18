/*
 * node.h --
 *
 * Definition of class Node; it represents a node in a group in an
 * application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_NODE_H_
#define _NODE_H_

namespace clusterlib
{

/*
 * Definition of class Node
 */
class Node
    : public virtual Notifyable
{
  public:
    /**
     * Is this node the leader of its group?
     *
     * @return true if I am the leader.
     */
    bool isLeader()
    {
        return (mp_group->getLeader() == this) ? true : false;
    }

    /**
     * Get the client-state of this node.
     *
     * @return a string representing the client state for this
     * node.
     */
    string getClientState() { return m_clientState; }

    /**
     * Get the master-set state of this node.
     *
     * @return an int32 value representing the state set by the
     * master for this node.
     */
    int32_t getMasterSetState() { return m_masterSetState; }

    /**
     * Is this node connected?
     *
     * @return true if this node is connected.
     */
    bool isConnected() { return m_connected; }

    /*
     * Return the time at which various events happened.
     */
    int64_t getClientStateTime() { return m_clientStateTime; }
    int64_t getMasterSetStateTime() { return m_masterSetStateTime; }
    int64_t getConnectionTime() { return m_connectionTime; }

    /**
     * Is this node healthy?
     * 
     * @return true if healthy, false if not
     */
    bool isHealthy()
    {
        return (m_clientState == "healthy") ? true : false;
    }

  protected:
    /*
     * Friend declaration for Factory so that it will be able
     * to call the constructor.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    Node(Group *group,
         const string &name,
         const string &key,
         FactoryOps *f)
        : Notifyable(f, key, name, group),
          mp_group(group),
          m_clientState(""),
          m_clientStateTime(0),
          m_masterSetState(0),
          m_masterSetStateTime(0),
          m_connected(false),
          m_connectionTime(0)
    {
    }

    /*
     * Destructor.
     */
    virtual ~Node() {}

    /*
     * Initialize the cached representation of this node.
     */
    virtual void initializeCachedRepresentation();

    /*
     * Set the client state associated with this node.
     */
    void setClientState(string ns) { m_clientState = ns; }
    void setClientStateTime(int64_t t) { m_clientStateTime = t; }

    /*
     * Set the master-set state associated with this node.
     */
    void setMasterSetState(int32_t ns) { m_masterSetState = ns; }
    void setMasterSetStateTime(int64_t t) { m_masterSetStateTime = t; }

    /*
     * Set the connected state of this node.
     */
    void setConnected(bool nc) { m_connected = nc; }
    void setConnectionTime(int64_t t) { m_connectionTime = t; }

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    Node()
        : Notifyable(NULL, "", "", NULL)
    {
        throw ClusterException("Someone called the Node default "
                               "constructor!");
    }

  private:
    /*
     * The group this node is in.
     */
    Group *mp_group;

    /*
     * The client state associated with this node.
     */
    string m_clientState;
    int64_t m_clientStateTime;

    /*
     * The master-set state associated with this node.
     */
    int32_t m_masterSetState;
    int64_t m_masterSetStateTime;

    /*
     * The connected state for this node.
     */
    bool m_connected;
    int64_t m_connectionTime;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NODE_H_ */
