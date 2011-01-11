/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_CACHEDKEYVALUES_H_
#define _CL_CACHEDKEYVALUES_H_

namespace clusterlib {

/**
 * Represents the cached key-values of a property list.
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
     * @param jsonValue The value of the given propery if found
     * @param searchParent Search the parent (and ancestors) as well?
     * @param ancestorMsecTimeout Msecs to wait on getting locks for ancestors
     * @param pUsedProperyListSP If not NULL, will be filled in with the 
     *        PropertyList where the jsonValue came from.
     * @return True if found, false otherwise
     */
    virtual bool get(
        const std::string &key, 
        json::JSONValue &jsonValue,
        bool searchParent = false,
        int64_t ancestorMsecTimeout = -1,
        boost::shared_ptr<PropertyList> *pUsedProperyListSP = NULL) = 0;

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

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDKEYVALUES_H_ */
