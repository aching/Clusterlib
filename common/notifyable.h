/*
 * notifyable.h --
 *
 * Contains the base class for notifyable objects.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_NOTIFYABLE_H_
#define _NOTIFYABLE_H_

using namespace std;

namespace clusterlib
{

/*
 * Interface that must be derived by specific notifyable objects.
 */
class Notifyable
{
  public:
    /*
     * Compare two Notifyable instances.
     */
    bool operator==(const Notifyable &other)
    {
        return (other.getKey() == getKey()) ? true : false;
    }

    /**
     * Get the name of the Notifyable.
     * 
     * @return name of the Notifyable
     */
    const string &getName() const { return m_name; }

    /**
     * Return the string identifying the represented
     * cluster object.
     *
     * @return key unique key that represents this Notifyable
     */
    const string &getKey() const { return m_key; }

#if TO_BE_IMPLEMENTED_IF_NECESSARY
    /**
     * Get the application name of this notifyable
     *
     * @return application this notifyable belongs to
     */
    string getMyApplicationName() const;

    /**
     * Get the group name of this notifyable
     *
     * @return group this notifyable belongs to
     */
    string getMyGroupName() const;

#endif
    /**
     * Get the parent of this Notifyable (if it exists)
     *
     * @return pointer to parent or NULL if this is an Application
     */
    Notifyable *getMyParent() const;
    
    /**
     * Retrieve the application object that this Notifyable is a part of.  
     *
     * @return pointer to the Application or NULL if it doesn't exist
     */
    Application *getMyApplication(); 

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
    bool isReady() const { return m_ready; }

    /**
     * Get the properties for this object (if it is allowed). If
     * subclasses do not want to allow getProperties(), override it
     * and throw a clusterlib exception.
     * 
     * @param create create the properties if doesn't exist?
     * @return NULL if no properties exists for this notifyable
     */
    virtual Properties *getProperties(bool create = false);

  protected:
    /*
     * Factory is a friend so it can call the below constructor.
     */
    friend class Factory;

    /*
     * Constructor.
     */
    Notifyable(FactoryOps *fp,
               const string &key,
               const string &name,
               Notifyable *parent);

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
    virtual ~Notifyable();

    /*
     * Update the cached representation -- must be provided
     * by subclasses.
     */
    virtual void updateCachedRepresentation()
        = 0;

    /*
     * Get locks associated with the various maps.
     */
    Mutex *getPropertiesMapLock() { return &m_propLock; }

  private:
    /*
     * Default constructor.
     */
    Notifyable() 
    {
        throw ClusterException("Someone called the Notifyable "
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
    const string m_key;

    /*
     * The name of the Notifyable.
     */
    const string m_name;

    /**
     * The parent notifyable (NULL if no parent)
     */
    Notifyable *mp_parent;

    /*
     * Is this notifyable "ready" according to the ready
     * protocol?
     */
    bool m_ready;

    /*
     * Map of all properties within this object.
     */
    PropertiesMap m_properties;
    Mutex m_propLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
