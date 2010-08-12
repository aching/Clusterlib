/*
 * cachedstate.h --
 *
 * Definition of class CachedState; it represents the cached
 * key-values of a property list.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDSTATE_H_
#define _CL_CACHEDSTATE_H_

namespace clusterlib {

/**
 * Definition of class CachedState.  Conceptually, CachedState is a
 * JSONArray that keeps track of the last X states.
 */
class CachedState
    : public virtual CachedData
{
  public:
    /**
     * Get the maximum number of states to keep.  The default is 5.
     * This data is not actually stored in the repository, it is a
     * local variable that is used when publishing the data to the
     * repository.
     *
     * @return The maximum number of states that will be kept when published.
     */
    virtual int32_t getMaxHistorySizePublished() = 0;

    /**
     * Set the maximum number of states to keep.  This data is not
     * actually stored in the repository, it is a local variable that
     * is used when publishing the data to the repository.  The default is 5.
     *
     * @param maxHistorySize The maximum number of states that will be kept
     *        after publish().
     */
    virtual void setMaxHistorySizePublished(int32_t maxHistorySize) = 0;

    /**
     * Get the number of historical states stored in this object.
     *
     * @return The number states available to access.
     */
    virtual int32_t getHistorySize() = 0;

    /**
     * \brief Get the keys available in the a historical state.
     * 
     * This function is safe to call without a lock as it acquires the
     * lock while getting the property list keys and returns them as a
     * vector.
     *
     * @param historyIndex the state to get the keys for
     * @return the vector of keys
     */
    virtual std::vector<json::JSONValue::JSONString> getHistoryKeys(
        int32_t historyIndex) = 0;

    /**
     * \brief Gets a value associated with a key in a particular
     * historical state.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data while it is being accessed.  If the calling process is
     * only reading, this procedure will implicitly hold the lock to
     * provide consistent results to the user.
     *
     * @param historyIndex the state to get the keys for
     * @param key the key
     * @param jsonValue The value of the given propery if found
     * @return True if found, false otherwise
     */
    virtual bool getHistory(int32_t historyIndex,
                            const std::string &key, 
                            json::JSONValue &jsonValue) = 0;

    /**
     * \brief Gets a value associated with a key in the current state.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data while it is being accessed.  If the calling process is
     * only reading, this procedure will implicitly hold the lock to
     * provide consistent results to the user.
     *
     * @param key the key
     * @param jsonValue The value of the given propery if found
     * @return True if found, false otherwise
     */
    virtual bool get(const std::string &key, 
                     json::JSONValue &jsonValue) = 0;

    /**
     * \brief Sets value of the given key for the current state.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data.  If addState() was not called, this function will throw.
     *
     * @param key the key
     * @param value the value to be set
     */
    virtual void set(const ::json::JSONValue::JSONString &key, 
                     const ::json::JSONValue &value) = 0;

    /**
     * \brief Deletes the key-value in the next state.
     * 
     * In most cases, the calling process should hold the lock prior
     * to calling this function to prevent another process or the
     * internal clusterlib events sytem from modifying this object
     * data.  Until publish() is called, this change is only local.
     * If addState() was not called, this function will throw.
     *
     * @param key The key to be erased.
     * @return Returns true if successful, otherwise false.
     */
    virtual bool erase(const ::json::JSONValue::JSONString &key) = 0;

    /**
     * Clears all the key-value pairs from the local cache.  Until
     * publish() is called, this change is only local.  This only
     * affects the next state.
     */
    virtual void clear() = 0;

    /**
     * Get all the historical states as a single JSONArray (current changes).
     * 
     * @return All states as a single JSONArray.
     */
    virtual json::JSONValue::JSONArray getHistoryArray() = 0;

    /**
     * Destructor.
     */
    virtual ~CachedState() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDSTATE_H_ */
