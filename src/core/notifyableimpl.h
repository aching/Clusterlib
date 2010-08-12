/*
 * notifyableimpl.h --
 *
 * Contains the implementation of the notifyable interface.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_NOTIFYABLEIMPL_H_
#define _CL_NOTIFYABLEIMPL_H_

namespace clusterlib {

struct NameRef {
    NameRef()
        : refCount(0) {}

    NameRef(int refCountArg, const std::string &lockOwnerArg)
        : refCount(refCountArg),
          lockOwner(lockOwnerArg) {}

    /**
     * Number of references on this lockOwner
     */
    int refCount;

    /**
     * Who actually has the lock
     */
    std::string lockOwner;
};

/**
 * Interface nad partial implementation that must be derived by
 * specific Notifyable objects.
 */
class NotifyableImpl
    : public virtual Notifyable, 
      public virtual boost::enable_shared_from_this<NotifyableImpl>
{
  public:
    virtual bool operator==(const Notifyable &other)
    {
        return (other.getKey() == getKey()) ? true : false;
    }

    virtual const std::string &getName() const 
    {
        return m_name; 
    }

    virtual const std::string &getKey() const 
    {
        return m_key; 
    }

    virtual boost::shared_ptr<Notifyable> getMyParent() const;
    
    virtual NotifyableList getMyChildren();

    virtual bool getMyApplicationWaitMsecs(
        int64_t msecTimeout,
        boost::shared_ptr<Application> *pApplicationSP); 

    virtual boost::shared_ptr<Application> getMyApplication(); 

    virtual bool getMyGroupWaitMsecs(
        int64_t msecTimeout,
        boost::shared_ptr<Group> *pGroupSP); 

    virtual boost::shared_ptr<Group> getMyGroup(); 

    virtual bool getNotifyableFromKeyWaitMsecs(
        const std::string &key,
        int64_t msecTimeout,
        boost::shared_ptr<Notifyable> *pNotifyableSP);

    virtual boost::shared_ptr<Notifyable> getNotifyableFromKey(
        const std::string &key);

    virtual Notifyable::State getState() const;
    
    virtual NameList getPropertyListNames();

    virtual bool getPropertyListWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<PropertyList> *pPropertyListSP);

    virtual boost::shared_ptr<PropertyList> getPropertyList(
        const std::string &name,
        AccessType accessType);

    virtual NameList getQueueNames();

    virtual bool getQueueWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Queue> *pQueueSP);

    virtual boost::shared_ptr<Queue> getQueue(const std::string &name,
                                              AccessType accessType);

    virtual void acquireLock(bool acquireChildren = false);

    virtual bool acquireLockWaitMsecs(int64_t msecTimeout, 
                                      bool acquireChildren = false);

    virtual void releaseLock(bool releaseChildren = false);
    
    virtual bool hasLock();
    
    virtual bool getLockInfo(std::string *id = NULL, 
                             int64_t *msecs = NULL);

    virtual NameList getLockBids(bool children);

    virtual void acquireOwnership();

    virtual bool acquireOwnershipWaitMsecs(int64_t msecTimeout);

    virtual void releaseOwnership();

    virtual bool hasOwnership();

    virtual bool getOwnershipInfo(std::string *id = NULL, 
                                  int64_t *msecs = NULL);

    virtual void remove(bool removeChildren = false);

    virtual CachedState &cachedCurrentState();

    virtual CachedState &cachedDesiredState();

    /*
     * Internal functions not used by outside clients
     */
  public:
    /**
     * Constructor.
     *
     * Note: Currently, clusterlib does not allow a user to remove an
     * object that has children.  If this changes, one possible
     * solution is to always keep a reference to parent.  This
     * prevents the parent cached representation from being removed
     * from memory which this notifyable is in the cache.  Upon
     * destruction, release that reference to the parent.
     *
     * @param fp pointer to the FactoryOps
     * @param key the key of this notifyable
     * @param name the name of this notifyable
     * @param parentSP Parent of this notifyable or NULL if this is the Root
     */
    NotifyableImpl(FactoryOps *fp,
                   const std::string &key,
                   const std::string &name,
                   const boost::shared_ptr<NotifyableImpl> &parentSP);

    /**
     * Get the associated factory object.
     */
    FactoryOps *getOps() { return mp_f; }

    /**
     * Destructor.
     */
    virtual ~NotifyableImpl() {}

    /**
     * Get the children notifyables specific to this subclassed
     * NotifyableImpl.  The children of NotifyableImpl (i.e. Queue and
     * PropertyList) are not included here.  This is a snapshot and
     * could change.
     *
     * @return A list of Notifyable * that are children specific to 
     *         this Notifyable
     */
    virtual NotifyableList getChildrenNotifyables() = 0;

    /**
     * Calls the local initializeCachedRepresentation() and also
     * initializes Notifyable cached data as well.
     */
    void initialize();

    /**
     * Initialize the cached representation when the object is loaded
     * into clusterlib.  All subclass specific handlers must be setup
     * in this function.  The implementation must be provided by
     * subclasses.
     */
    virtual void initializeCachedRepresentation() = 0;

    /**
     * Take all actions to remove all backend storage (i.e. Zookeeper data)
     */
    virtual void removeRepositoryEntries();

    /**
     * Get the key of the distributed lock owner
     *
     * @param lockName the name of the lock
     * @return the owner of the lock
     */
    virtual const std::string getDistributedLockOwner(
        const std::string &lockName);

    /**
     * Set the key of the distributed lock
     *
     * @param lockName the name of the lock
     * @param lockOwner the name of the lock owner
     */
    virtual void setDistributedLockOwner(const std::string &lockName,
                                         const std::string &lockOwner);
    
    /**
     * Safe reference count changes for distributed locks.
     *
     * @param lockName the name of the lock
     * @return the reference count after the operation
     */
    virtual int32_t incrDistributedLockOwnerCount(
        const std::string &lockName);
    
    /**
     * Safe reference count changes for distributed locks.
     *
     * @param lockName the name of the lock
     * @return the reference count after the operation
     */
    virtual int32_t decrDistributedLockOwnerCount(
        const std::string &lockName);

    /**
     * Get the number of references on the current distributed lock key
     *
     * @param lockName the name of the lock
     */
    virtual int32_t getDistributedLockOwnerCount(
        const std::string &lockName);

    /**
     * Check the state of the Notifyable and throw an exception if it
     * is removed.  This should be called prior to any Notifyable
     * member function.
     */
    virtual void throwIfRemoved() const
    {
        if (getState() == Notifyable::REMOVED) {
            throw ObjectRemovedException(
                std::string("throwIfRemoved: Notifyable ") +
                getKey() + 
                std::string(" was removed, exiting current "
                            "function"));
        }
    }

    /**
     * Get the NotifyableImpl synchronization lock
     * 
     * @return pointer to the synchronization lock
     */
    const Mutex &getSyncLock() const;

    /**
     * Get the NotifyableImpl synchronization lock for distributed locks
     * 
     * @return pointer to the synchronization lock for distributed locks
     */
    const Mutex &getSyncDistLock() const;

    /**
     * Return the stringified state
     */
    static std::string getStateString(Notifyable::State state);

    /**
     * Set the state of the NotifyableImpl.
     *
     * @param state the state to be set
     */
    void setState(NotifyableImpl::State state)
    {
        Locker l1(getSyncLock());
        m_state = state;
    }

    /**
     * Increment the reference count of this cached representation of
     * the object.
     */
    void incrRefCount();

    /**                                                                        
     * Set the SafeNotifyableMap that contains this object.
     *
     * @param safeNotifyableMap The SafeNotifyableMap that has this object.
     */
    void setSafeNotifyableMap(SafeNotifyableMap &safeNotifyableMap)
    {
        Locker l(&m_syncLock);
        m_safeNotifyableMap = &safeNotifyableMap;
    }

    /**                                                                        
     * Get the SafeNotifyableMap that contains this object.
     *
     * @return a reference to the map that contains this object
     */
    SafeNotifyableMap *getSafeNotifyableMap()
    {
        Locker l(&m_syncLock);
        return m_safeNotifyableMap;
    }

    /**
     * Create the current state JSONValue key
     *
     * @param notifyableKey The notifyable key.
     * @return the generated current state JSONValue key
     */
    static std::string createCurrentStateJSONValueKey(
        const std::string &notifyableKey);

    /**
     * Create the state JSONValue key
     *
     * @param notifyableKey The notifyable key.
     * @param stateType State type (current or desired)
     * @return the generated current state JSONValue key
     */
    static std::string createStateJSONArrayKey(
        const std::string &notifyableKey, 
        CachedStateImpl::StateType stateType);

  private:
    /*
     * Default constructor that no one should call.
     */
    NotifyableImpl();

    /*
     * Default assignment operator that no one should call.
     */
    NotifyableImpl & operator=(NotifyableImpl &other);

  private:
    /**
     * The associated factory delegate.
     */
    FactoryOps *mp_f;

    /**
     * The key to pass to the factory delegate for
     * operations on the represented cluster node.
     */
    const std::string m_key;

    /**
     * The name of the Notifyable.
     */
    const std::string m_name;

    /**
     * The parent notifyable (NULL if no parent)
     */
    boost::shared_ptr<NotifyableImpl> mp_parent;

    /**
     * Is this notifyable "init", "ready", or "deleted" according to the
     * Notifyable state protocol?
     */
    State m_state;
    
    /** 
     * All lock info (locks, their owners and their owner's reference
     * counts are stored here.
     */
    std::map<std::string, NameRef> m_distLockMap;

    /**                 
     * This is the map that contains this object (and these types of
     * objects).
     */
    SafeNotifyableMap *m_safeNotifyableMap;

    /**
     * Lock to synchronize the Notifyable with the clusterlib event
     * processing thread.  It is mutable since it is used by
     * throwIfRemoved() - which is const so it can be used everywhere.
     */
    Mutex m_syncLock;

    /**
     * Lock to synchronize all clusterlib distributed locks.
     */
    Mutex m_syncDistLock;

    /**
     * The cached current state
     */
    CachedStateImpl m_cachedCurrentState;

    /**
     * The cached desired state
     */
    CachedStateImpl m_cachedDesiredState;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NOTIFYABLEIMPL_H_ */
