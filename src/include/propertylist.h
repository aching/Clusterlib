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

#ifndef _CL_PROPERTYLIST_H_
#define _CL_PROPERTYLIST_H_

namespace clusterlib {

/**
 * Simple key-value storage object.
 */
class PropertyList
    : public virtual Notifyable
{
  public:
    /**
     * Access the cached key-values
     *
     * @return A reference to the cached key-values.
     */
    virtual CachedKeyValues &cachedKeyValues() = 0;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIB_H_ */

