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

/*
 * Interface that must be derived by specific notifyable objects.
 */
class NotifyableImpl
    : public virtual Notifyable
{
  public:
    /*
     * Compare two Notifyable instances.
     */
    virtual bool operator==(const Notifyable &other)
    {
        return (other.getKey() == getKey()) ? true : false;
    }

    /**
     * Get the name of the Notifyable.
     * 
     * @return name of the Notifyable
     */
    virtual const std::string &getName() const { return m_name; }

    /**
     * Return the string identifying the represented
     * cluster object.
     *
     * @return key unique key that represents this Notifyable
     */
    virtual const std::string &getKey() const { return m_key; }

#if TO_BE_IMPLEMENTED_IF_NECESSARY
    /**
     * Get the application name of this notifyable
     *
     * @return application this notifyable belongs to
     */
    virtual std::string getMyApplicationName() const;

    /**
     * Get the group name of this notifyable
     *
     * @return group this notifyable belongs to
     */
    virtual std::string getMyGroupName() const;
#endif

    /**
     * Get the parent of this Notifyable (if it exists)
     *
     * @return pointer to parent or NULL if this is an Application
     */
    virtual Notifyable *getMyParent() const;
    
    /**
     * Retrieve the application object that this Notifyable is a part of.  
     *
     * @return pointer to the Application or NULL if it doesn't exist
     */
    virtual Application *getMyApplication(); 

    /**
     * Retrieve the group object that this Notifyable is a part of.
     * If subclasses do not want to allow getMyGroup(), override it
     * and throw a clusterlib exception.
     *
     * @return pointer to the Group or NULL if it doesn't exist
     */
    virtual Group *getMyGroup(); 

    /**
     * Is this notifyable "ready"? (according to the
     * ready protocol)
     *
     * @return true if this Notifyable is ready for use
     */
    virtual bool isReady() const { return m_ready; }

    /**
     * Get the properties for this object (if it is allowed). If
     * subclasses do not want to allow getProperties(), override it
     * and throw a clusterlib exception.
     * 
     * @param create create the properties if doesn't exist?
     * @return NULL if no properties exists for this notifyable
     */
    virtual Properties *getProperties(bool create = false);

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
          mp_myGroup(NULL),
          mp_myApplication(NULL),
          mp_myProperties(NULL),
          m_ready(false)
    {
    }

    /*
     * Set the "ready" state of this notifyable.
     */
    void setReady(bool v) { m_ready = v; }

    /*
     * Get the associated factory delegate object.
     */
    FactoryOps *getDelegate() { return mp_f; }

    /*
     * Destructor.
     */
    virtual ~NotifyableImpl() {}

    /*
     * Initialize the cached representation -- must be provided
     * by subclasses.
     */
    virtual void initializeCachedRepresentation() = 0;

    /*
     * Get lock associated with cached information.
     */
    Mutex *getChainLock() { return &m_chainLock; }

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

    /*
     * The group this notifyable is in (if any).
     */
    Group *mp_myGroup;

    /*
     * The application this notifyable is in.
     */
    Application *mp_myApplication;

    /*
     * The properties list for this object.
     */
    Properties *mp_myProperties;

    /*
     * Lock for protecting mp_myApplication
     * and mp_myGroup.
     */
    Mutex m_chainLock;

    /*
     * Is this notifyable "ready" according to the ready
     * protocol?
     */
    bool m_ready;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLEIMPL_H_ */
