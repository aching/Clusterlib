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

#ifndef	_CL_DATADISTRIBUTION_H_
#define _CL_DATADISTRIBUTION_H_

namespace clusterlib {

/**
 * Interface of class DataDistribution; it represents a data
 * distribution (mapping from a key or a HashRange to a notifyable) in
 * clusterlib.
 */
class DataDistribution
    : public virtual Notifyable
{
  public:
    /**
     * Access the cached shards
     *
     * @return A reference to the CachedShards.
     */
    virtual CachedShards &cachedShards() = 0;

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistribution() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_DATADISTRIBUTION_H_ */
