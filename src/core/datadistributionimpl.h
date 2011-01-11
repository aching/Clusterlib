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

#ifndef	_CL_DATADISTRIBUTIONIMPL_H_
#define _CL_DATADISTRIBUTIONIMPL_H_

namespace clusterlib {

/**
 * Implements class DataDistribution.
 */
class DataDistributionImpl
    : public virtual DataDistribution, 
      public virtual NotifyableImpl
{
  public:
    virtual CachedShards &cachedShards();

  public:
    /**
     * Constructor used by Factory.
     */
    DataDistributionImpl(FactoryOps *fp,
                         const std::string &key,
                         const std::string &name,
                         const boost::shared_ptr<NotifyableImpl> &parentGroup);

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistributionImpl();

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

    /**
     * Create the shard JSONObject key
     *
     * @param dataDistributionKey the data distribution key
     * @return the generated shard JSONObject key
     */
    static std::string createShardJsonObjectKey(
        const std::string &dataDistributionKey);

  private:
    /**
     * Do not call the default constructor
     */
    DataDistributionImpl();
    
  private:
    /**
     * The cached shards.
     */
    CachedShardsImpl m_cachedShards;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_DATADISTRIBUTIONIMPL_H_ */
