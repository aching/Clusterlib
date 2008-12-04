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
     * \brief Sets value of the given property.
     * 
     * @param name the property name for which to set value
     * @param value the value to be set
     */
    void setProperties(const vector<string> &keys, 
		       const vector<string> &values);
    
    /**
     * \brief Gets a value associated with the given property.
     * 
     * @param name the property name
     * @return the value of the given propery or an empty string
     */
    string getProperty(const string &name);
        
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
        
    /**
     * \brief Resets the property list to empty.
     */
    void reset() 
    {
	m_keyValMap.clear();
    }
    
    int32_t getKeyValVersion() { return m_keyValMapVersion; }
    void setKeyValVersion(int32_t version) { m_keyValMapVersion = version; }


    Mutex *getKeyValMapLock() { return &m_keyValMapMutex; }

  protected:
	    /*
     * Friend declaration for Factory so that it will be able
     * to call the constructor.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    Properties(const string &key,
	       FactoryOps *f)
        : Notifyable(f, key, ""),
	  m_keyValMapVersion(-2)
    {
	updateCachedRepresentation();
    }

    /*
     * Update the cached representation of this node.
     */
    virtual void updateCachedRepresentation();

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

