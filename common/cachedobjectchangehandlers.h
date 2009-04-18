/*
 * cachedobjectchangehandlers.h --
 *
 * The definition of CachedObjectChangeHandlers
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CACHEDOBJECTCHANGEHANDLERS_H_
#define	_CACHEDOBJECTCHANGEHANDLERS_H_

namespace clusterlib
{

/*
 * The actual factory class.
 */
class CachedObjectChangeHandlers
{
  public:


    /**
     * Handle state changes on notifyables.
     */
    Event handleNotifyableStateChange(NotifyableImpl *ntp,
                                      int32_t etype,
                                      const std::string &key);

    /**
     * Handle changes in the set of applications.
     */
    Event handleApplicationsChange(NotifyableImpl *ntp,
                                   int32_t etype,
                                   const std::string &key);

    /**
     * Handle changes in the set of groups in
     * a group.
     */
    Event handleGroupsChange(NotifyableImpl *ntp,
                             int32_t etype,
                             const std::string &key);

    /**
     * Handle changes in the set of data distributions
     * in a group.
     */
    Event handleDataDistributionsChange(NotifyableImpl *ntp,
                                        int32_t etype,
                                        const std::string &key);

    /**
     * Handle changes in the set of nodes in a group.
     */
    Event handleNodesChange(NotifyableImpl *ntp,
                            int32_t etype,
                            const std::string &key);

    /**
     * Handle changes in a property list value.
     */
    Event handlePropertiesValueChange(NotifyableImpl *ntp,
                                      int32_t etype,
                                      const std::string &key);

    /**
     * Handle changes in shards of a distribution.
     */
    Event handleShardsChange(NotifyableImpl *ntp,
                             int32_t etype,
                             const std::string &key);

    /**
     * Handle changes in manual overrides in
     * a distribution.
     */
    Event handleManualOverridesChange(NotifyableImpl *ntp,
                                      int32_t etype,
                                      const std::string &key);

    /**
     * Handle changes in client-reported state for
     * a node.
     */
    Event handleClientStateChange(NotifyableImpl *ntp,
                                  int32_t etype,
                                  const std::string &key);

    /**
     * Handle changes in master-set desired state
     * for a node.
     */
    Event handleMasterSetStateChange(NotifyableImpl *ntp,
                                     int32_t etype,
                                     const std::string &key);

    /**
     * Handle a change in the connected state for
     * a node.
     */
    Event handleNodeConnectionChange(NotifyableImpl *ntp,
                                     int32_t etype,
                                     const std::string &key);

    /**
     * Handle changes in the leadership of a
     * group.
     */
    Event handleLeadershipChange(NotifyableImpl *ntp,
                                 int32_t etype,
                                 const std::string &key);

    /**
     * Handle existence change for preceding leader of
     * a group.
     */
    Event handlePrecLeaderExistsChange(NotifyableImpl *ntp,
                                       int32_t etype,
                                       const std::string &key);

    /**
     * Handle changes in synchronization of a zookeeper key.
     */
    Event handleSynchronizeChange(NotifyableImpl *ntp,
                                  int32_t etype,
                                  const std::string &key);

    /*
     * Get the CachedObjectEventHandler for the appropriate change event
     */
    CachedObjectEventHandler *getNotifyableStateChangeHandler()
    {
        return &m_notifyableStateChangeHandler;
    }
    CachedObjectEventHandler *getPropertiesValueChangeHandler()
    {
        return &m_propertiesValueChangeHandler;
    }
    CachedObjectEventHandler *getApplicationsChangeHandler()
    {
        return &m_applicationsChangeHandler;
    }
    CachedObjectEventHandler *getGroupsChangeHandler()
    {
        return &m_groupsChangeHandler;
    }
    CachedObjectEventHandler *getDataDistributionsChangeHandler()
    {
        return &m_dataDistributionsChangeHandler;
    }
    CachedObjectEventHandler *getShardsChangeHandler()
    {
        return &m_shardsChangeHandler;
    }
    CachedObjectEventHandler *getManualOverridesChangeHandler()
    {
        return &m_manualOverridesChangeHandler;
    }
    CachedObjectEventHandler *getNodesChangeHandler()
    {
        return &m_nodesChangeHandler;
    }
    CachedObjectEventHandler *getNodeClientStateChangeHandler()
    {
        return &m_nodeClientStateChangeHandler;
    }
    CachedObjectEventHandler *getNodeMasterSetStateChangeHandler()
    {
        return &m_nodeMasterSetStateChangeHandler;
    }
    CachedObjectEventHandler *getNodeConnectionChangeHandler()
    {
        return &m_nodeConnectionChangeHandler;
    }
    CachedObjectEventHandler *getLeadershipChangeHandler()
    {
        return &m_leadershipChangeHandler;
    }
    CachedObjectEventHandler *getPrecLeaderExistsHandler()
    {
        return &m_precLeaderExistsHandler;
    }
    CachedObjectEventHandler *getSynchronizeChangeHandler()
    {
        return &m_synchronizeChangeHandler;
    }

    /*
     * Constructor used by FactoryOps.
     */
    CachedObjectChangeHandlers(FactoryOps *factoryOps) 
        : mp_ops(factoryOps),
        m_notifyableStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleNotifyableStateChange),
        m_propertiesValueChangeHandler(
            this,
            &CachedObjectChangeHandlers::handlePropertiesValueChange),
        m_applicationsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleApplicationsChange),
        m_groupsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleGroupsChange),
        m_dataDistributionsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleDataDistributionsChange),
        m_shardsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleShardsChange),
        m_manualOverridesChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleManualOverridesChange),
        m_nodesChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleNodesChange),
        m_nodeClientStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleClientStateChange),
        m_nodeMasterSetStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleMasterSetStateChange),
        m_nodeConnectionChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleNodeConnectionChange),
        m_leadershipChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleLeadershipChange),
        m_precLeaderExistsHandler(
            this,
            &CachedObjectChangeHandlers::handlePrecLeaderExistsChange),
        m_synchronizeChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleSynchronizeChange) {}

  private:
    /**
     * Private access to mp_ops
     */
    FactoryOps *getOps() { return mp_ops; }
    
  private:
    /**
     * Does all the factory operations
     */
    FactoryOps *mp_ops;

    /*
     * Handlers for event delivery.
     */
    CachedObjectEventHandler m_notifyableStateChangeHandler;
    CachedObjectEventHandler m_propertiesValueChangeHandler;
    CachedObjectEventHandler m_applicationsChangeHandler;
    CachedObjectEventHandler m_groupsChangeHandler;
    CachedObjectEventHandler m_dataDistributionsChangeHandler;
    CachedObjectEventHandler m_shardsChangeHandler;
    CachedObjectEventHandler m_manualOverridesChangeHandler;
    CachedObjectEventHandler m_nodesChangeHandler;
    CachedObjectEventHandler m_nodeClientStateChangeHandler;
    CachedObjectEventHandler m_nodeMasterSetStateChangeHandler;
    CachedObjectEventHandler m_nodeConnectionChangeHandler;
    CachedObjectEventHandler m_leadershipChangeHandler;
    CachedObjectEventHandler m_precLeaderExistsHandler;
    CachedObjectEventHandler m_synchronizeChangeHandler;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CACHEDOBJECTCHANGEHANDLERS_H_ */
