/*
 * cachedkeyvalues.h --
 *
 * Definition of class CachedKeyValues; it represents the cached
 * key-values of a property list.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDKEYVALUES_H_
#define _CL_CACHEDKEYVALUES_H_

namespace clusterlib
{

/**
 * Definition of class CachedKeyValues
 */
class CachedKeyValues
    : public virtual CachedData
{
  public:
    /**
     * \brief Get the keys in the property list.
     * 
     * This function is safe to call without a lock as it acquires the
     * lock while getting the property list keys and returns them as a
     * vector.
     *
     * @return the vector of property list keys
     */
    virtual std::vector<json::JSONValue::JSONString> getKeys() = 0;

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
     * @param key the key
     * @param The value of the given propery if found
     * @param searchParent Try the parent for the property as well?
     * @return True if found, false otherwise
     */
    virtual bool get(const std::string &key, 
                     json::JSONValue &jsonValue,
                     bool searchParent = false,
                     PropertyList **propertyListWithKey = NULL) = 0;

    /**
     * \brief Sets value of the given key.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data.
     *
     * @param key the key
     * @param value the value to be set
     */
    virtual void set(const ::json::JSONValue::JSONString &key, 
                     const ::json::JSONValue &value) = 0;

    /**
     * \brief Deletes the key-value.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data.  Until publish() is called, this change is only local.
     *
     * @param key The key to be erased.
     * @return Returns true if successful, otherwise false.
     */
    virtual bool erase(const ::json::JSONValue::JSONString &key) = 0;

    /**
     * Clears all the key-value pairs from the local cache.  Until
     * publish() is called, this change is only local.
     */
    virtual void clear() = 0;

    /**
     * Destructor.
     */
    virtual ~CachedKeyValues() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDKEYVALUES_H_ */
