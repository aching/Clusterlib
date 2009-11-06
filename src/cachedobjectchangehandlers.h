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

/**
 * The class for all cached object handlers.
 */
class CachedObjectChangeHandlers
{
  public:
    /**
     * Define an enum for the change handlers.
     */
    enum CachedObjectChange {
        NOTIFYABLE_STATE_CHANGE,
        APPLICATIONS_CHANGE,
        GROUPS_CHANGE,
        DATADISTRIBUTIONS_CHANGE,
        NODES_CHANGE,
        PROCESSSLOTS_CHANGE,
        PROCESSSLOTS_USAGE_CHANGE,
        PROCESSSLOT_PORTVEC_CHANGE,
        PROCESSSLOT_EXECARGS_CHANGE,
        PROCESSSLOT_RUNNING_EXECARGS_CHANGE,
        PROCESSSLOT_PID_CHANGE,
        PROCESSSLOT_DESIRED_STATE_CHANGE,
        PROCESSSLOT_CURRENT_STATE_CHANGE,
        PROCESSSLOT_RESERVATION_CHANGE,
        PROPERTYLISTS_CHANGE,
        PROPERTYLIST_VALUES_CHANGE,
        SHARDS_CHANGE,
        NODE_CLIENT_STATE_CHANGE,
        NODE_MASTER_SET_STATE_CHANGE,
        NODE_CONNECTION_CHANGE,
        SYNCHRONIZE_CHANGE,
        PREC_LOCK_NODE_EXISTS_CHANGE
    };

    static std::string getCachedObjectChangeString(
        CachedObjectChange change);
 
    /**
     * Get the lock that protects this entire data structure.  This is
     * primarily used to ensure that the object that manages the
     * handlers and paths is thread-safe.  It can be used with
     * (is|set|unset)HandlerCallbackReady() since they use the same
     * lock and it is re-entrant.
     *
     * @return Pointer to the mutex
     */
    Mutex *getLock() { return &m_mutex; }

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
     * Handle changes in the set of process slots in a node.
     */
    Event handleProcessSlotsChange(NotifyableImpl *ntp,
                                   int32_t etype,
                                   const std::string &key);

    /**
     * Handle changes in the usage of process slots in a node.
     */
    Event handleProcessSlotsUsageChange(NotifyableImpl *ntp,
                                        int32_t etype,
                                        const std::string &key);

    /**
     * Handle changes in the port vector of a process slot.
     */
    Event handleProcessSlotPortVecChange(NotifyableImpl *ntp,
                                         int32_t etype,
                                         const std::string &key);

    /**
     * Handle changes in the executable arguments of a process slot.
     */
    Event handleProcessSlotExecArgsChange(NotifyableImpl *ntp,
                                          int32_t etype,
                                          const std::string &key);

    /**
     * Handle changes in the running executable arguments of a process
     * slot.
     */
    Event handleProcessSlotRunningExecArgsChange(NotifyableImpl *ntp,
                                                 int32_t etype,
                                                 const std::string &key);

    /**
     * Handle changes in the PID of a process slot.
     */
    Event handleProcessSlotPIDChange(NotifyableImpl *ntp,
                                     int32_t etype,
                                     const std::string &key);

    /**
     * Handle changes in the desired state of a process slot.
     */
    Event handleProcessSlotDesiredStateChange(NotifyableImpl *ntp,
                                              int32_t etype,
                                              const std::string &key);

    /**
     * Handle changes in the current state of a process slot.
     */
    Event handleProcessSlotCurrentStateChange(NotifyableImpl *ntp,
                                              int32_t etype,
                                              const std::string &key);

    /**
     * Handle changes in the reservation of a process slot.
     */
    Event handleProcessSlotReservationChange(NotifyableImpl *ntp,
                                             int32_t etype,
                                             const std::string &key);

    /**
     * Handle changes in the set of property lists in a notifyable.
     */
    Event handlePropertyListsChange(NotifyableImpl *ntp,
                                 int32_t etype,
                                 const std::string &key);

    /**
     * Handle changes in a property list value.
     */
    Event handlePropertyListValueChange(NotifyableImpl *ntp,
                                      int32_t etype,
                                      const std::string &key);

    /**
     * Handle changes in shards of a distribution.
     */
    Event handleDataDistributionShardsChange(NotifyableImpl *ntp,
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
     * Handle changes in synchronization of a zookeeper key.
     */
    Event handleSynchronizeChange(NotifyableImpl *ntp,
                                  int32_t etype,
                                  const std::string &key);

    /**
     * Handle change of a preceding lock node exists event.
     */
    Event handlePrecLockNodeExistsChange(NotifyableImpl *ntp,
                                         int32_t etype,
                                         const std::string &key);

    /**
     * Get the CachedObjectEventHandler for the appropriate change event
     *
     * @return a pointer to the desired event handler
     */
    CachedObjectEventHandler *getChangeHandler(CachedObjectChange change);

    /**
     * Each handler needs to be called at least once per key and event
     * they are waiting on.  Additionally, it is most beneficial for
     * performance reasons that they are called exactly once per event
     * they are waiting on.  This function gets the number of times it
     * will be called per event.
     *
     * @param change the change handler
     * @param key the path of the node
     * @return if key and handler are set for callback
     */
    bool isHandlerCallbackReady(CachedObjectChange change, 
                                const std::string &key);

    /**
     * Set a handler ready for a key and event.
     *
     * @param change the change handler
     * @param key the path of the node
     */
    void setHandlerCallbackReady(CachedObjectChange change,
                                 const std::string &key);

    /**
     * Unset a handler ready for a key and event.
     *
     * @param change the change handler
     * @param key the path of the node
     */
    void unsetHandlerCallbackReady(CachedObjectChange change,
                                   const std::string &key);

    /*
     * Constructor used by FactoryOps.
     */
    CachedObjectChangeHandlers(FactoryOps *factoryOps) 
        : mp_ops(factoryOps),
        m_notifyableStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleNotifyableStateChange),
        m_propertyListsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handlePropertyListsChange),
        m_propertyListValueChangeHandler(
            this,
            &CachedObjectChangeHandlers::handlePropertyListValueChange),
        m_applicationsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleApplicationsChange),
        m_groupsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleGroupsChange),
        m_dataDistributionsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleDataDistributionsChange),
        m_dataDistributionShardsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleDataDistributionShardsChange),
        m_nodesChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleNodesChange),
        m_processSlotsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotsChange),
        m_processSlotsUsageChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotsUsageChange),
        m_processSlotPortVecChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotPortVecChange),
        m_processSlotExecArgsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotExecArgsChange),
        m_processSlotRunningExecArgsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotRunningExecArgsChange),
        m_processSlotPIDChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotPIDChange),
        m_processSlotDesiredStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotDesiredStateChange),
        m_processSlotCurrentStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotCurrentStateChange),
        m_processSlotReservationChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleProcessSlotReservationChange),
        m_nodeClientStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleClientStateChange),
        m_nodeMasterSetStateChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleMasterSetStateChange),
        m_nodeConnectionChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleNodeConnectionChange),
        m_synchronizeChangeHandler(
            this,
            &CachedObjectChangeHandlers::handleSynchronizeChange),
        m_precLockNodeExistsChangeHandler(
            this,
            &CachedObjectChangeHandlers::handlePrecLockNodeExistsChange) {}

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

    /**
     * Protects m_handlerKeyCallbackCount from race conditions.  
     */
    Mutex m_mutex;

    /**
     * Each handler is represented by a CachedObjectChange and will be
     * set to run at most once per path (string).  m_mutex protects
     * access to this object.
     */
    std::map<CachedObjectChange, std::map<std::string, bool> >
        m_handlerKeyCallbackCount;

    /*
     * Handlers for event delivery.
     */
    CachedObjectEventHandler m_notifyableStateChangeHandler;
    CachedObjectEventHandler m_propertyListsChangeHandler;
    CachedObjectEventHandler m_propertyListValueChangeHandler;
    CachedObjectEventHandler m_applicationsChangeHandler;
    CachedObjectEventHandler m_groupsChangeHandler;
    CachedObjectEventHandler m_dataDistributionsChangeHandler;
    CachedObjectEventHandler m_dataDistributionShardsChangeHandler;
    CachedObjectEventHandler m_nodesChangeHandler;
    CachedObjectEventHandler m_processSlotsChangeHandler;

    CachedObjectEventHandler m_processSlotsUsageChangeHandler;
    CachedObjectEventHandler m_processSlotPortVecChangeHandler;
    CachedObjectEventHandler m_processSlotExecArgsChangeHandler;
    CachedObjectEventHandler m_processSlotRunningExecArgsChangeHandler;
    CachedObjectEventHandler m_processSlotPIDChangeHandler;
    CachedObjectEventHandler m_processSlotDesiredStateChangeHandler;
    CachedObjectEventHandler m_processSlotCurrentStateChangeHandler;
    CachedObjectEventHandler m_processSlotReservationChangeHandler;

    CachedObjectEventHandler m_nodeClientStateChangeHandler;
    CachedObjectEventHandler m_nodeMasterSetStateChangeHandler;
    CachedObjectEventHandler m_nodeConnectionChangeHandler;
    CachedObjectEventHandler m_synchronizeChangeHandler;
    CachedObjectEventHandler m_precLockNodeExistsChangeHandler;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CACHEDOBJECTCHANGEHANDLERS_H_ */
