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

namespace clusterlib
{

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
    /*
     * Who actually has the lock
     */
    std::string lockOwner;
};

/**
 * Interface that must be derived by specific notifyable objects.
 */
class NotifyableImpl
    : public virtual Notifyable
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

#if TO_BE_IMPLEMENTED_IF_NECESSARY
    virtual std::string getMyApplicationName() const;

    virtual std::string getMyGroupName() const;
#endif

    virtual Notifyable *getMyParent() const;
    
    virtual NotifyableList getMyChildren();

    virtual Application *getMyApplication(); 

    virtual Group *getMyGroup(); 

    virtual Notifyable::State getState() const;
    
    virtual NameList getPropertyListNames();

    virtual PropertyList *getPropertyList(const std::string &name,
                                          bool create = false);

    virtual NameList getQueueNames();

    virtual Queue *getQueue(const std::string &name,
                            bool create = false);

    virtual int32_t getRefCount()
    {
        Locker l(getRefCountLock());
        return m_refCount;
    }

    virtual void releaseRef();

    virtual void acquireLock(bool acquireChildren = false);

    virtual bool acquireLockWaitMsecs(int64_t msecTimeout, 
                                      bool acquireChildren = false);

    virtual void releaseLock(bool releaseChildren = false);
    
    virtual bool hasLock();
    
    virtual NameList getLockBids(bool children = false);

    virtual void remove(bool removeChildren = false);

    /*
     * Internal functions not used by outside clients
     */
  public:
    /**
     * Constructor.
     *
     * Initialize m_refCount to 1 since on creation, it must begin
     * with 1 to allow a releaseRef().  load*() will also increment the
     * m_refCount every time the data is acquired from the cache.
     *
     * Note: Currently, clusterlib does not allow a user to remove an
     * object that has children.  If this changes, one possible
     * solution is to always keep a reference to parent.  This
     * prevents the parent cached representation from being removed
     * from memory which this notifyable is in the cache.  Upon
     * destruction, release that reference to the parent.
     */
    NotifyableImpl(FactoryOps *fp,
                   const std::string &key,
                   const std::string &name,
                   NotifyableImpl *parent)
        : mp_f(fp),
          m_key(key),
          m_name(name),
          mp_parent(parent),
          m_state(Notifyable::READY),
          m_refCount(1) {}

    /**
     * Get the associated factory object.
     */
    FactoryOps *getOps() { return mp_f; }

    /**
     * Destructor.
     *
     */
    virtual ~NotifyableImpl() {}

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
    virtual void removeRepositoryEntries() = 0;

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
    Mutex *getSyncLock() const;

    /**
     * Get the NotifyableImpl synchronization lock for distributed locks
     * 
     * @return pointer to the synchronization lock for distributed locks
     */
    Mutex *getSyncDistLock();

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

  private:
    /*
     * Default constructor.
     */
    NotifyableImpl() 
    {
        throw InvalidMethodException("Someone called the NotifyableImpl "
                                     "default constructor!");
    }

    /**
     * Decrement the reference count of this cached representation of
     * the object.
     */
    void decrRefCount();

    /**
     * Remove the pointer to this NotifyableImpl from the FactoryOps
     * "removed cache" and also free the memory used iff the refcount is 0.
     *
     * @param decrRefCount do the decrement of the ref count as well?
     */
    void removeFromRemovedNotifyablesIfReleased(bool decrRefCount);

    /**
     * Get the mutex that protects the reference count
     * 
     * @return pointer to the mutex
     */
    Mutex *getRefCountLock();

  private:
    /*
     * The associated factory delegate.
     */
    FactoryOps *mp_f;

    /*
     * The key to pass to the factory delegate for
     * operations on the represented cluster node.
     */
    const std::string m_key;

    /*
     * The name of the Notifyable.
     */
    const std::string m_name;

    /**
     * The parent notifyable (NULL if no parent)
     */
    NotifyableImpl *mp_parent;

    /**
     * Is this notifyable "init", "ready", or "deleted" according to the
     * Notifyable state protocol?
     */
    State m_state;
    
    /** 
     * All lock info (locks, their owners and their owner's reference counts are
     * stored here.
     */
    std::map<std::string, NameRef> m_distLockMap;

    /** 
     * Make the reference count (m_refCount) thread-safe
     */
    Mutex m_refCountLock;

    /**
     * Local object reference count.  Once this goes to 0 and m_state
     * == REMOVED, it can be removed from the "removed cache".
     */
    int32_t m_refCount;

    /**
     * Lock to synchronize the Notifyable with the clusterlib event
     * processing thread.  It is mutable since it is used by
     * throwIfRemoved() - which is const so it can be used everywhere.
     */
    mutable Mutex m_syncLock;

    /**
     * Lock to synchronize all clusterlib distributed locks.
     */
    Mutex m_syncDistLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NOTIFYABLEIMPL_H_ */
