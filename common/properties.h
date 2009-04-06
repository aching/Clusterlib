/*
 * properties.h --
 *
 * Interface of class Properties; it represents the properties of a
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
     * \brief Get the keys of all the properties.
     * 
     * This function is safe to call without a lock as it acquires the
     * lock while getting the property keys and returns them as a
     * vector.
     *
     * @return the vector of property keys
     */
    virtual std::vector<std::string> getPropertyKeys() const = 0;
         
    /**
     * \brief Gets a value associated with the given property.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data while it is being accessed.  If the calling process is
     * only reading, this procedure will implicitly hold the lock to
     * provide consistent results to the user.
     *
     * @param name the property name
     * @param searchParent try the parent for the property as well?
     * @return the value of the given propery or an empty string
     */
    virtual std::string getProperty(const std::string &name, 
                                    bool searchParent = false) = 0;
        
    /**
     * \brief Sets value of the given property.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data.
     *
     * @param name the property name for which to set value
     * @param value the value to be set
     */
    virtual void setProperty(const std::string &name, 
                             const std::string &value) = 0;

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
    virtual void deleteProperty(const std::string &name) = 0;

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
    virtual void publish() = 0;

    /**
     * \brief Resets the property list to empty (acquires/releases lock).
     *
     */
    virtual void reset() = 0;

    /**
     * \brief Return the time at which the last value change happened.
     *
     * @return the int64 representing the time that the value changed.
     */
    virtual int64_t getValueChangeTime() = 0;

    /**
     * \brief Do not allow getProperties() on a Properties object (throw)
     */
    virtual Properties *getProperties(bool create = false) = 0;    
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIB_H_ */

