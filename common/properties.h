/*
 * properties.h --
 *
 * Definition of class Properties; it represents a properties of a
 * clusterlib object.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */


#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

namespace clusterlib
{

/**
 * Definition of class Properties
 */
class Properties 
    : public virtual Notifyable
{
    public:
    /** 
     * \brief Get the property lock.
     *
     * Guarantees that the this object's members will not be modified by
     * any process that does not own the lock (including the internal
     * clusterlib event system).
     */
    void acquireLock()
    {
	getKeyValMapLock()->Acquire();
    }

    /** 
     * \brief Releases the property lock.
     *
     * Releases the lock so that other processes that do not own the
     * lock (including the internal clusterlib event system).
     */
    void releaseLock()
    {
	getKeyValMapLock()->Release();
    }

    /**
     * \brief Get the keys of all the properties.
     * 
     * This function is safe to call without a lock as it acquires the
     * lock while getting the property keys and returns them as a
     * vector.
     *
     * @return the vector of property keys
     */
    vector<string> getPropertyKeys() const;
         
    /**
     * \brief Gets a value associated with the given property.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data while it is being accessed.  If the calling process is
     * only reading, this procedure will implicitly hold the lock to
     * provide consistent results to the user.  If searchParent is
     * true, there are not guarantees provided to the user about the
     * consistency between properties fetched from various higher
     * levels in the hierarchy.
     *
     * @param name the property name
     * @param searchParent try the parent for the property as well?
     * @return the value of the given propery or an empty string
     */
    string getProperty(const string &name, bool searchParent = false);
        
    /**
     * \brief Sets value of the given property.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data.  Until publish() is called, this change is only local.
     *
     * @param name the property name for which to set value
     * @param value the value to be set
     */
    void setProperty(const string &name, 
		     const string &value);

    /**
     * \brief Deletes the property name.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data.  Until publish() is called, this change is only local.
     *
     * @param name the property name to be deleted
     */
    void deleteProperty(const string &name);

    /**
     * \brief Push the key-value map to the storage.
     * 
     * Changes made through setProperty are not seen by other clients
     * unless they are published.  It is possible that an exception
     * from clusterlib may be thrown if the versions don't match.  In
     * this case, the user should catch the exception, release the
     * lock and wait until the properties are updated (either through
     * polling or waiting on events).  Then they should try to set
     * their properties again under a lock and publish again.
     */
    void publish();

    /**
     * \brief Resets the property list to empty (acquires/releases lock).
     *
     */
    void reset() 
    {
	acquireLock();
	m_keyValMap.clear();
	releaseLock();
    }

    /**
     * \brief Do not allow getProperties() on a Properties object (throw)
     */
    virtual Properties *getProperties(bool create = false);
    
  protected:
    /*
     * Friend declaration for Factory so that it will be able
     * to call the constructor.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    Properties(Notifyable *parent, 
               const string &key,
	       FactoryOps *fp)
        : Notifyable(fp, key, "", parent),
	  m_keyValMapVersion(-2)
    {
    }

    /**
     * Update the cached representation of this node.
     * 
     * \brief The clusterlib event system uses this to update the
     * m_keyValMap when it changes.
     *
     * The appropriate handler calls this when it notices that
     * Properties has changed.
     */
    virtual void updateCachedRepresentation();

    /**
     * \brief Converts this properties map into a string.
     * 
     * @return the string representation of this object
     */
    string marshall() const;
        
    /**
     * \brief Unmarshalls the given properties into this object.
     * This method will effectively override existing
     * properties unless there is a parse error.
     * 
     * @return whether the method successfully unmarshalled properties 
     *         or not
     */
    bool unmarshall(const string &propsAsStr);

    int32_t getKeyValVersion() { return m_keyValMapVersion; }

    void setKeyValVersion(int32_t version) { m_keyValMapVersion = version; }

    Mutex *getKeyValMapLock() { return &m_keyValMapMutex; }

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    Properties()
        : Notifyable(NULL, "", "", NULL)
    {
        throw ClusterException("Someone called the Properties default "
                               "constructor!");
    }

    /*
     * Make the destructor private also.
     */
    virtual ~Properties() {}

  private:
    /**                                                                        
     * The maps of properties.                                                 
     */
    KeyValMap m_keyValMap;
    int32_t m_keyValMapVersion;
    Mutex m_keyValMapMutex;

};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIB_H_ */

