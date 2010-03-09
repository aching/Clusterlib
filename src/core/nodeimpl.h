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

#ifndef	_CL_NODEIMPL_H_
#define _CL_NODEIMPL_H_

namespace clusterlib
{

/**
 * Definition of class NodeImpl.
 */
class NodeImpl
    : public virtual Node, 
      public virtual NotifyableImpl
{
  public:
    virtual void getClientState(int64_t *msecs,
                                std::string *clientState,
                                std::string *clientStateDesc);

    virtual int32_t getMasterSetState() 
    {
        Locker l1(getStateMutex());
        return m_masterSetState; 
    }

    virtual bool isConnected(std::string *id = NULL, int64_t *msecs = NULL);

    virtual bool initializeConnection(bool force = false);

    virtual int64_t getClientStateTime() 
    {
        Locker l1(getStateMutex());
        return m_clientStateTime; 
    }

    virtual int64_t getMasterSetStateTime() 
    {
        Locker l1(getStateMutex());
        return m_masterSetStateTime; 
    }

    virtual int64_t getConnectionTime() 
    {
        Locker l1(getStateMutex());
        return m_connectionTime; 
    }

    virtual bool isHealthy();

    virtual void registerHealthChecker(HealthChecker *healthChecker);

    virtual void unregisterHealthChecker();

    virtual void setUseProcessSlots(bool use);

    virtual bool getUseProcessSlots();

    virtual NameList getProcessSlotNames();

    virtual ProcessSlot *getProcessSlot(const std::string &name, 
                                        bool create = false);

    virtual int32_t getMaxProcessSlots();

    virtual void setMaxProcessSlots(int32_t maxProcessSlots);

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /*
     * Constructor used by the factory.
     */
    NodeImpl(FactoryOps *fp,
             const std::string &key,
             const std::string &name,
             GroupImpl *group)
        : NotifyableImpl(fp, key, name, group),
          mp_group(group),
          m_clientState(""),
          m_clientStateTime(0),
          m_masterSetState(0),
          m_masterSetStateTime(0),
          m_connected(false),
          m_connectionTime(0),
          mp_healthChecker(NULL),
          m_terminateDoHealthChecks(false) {}

    /*
     * Destructor.
     */
    virtual ~NodeImpl();

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries();

    /**
     * Set the client state, client state description, and set time
     * associated with this node.
     *
     * @param msecs the msecs since the epoch when the state was last reported
     * @param clientState the current state of the node (defined in class
     *        HealthReport)
     * @param clientStateDesc the human readable state of the node
     */
    void setClientState(int64_t msecs, 
                        std::string clientState, 
                        std::string clientStateDesc) 
    {
        Locker l1(getStateMutex());
        m_clientStateTime = msecs;
        m_clientState = clientState; 
        m_clientStateDesc = clientStateDesc;
    }

    /*
     * Set the master-set state and set time associated with this node.
     */
    void setMasterSetStateAndTime(int32_t ns, int64_t t) 
    {
        Locker l1(getStateMutex());
        m_masterSetState = ns; 
        m_masterSetStateTime = t; 
    }

    /*
     * Set the connected state and connected time of this node.
     */
    void setConnectedAndTime(bool nc, const std::string &id, int64_t t);

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    NodeImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException("Someone called the Node default "
                                       "constructor!");
    }

    /**
     * mp_healthChecker is called by this member function.
     */
    void doHealthChecks(void *param);

    /**
     * Protects client, master, and connected state and their
     * respective set times.
     */
    Mutex *getStateMutex()
    {
        return &m_stateMutex;
    }

    Mutex *getHealthMutex() 
    {
        return &m_healthMutex;
    }

    Cond *getHealthCond()
    {
        return &m_healthCond;
    }

  private:
    /**
     * The group this node is in.
     */
    GroupImpl *mp_group;

    /**
     * Protects m_clientState, m_clientStateTime, m_masterSetState,
     * m_masterSetStateTime, m_connection, m_connectionTime
     */
    Mutex m_stateMutex;

    /*
     * The client state associated with this node.
     */
    std::string m_clientState;
    std::string m_clientStateDesc;
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
    std::string m_connectedId;
    int64_t m_connectionTime;

    /**
     * Protects healthChecker thread variables 
     */
    Mutex m_healthMutex;

    /**
     * Used in conjunction with m_healthMutex to be a timer and also
     * to quit waiting if we are trying to unregisterHealthChecker.
     */
    Cond m_healthCond;

    /**
     * Pointer to the client object that does that actual health
     * check.
     */
    HealthChecker *mp_healthChecker;

    /**
     * Continue to do the health checks?
     */
    volatile bool m_terminateDoHealthChecks;

    /*
     * The thread doing the health checks.
     */
    CXXThread<NodeImpl> m_doHealthChecksThread;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NODEIMPL_H_ */
