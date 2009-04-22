/*
 * notifyableimpl.h --
 *
 * Contains the implementation of the notifyable interface.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_NOTIFYABLEIMPL_H_
#define _NOTIFYABLEIMPL_H_

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
    
    virtual Application *getMyApplication(); 

    virtual Group *getMyGroup(); 

    virtual Notifyable::State getState() const;
    
    virtual Properties *getProperties(bool create = false);

    virtual void acquireLock(bool acquireChildren = false);

    virtual void releaseLock(bool releaseChildren = false);
    
    virtual bool hasLock();
    
    virtual void remove(bool removeChildren = false);

    /*
     * Internal functions not used by outside clients
     */
  public:
    /*
     * Constructor.
     */
    NotifyableImpl(FactoryOps *fp,
                   const std::string &key,
                   const std::string &name,
                   NotifyableImpl *parent)
        : mp_f(fp),
          m_key(key),
          m_name(name),
          mp_parent(parent),
          m_state(Notifyable::READY) {}

    /*
     * Get the associated factory object.
     */
    FactoryOps *getOps() { return mp_f; }

    /*
     * Destructor.
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
    virtual int32_t incrDistributedLockOwnerCount(const std::string &lockName);
    
    /**
     * Safe reference count changes for distributed locks.
     *
     * @param lockName the name of the lock
     * @return the reference count after the operation
     */
    virtual int32_t decrDistributedLockOwnerCount(const std::string &lockName);

    /**
     * Get the number of references on the current distributed lock key
     *
     * @param lockName the name of the lock
     */
    virtual int32_t getDistributedLockOwnerCount(const std::string &lockName);

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
     * Get the NotifyableImpl state lock
     */
    virtual Mutex *getStateLock() const { return &m_stateLock; }

    /**
     * Set the state of the NotifyableImpl.  Client must hold
     * m_stateLock to ensure atomicity.
     */
    void setState(NotifyableImpl::State state)
    {
        m_state = state;
    }

  private:
    /*
     * Default constructor.
     */
    NotifyableImpl() 
    {
        throw InvalidMethodException("Someone called the NotifyableImpl "
                                     "default constructor!");
    }

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
     * All locks, their owners and their owner's reference counts are
     * stored here.
     */
    std::map<std::string, NameRef> m_distLockMap;

    /**
     * Lock to synchronize the Notifyable state (includes distibuted
     * lock key and count).  It is mutable since it is used by
     * throwIfRemoved() - which is const so it can be used everywhere.
     */
    mutable Mutex m_stateLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLEIMPL_H_ */
