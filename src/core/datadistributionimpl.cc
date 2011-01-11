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

#include "clusterlibinternal.h"

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib {

CachedShards &
DataDistributionImpl::cachedShards()
{
    return m_cachedShards;
}

DataDistributionImpl::DataDistributionImpl(
    FactoryOps *fp,
    const string &key,
    const string &name,
    const shared_ptr<NotifyableImpl> &parentGroup)
    : NotifyableImpl(fp, key, name, parentGroup),
      m_cachedShards(this)
{
    TRACE(CL_LOG, "DataDistributionImpl");
}

NotifyableList
DataDistributionImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");

    throwIfRemoved();
    
    return NotifyableList();
}

void
DataDistributionImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Initialize the shards.
     */
    m_cachedShards.loadDataFromRepository(false);
}

string
DataDistributionImpl::createShardJsonObjectKey(
    const string &dataDistributionKey)
{
    string res;
    res.append(dataDistributionKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::SHARD_JSON_OBJECT);

    return res;
}

DataDistributionImpl::~DataDistributionImpl()
{
}

}       /* End of 'namespace clusterlib' */
