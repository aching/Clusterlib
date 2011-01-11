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

#ifndef	_CL_CACHEDKEYVALUESIMPL_H_
#define _CL_CACHEDKEYVALUESIMPL_H_

namespace clusterlib {

/**
 * Implementation of class CachedKeyValues
 */
class CachedKeyValuesImpl
    : public virtual CachedDataImpl,
      public virtual CachedKeyValues
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    virtual std::vector<json::JSONValue::JSONString> getKeys();

    virtual bool get(
        const std::string &key, 
        json::JSONValue &jsonValue,
        bool searchParent = false,
        int64_t ancestorMsecTimeout = -1,
        boost::shared_ptr<PropertyList> *pUsedProperyListSP = NULL);

    virtual void set(const ::json::JSONValue::JSONString &key, 
                     const ::json::JSONValue &value);

    virtual bool erase(const ::json::JSONValue::JSONString &key);

    virtual void clear();

    /**
     * Constructor.
     */
    explicit CachedKeyValuesImpl(NotifyableImpl *pNotifyable);

    /**
     * Destructor.
     */
    virtual ~CachedKeyValuesImpl() {}
    
  private:
    /**
     * The key values state in user-defined format.
     */
    ::json::JSONValue::JSONObject m_keyValues;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDKEYVALUESIMPL_H_ */
