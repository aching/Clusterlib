/* 
 * datadistributionimpl.cc --
 *
 * Implementation of the DataDistributionImpl class.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#define MODULE_NAME "DataDistribution"

#include <limits>
#include "clusterlibinternal.h"
#include <boost/regex.hpp>

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
