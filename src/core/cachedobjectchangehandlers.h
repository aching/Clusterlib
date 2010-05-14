/*
 * cachedobjectchangehandlers.h --
 *
 * The definition of CachedObjectChangeHandlers
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDOBJECTCHANGEHANDLERS_H_
#define	_CL_CACHEDOBJECTCHANGEHANDLERS_H_

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
        NOTIFYABLE_REMOVED_CHANGE,
        CURRENT_STATE_CHANGE,
        DESIRED_STATE_CHANGE,
        APPLICATIONS_CHANGE,
        GROUPS_CHANGE,
        DATADISTRIBUTIONS_CHANGE,
        NODES_CHANGE,
        PROCESSSLOTS_CHANGE,
        PROCESSSLOTS_USAGE_CHANGE,
        PROCESSSLOT_PROCESSINFO_CHANGE,
        PROPERTYLISTS_CHANGE,
        PROPERTYLIST_VALUES_CHANGE,
        SHARDS_CHANGE,
        NODE_PROCESS_SLOT_INFO_CHANGE,
        SYNCHRONIZE_CHANGE,
        PREC_LOCK_NODE_EXISTS_CHANGE,
        QUEUES_CHANGE,
        QUEUE_CHILD_CHANGE
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
     * Handle notifyable removed change.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleNotifyableRemovedChange(NotifyableImpl *ntp,
                                        int32_t etype,
                                        const std::string &key);

    /**
     * Handle current state changes on notifyables.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleCurrentStateChange(NotifyableImpl *ntp,
                                   int32_t etype,
                                   const std::string &key);

    /**
     * Handle desired state changes on notifyables.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleDesiredStateChange(NotifyableImpl *ntp,
                                   int32_t etype,
                                   const std::string &key);

    /**
     * Handle changes in the set of applications.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleApplicationsChange(NotifyableImpl *ntp,
                                   int32_t etype,
                                   const std::string &key);

    /**
     * Handle changes in the set of groups in
     * a group.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleGroupsChange(NotifyableImpl *ntp,
                             int32_t etype,
                             const std::string &key);

    /**
     * Handle changes in the set of data distributions
     * in a group.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleDataDistributionsChange(NotifyableImpl *ntp,
                                        int32_t etype,
                                        const std::string &key);

    /**
     * Handle changes in the set of nodes in a group.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleNodesChange(NotifyableImpl *ntp,
                            int32_t etype,
                            const std::string &key);

    /**
     * Handle changes in the set of process slots in a node.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleProcessSlotsChange(NotifyableImpl *ntp,
                                   int32_t etype,
                                   const std::string &key);

    /**
     * Handle changes in the usage of process slots in a node.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleProcessSlotsUsageChange(NotifyableImpl *ntp,
                                        int32_t etype,
                                        const std::string &key);

    /**
     * Handle changes in the port vector of a process slot.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleProcessSlotProcessInfoChange(NotifyableImpl *ntp,
                                         int32_t etype,
                                         const std::string &key);

    /**
     * Handle changes in the set of property lists in a notifyable.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handlePropertyListsChange(NotifyableImpl *ntp,
                                 int32_t etype,
                                 const std::string &key);

    /**
     * Handle changes in a property list value.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handlePropertyListValueChange(NotifyableImpl *ntp,
                                      int32_t etype,
                                      const std::string &key);

    /**
     * Handle changes in shards of a distribution.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleDataDistributionShardsChange(NotifyableImpl *ntp,
                                             int32_t etype,
                                             const std::string &key);

    /**
     * Handle a change in the process slot information for
     * a node.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleNodeProcessSlotInfoChange(NotifyableImpl *ntp,
                                          int32_t etype,
                                          const std::string &key);

    /**
     * Handle changes in synchronization of a zookeeper key.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleSynchronizeChange(NotifyableImpl *ntp,
                                  int32_t etype,
                                  const std::string &key);

    /**
     * Handle change of a preceding lock node exists event.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handlePrecLockNodeExistsChange(NotifyableImpl *ntp,
                                         int32_t etype,
                                         const std::string &key);

    /**
     * Handle changes in the set of queues in a notifyable
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleQueuesChange(NotifyableImpl *ntp,
                             int32_t etype,
                             const std::string &key);

    /**
     * Handle child change of a queue parent.
     *
     * @param ntp the notifyable pointer
     * @param etype the event mask on this notifyable
     * @param key the actual key that the event was on
     * @return the user-level event that will be processed by the user.
     */
    Event handleQueueChildChange(NotifyableImpl *ntp,
                                 int32_t etype,
                                 const std::string &key);

    /**
     * Get the CachedObjectEventHandler for the appropriate change event
     *
     * @param change the change that gets the appropriate handler
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
     * @return true if key and handler are set for callback
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

    /**
     * Constructor used by FactoryOps.
     * 
     * @param factoryOps the factory ops pointer
     */
    CachedObjectChangeHandlers(FactoryOps *factoryOps) 
        : mp_ops(factoryOps),
          m_notifyableRemovedChangeHandler(
              this,
              &CachedObjectChangeHandlers::handleNotifyableRemovedChange),
          m_currentStateChangeHandler(
              this,
              &CachedObjectChangeHandlers::handleCurrentStateChange),
          m_desiredStateChangeHandler(
              this,
              &CachedObjectChangeHandlers::handleDesiredStateChange),
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
          m_processSlotProcessInfoChangeHandler(
              this,
            &CachedObjectChangeHandlers::handleProcessSlotProcessInfoChange),
          m_nodeProcessSlotInfoChangeHandler(
              this,
              &CachedObjectChangeHandlers::handleNodeProcessSlotInfoChange),
          m_synchronizeChangeHandler(
              this,
              &CachedObjectChangeHandlers::handleSynchronizeChange),
          m_precLockNodeExistsChangeHandler(
              this,
              &CachedObjectChangeHandlers::handlePrecLockNodeExistsChange),
          m_queuesChangeHandler(
              this,
              &CachedObjectChangeHandlers::handleQueuesChange),
          m_queueChildChangeHandler(
              this,
              &CachedObjectChangeHandlers::handleQueueChildChange) {}
    
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
    CachedObjectEventHandler m_notifyableRemovedChangeHandler;
    CachedObjectEventHandler m_currentStateChangeHandler;
    CachedObjectEventHandler m_desiredStateChangeHandler;
    CachedObjectEventHandler m_propertyListsChangeHandler;
    CachedObjectEventHandler m_propertyListValueChangeHandler;
    CachedObjectEventHandler m_applicationsChangeHandler;
    CachedObjectEventHandler m_groupsChangeHandler;
    CachedObjectEventHandler m_dataDistributionsChangeHandler;
    CachedObjectEventHandler m_dataDistributionShardsChangeHandler;
    CachedObjectEventHandler m_nodesChangeHandler;
    CachedObjectEventHandler m_processSlotsChangeHandler;
    CachedObjectEventHandler m_processSlotsUsageChangeHandler;
    CachedObjectEventHandler m_processSlotProcessInfoChangeHandler;
    CachedObjectEventHandler m_nodeProcessSlotInfoChangeHandler;
    CachedObjectEventHandler m_synchronizeChangeHandler;
    CachedObjectEventHandler m_precLockNodeExistsChangeHandler;
    CachedObjectEventHandler m_queuesChangeHandler;
    CachedObjectEventHandler m_queueChildChangeHandler;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDOBJECTCHANGEHANDLERS_H_ */
