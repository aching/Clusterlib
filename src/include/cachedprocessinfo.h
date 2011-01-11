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

#ifndef	_CL_CACHEDPROCESSINFO_H_
#define _CL_CACHEDPROCESSINFO_H_

namespace clusterlib {

/**
 * Process metadata of a ProcessSlot.
 */
class CachedProcessInfo
    : public virtual CachedData
{
  public:
    /**
     * Get a vector of hostnames (user-defined).  They are of type
     * JSONValue::JSONString.  If not defined, will be empty.
     */
    virtual json::JSONValue::JSONArray getHostnameArr() = 0;

    /**
     * Set a vector of hostnames (user-defined) as a JSONValue.
     *
     * @param hostnameArr The JSONArray of hostnames to set as a JSONValue
     */
    virtual void setHostnameArr(
        const json::JSONValue::JSONArray &hostnameArr) = 0;

    /**
     * Get a vector of ports (user-defined).  They are of type
     * JSONValue::JSONInteger.  If not defined, will be empty.
     *
     * @return the vector of ports
     */
    virtual json::JSONValue::JSONArray getPortArr() = 0;

    /**
     * Set a vector of ports (user-defined) as a JSONValue.
     *
     * @param portArr The JSONArray of ports to set as a JSONValue
     */
    virtual void setPortArr(const json::JSONValue::JSONArray &portArr) = 0;
 
    /**
     * Destructor.
     */
    virtual ~CachedProcessInfo() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSINFO_H_ */
