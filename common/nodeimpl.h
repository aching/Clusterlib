/*
 * nodeimpl.h --
 *
 * Definition of class NodeImpl; it represents a node in a group in an
 * application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_NODEIMPL_H_
#define _NODEIMPL_H_

namespace clusterlib
{

/*
 * Definition of class Node
 */
class NodeImpl
    : public virtual Node, 
      public virtual NotifyableImpl
{
  public:
    /**
     * Is this node the leader of its group?
     *
     * @return true if I am the leader.
     */
    virtual bool isLeader()
    {
        return (mp_group->getLeader() == this) ? true : false;
    }

    /**
     * Get the client-state of this node.
     *
     * @return a string representing the client state for this
     * node.
     */
    virtual std::string getClientState() { return m_clientState; }

    /**
     * Get the master-set state of this node.
     *
     * @return an int32 value representing the state set by the
     * master for this node.
     */
    virtual int32_t getMasterSetState() { return m_masterSetState; }

    /**
     * Is this node connected?
     *
     * @return true if this node is connected.
     */
    virtual bool isConnected() { return m_connected; }

    /*
     * Return the time at which various events happened.
     */
    virtual int64_t getClientStateTime() { return m_clientStateTime; }
    virtual int64_t getMasterSetStateTime() { return m_masterSetStateTime; }
    virtual int64_t getConnectionTime() { return m_connectionTime; }

    /**
     * Is this node healthy?
     * 
     * @return true if healthy, false if not
     */
    virtual bool isHealthy()
    {
        return (m_clientState == "healthy") ? true : false;
    }

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /*
     * Constructor used by the factory.
     */
    NodeImpl(GroupImpl *group,
             const std::string &name,
             const std::string &key,
             FactoryOps *f)
        : NotifyableImpl(f, key, name, group),
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
    virtual ~NodeImpl() {}

    /*
     * Initialize the cached representation of this node.
     */
    virtual void initializeCachedRepresentation();

    /*
     * Set the client state associated with this node.
     */
    void setClientState(std::string ns) { m_clientState = ns; }
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
    NodeImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw ClusterException("Someone called the Node default "
                               "constructor!");
    }

  private:
    /*
     * The group this node is in.
     */
    GroupImpl *mp_group;

    /*
     * The client state associated with this node.
     */
    std::string m_clientState;
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

#endif	/* !_NODEIMPL_H_ */
