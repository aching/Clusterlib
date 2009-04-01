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

    virtual bool isReady() const { return m_ready; }

    virtual Properties *getProperties(bool create = false);

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
          m_ready(false) {}

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
     * Is this notifyable "ready" according to the ready
     * protocol?
     */
    bool m_ready;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLEIMPL_H_ */
