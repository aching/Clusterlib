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

    virtual const std::string &getName() const { return m_name; }

    virtual const std::string &getKey() const { return m_key; }

#if TO_BE_IMPLEMENTED_IF_NECESSARY
    virtual std::string getMyApplicationName() const;

    virtual std::string getMyGroupName() const;
#endif

    virtual Notifyable *getMyParent() const;
    
    virtual Application *getMyApplication(); 

    virtual Group *getMyGroup(); 

    virtual Notifyable::State getState();
    
    virtual Properties *getProperties(bool create = false);

    virtual void acquireLock();

    virtual void releaseLock();

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
          m_state(Notifyable::INIT) {}

    /**
     * Set the state of this notifyable.
     */
    void setState(Notifyable::State state);

    /*
     * Get the associated factory delegate object.
     */
    FactoryOps *getDelegate() { return mp_f; }

    /*
     * Destructor.
     */
    virtual ~NotifyableImpl() {}

    /**
     * Initialize the cached representation when the object is loaded
     * into clusterlib -- must be provided by subclasses.
     */
    virtual void initializeCachedRepresentation() = 0;

    /**
     * Take all actions to remove all backend storage (i.e. Zookeeper data)
     */
    virtual void removeRepositoryEntries() {}

    /**
     * Get the NotifyableImpl lock
     */
    virtual Mutex *getStateLock() { return &m_stateLock; }

    /**
     * Get the key of the distributed lock
     */
    virtual const std::string &getDistributedLockKey() const 
    {
        return m_distLockKey; 
    }

    /**
     * Set the key of the distributed lock
     */
    virtual void setDistributedLockKey(const std::string &distLockKey)
    { 
        m_distLockKey = distLockKey; 
    }
    
  private:
    /*
     * Default constructor.
     */
    NotifyableImpl() 
    {
        throw ClusterException("Someone called the NotifyableImpl "
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
     * Lock to synchronize the Notifyable state
     */
    Mutex m_stateLock;

    /** 
     * The key that represents the holder of the distributed lock of
     * this object.
     */
    std::string m_distLockKey;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLEIMPL_H_ */
