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

#ifndef	_CL_CACHEDPROCESSINFOIMPL_H_
#define _CL_CACHEDPROCESSINFOIMPL_H_

namespace clusterlib {

/**
 * Implements class CachedProcessInfo
 */
class CachedProcessInfoImpl
    : public virtual CachedDataImpl,
      public virtual CachedProcessInfo
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    virtual json::JSONValue::JSONArray getHostnameArr();

    virtual void setHostnameArr(const json::JSONValue::JSONArray &hostnameArr);

    virtual json::JSONValue::JSONArray getPortArr();

    virtual void setPortArr(const json::JSONValue::JSONArray &portArr);

    /**
     * Constructor.
     */
    explicit CachedProcessInfoImpl(NotifyableImpl *notifyable);

    /**
     * Destructor.
     */
    virtual ~CachedProcessInfoImpl() {}
    
  private:
    /**
     * Marshal the m_hostnameArr and m_portArr into a JSONValue for publishing.
     *
     * @return An JSON array of the two arrays.
     */
    json::JSONValue::JSONArray marshal();

    /**
     * Unmarshal the two arrays into this object
     *
     * @param encodedJsonArr The encoded JSON array of two arrays
     */
    void unmarshal(const std::string &encodedJsonArr);

  private:
    /**
     * The hostname array stored as a JSONArray
     */
    ::json::JSONValue::JSONArray m_hostnameArr;

    /**
     * The port array stored as a JSONArray
     */
    ::json::JSONValue::JSONArray m_portArr;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSINFOIMPL_H_ */
