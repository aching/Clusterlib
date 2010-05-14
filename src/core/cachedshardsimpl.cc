/*
 * cachedshardsimpl.cc --
 *
 * Implementation of the CachedShardsImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace json;

namespace clusterlib
{

/** 
 * Given 2 shards, compare them based on priority.  Used to sort shard
 * containers.
 *
 * @param a the first shard
 * @param b the second shard
 * @return true if a < b
 */
static bool shardPriorityCompare(Shard a, Shard b)
{
    if (a.getPriority() < b.getPriority()) {
        return true;
    }
    else {
        return false;
    }
}

CachedShardsImpl::CachedShardsImpl(NotifyableImpl *ntp)
    : CachedDataImpl(ntp),
      m_shardTreeCount(0)
{
}

CachedShardsImpl::~CachedShardsImpl()
{
    Locker l(&getCachedDataLock());

    /*
     * Cannot simply call "clear()" since the repository object may have
     * been removed.
     */
    while (m_shardTree.empty() == false) {
        m_shardTree.deleteNode(m_shardTree.getTreeHead());
    }
    m_shardTreeCount = 0;
}

int32_t
CachedShardsImpl::publish(bool unconditional)
{
    TRACE(CL_LOG, "publish");

    getNotifyable()->throwIfRemoved();

    string shardsKey = DataDistributionImpl::createShardJsonObjectKey(
        getNotifyable()->getKey());

    Locker l(&getCachedDataLock());

    string encodedJsonObject = JSONCodec::encode(marshallShards());
    LOG_INFO(CL_LOG,
             "Tried to publish shards for notifyable %s to %s "
             "with current version %d, unconditional %d\n",
             getNotifyable()->getKey().c_str(), 
             encodedJsonObject.c_str(),
             getVersion(),
             unconditional);

    Stat stat;
    try {
        SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                         shardsKey,
                         encodedJsonObject,
                         ((unconditional == false) ? getVersion(): -1),
                         &stat),
                     "Setting of %s failed: %s",
                     shardsKey.c_str(),
                     false,
                     true);
    } catch (const zk::BadVersionException &e) {
        throw PublishVersionException(e.what());
    }
    
    /* 
     * Since we should have the lock, the data should be identical to
     * the zk data.  When the lock is released, clusterlib events will
     * try to push this change again.  
     */
    setStat(stat);
    return stat.version;
}

void
CachedShardsImpl::loadDataFromRepository(bool setWatchesOnly)
{
    TRACE(CL_LOG, "loadDataFromRepository");
    
    string shardsKey = DataDistributionImpl::createShardJsonObjectKey(
        getNotifyable()->getKey());
    string encodedJsonValue;
    Stat stat;

    Locker l(&getCachedDataLock());

    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            shardsKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::SHARDS_CHANGE),
            &stat),
        getOps()->getRepository()->getNodeData(
            shardsKey, encodedJsonValue, NULL, NULL, &stat),
        CachedObjectChangeHandlers::SHARDS_CHANGE,
        shardsKey,
        "Loading shardsKey %s failed: %s",
        shardsKey.c_str(),
        false,
        true);

    if (setWatchesOnly) {
        return;
    }

    if (!updateStat(stat)) {
        return;
    }

    /* 
     * Default values from the constructor are used when there are
     * empty nodes 
     */
    if (encodedJsonValue.empty()) {
        return;
    }

    unmarshallShards(encodedJsonValue);
}

vector<Notifyable *> 
CachedShardsImpl::getNotifyables(const Key &key)
{
    TRACE(CL_LOG, "getNotifyables");

    getNotifyable()->throwIfRemoved();

    /* This is a slow implementation, will be optimized later. */
    vector<Shard> shardVec = getAllShards(NULL, -1);
    /* Sort the shard by priority */
    sort(shardVec.begin(), shardVec.end(), shardPriorityCompare);

    vector<Shard>::iterator shardVecIt;
    vector<Notifyable *> ntpVec;
    for (shardVecIt = shardVec.begin(); 
         shardVecIt != shardVec.end(); 
         ++shardVecIt) {
        if ((shardVecIt->getStartRange() <= key.hashKey()) &&
            (shardVecIt->getEndRange() >= key.hashKey())) {
            ntpVec.push_back(shardVecIt->getNotifyable());
        }
    }

    return ntpVec;
}

vector<Notifyable *> 
CachedShardsImpl::getNotifyables(HashRange hashedKey)
{
    TRACE(CL_LOG, "getNotifyables");

    getNotifyable()->throwIfRemoved();

    /* This is a slow implementation, will be optimized later. */
    vector<Shard> shardVec = getAllShards(NULL, -1);
    /* Sort the shard by priority */
    sort(shardVec.begin(), shardVec.end(), shardPriorityCompare);

    vector<Shard>::iterator shardVecIt;
    vector<Notifyable *> ntpVec;
    for (shardVecIt = shardVec.begin(); shardVecIt != shardVec.end(); 
         shardVecIt++) {
        if ((shardVecIt->getStartRange() <= hashedKey) &&
            (shardVecIt->getEndRange() >= hashedKey)) {
            ntpVec.push_back(shardVecIt->getNotifyable());
        }
    }

    return ntpVec;
}

uint32_t
CachedShardsImpl::getCount()
{
    TRACE(CL_LOG, "getCount");

    getNotifyable()->throwIfRemoved();

    Locker l(&getCachedDataLock());
    return m_shardTreeCount;
}

bool
CachedShardsImpl::isCovered()
{
    TRACE(CL_LOG, "isCovered");

    getNotifyable()->throwIfRemoved();

    Locker l(&getCachedDataLock());

    IntervalTree<HashRange, ShardTreeData>::iterator it;
    HashRange end = 0;
    for (it = m_shardTree.begin(); it != m_shardTree.end(); it++) {
        LOG_DEBUG(CL_LOG, "isCovered: end=%" PRIu64 " sr=%" PRIu64 " er=%"
                  PRIu64, 
                  end, it->getStartRange(), it->getEndRange());
        if (it->getStartRange() > end) {
            return false;
        }
        else if (it->getEndRange() >= end) {
            if (it->getEndRange() == numeric_limits<HashRange>::max()) {
                return true;
            }
            else {
                end = it->getEndRange() + 1;
            }
        }
    }

    return false;
}

vector<HashRange> 
CachedShardsImpl::splitHashRange(int32_t numShards)
{
    TRACE(CL_LOG, "splitHashRange");

    HashRange shardSize = numeric_limits<HashRange>::max() / numShards;
    vector<HashRange> resVec;
    resVec.resize(numShards);
    for (int32_t i = 0; i < numShards; i++) {
        resVec[i] = i*shardSize;
    }
    
    return resVec;
}

void 
CachedShardsImpl::insert(HashRange start,
                         HashRange end,
                         Notifyable *ntp,
                         int32_t priority) 
{
    TRACE(CL_LOG, "insert");

    getNotifyable()->throwIfRemoved();
    
    Locker l(&getCachedDataLock());

    m_shardTree.insertNode(start, end, ShardTreeData(priority, ntp));
    ++m_shardTreeCount;
}

vector<Shard> 
CachedShardsImpl::getAllShards(const Notifyable *ntp, int32_t priority)
{
    TRACE(CL_LOG, "getAllShards");

    getNotifyable()->throwIfRemoved();

    /* 
     * Get all the shards and then filter based on the notifyable and/or
     * priority.
     */
    vector<Shard> res;
    IntervalTree<HashRange, ShardTreeData>::iterator treeIt;

    Locker l(&getCachedDataLock());
    for (treeIt = m_shardTree.begin(); treeIt != m_shardTree.end(); ++treeIt) {
        res.push_back(Shard(treeIt->getStartRange(),
                            treeIt->getEndRange(),
                            treeIt->getData().getNotifyable(),
                            treeIt->getData().getPriority()));
    }
    vector<Shard>::iterator resIt;
    vector<Shard> finalRes;
    for (resIt = res.begin(); resIt != res.end(); resIt++) {
        /* Filter by notifyable if not NULL */
        if ((ntp == NULL) || (ntp == resIt->getNotifyable())) {
            finalRes.push_back(*resIt);
        }

        /* Filter by priority if not -1 */
        if ((priority != -1) || (priority == resIt->getPriority())) {
            finalRes.push_back(*resIt);
        }
    }

    return finalRes;
}

bool
CachedShardsImpl::remove(Shard &shard)
{
    TRACE(CL_LOG, "remove");

    getNotifyable()->throwIfRemoved();

    Locker l(&getCachedDataLock());

    IntervalTreeNode<HashRange, ShardTreeData> *treeNtp = 
        m_shardTree.nodeSearch(shard.getStartRange(),
                               shard.getEndRange(),
                               ShardTreeData(shard.getPriority(),
                                             shard.getNotifyable()));
    if (treeNtp != NULL) {
        m_shardTree.deleteNode(treeNtp);
        m_shardTreeCount--;
        return true;
    }
    
    return false;
}

void
CachedShardsImpl::clear()
{
    TRACE(CL_LOG, "clear");
    
    getNotifyable()->throwIfRemoved();
    
    Locker l(&getCachedDataLock());
    
    while (m_shardTree.empty() == false) {
        m_shardTree.deleteNode(m_shardTree.getTreeHead());
    }
    m_shardTreeCount = 0;
}

JSONValue::JSONArray
CachedShardsImpl::marshallShards()
{
    TRACE(CL_LOG, "marshallShards");

    getNotifyable()->throwIfRemoved();

    Locker l(&getCachedDataLock());

    IntervalTree<HashRange, ShardTreeData>::iterator it;
    JSONValue::JSONArray shardArr;
    JSONValue::JSONArray shardMetadataArr;
    for (it = m_shardTree.begin(); it != m_shardTree.end(); it++) {
        shardMetadataArr.clear();
        shardMetadataArr.push_back(it->getStartRange());
        shardMetadataArr.push_back(it->getEndRange());
        if (it->getData().getNotifyable() == NULL) {
            shardMetadataArr.push_back(string());
        }
        else {
            shardMetadataArr.push_back(
                it->getData().getNotifyable()->getKey());
        }
        shardMetadataArr.push_back(it->getData().getPriority());
        shardArr.push_back(shardMetadataArr);
    }

    LOG_DEBUG(CL_LOG, 
              "marshallShards: Generated string (%s)", 
              JSONCodec::encode(shardArr).c_str());

    return shardArr;
}

void
CachedShardsImpl::unmarshallShards(const string &encodedJsonArr)
{
    TRACE(CL_LOG, "unmarshallShards");

    getNotifyable()->throwIfRemoved();

    vector<string> components;
    vector<string> shardComponents;
    vector<string>::iterator sIt;

    LOG_DEBUG(CL_LOG, 
              "unmarshallShards: Got encodedJsonArr '%s'", 
              encodedJsonArr.c_str());

    Locker l(&getCachedDataLock());

    clear();
    m_shardTreeCount = 0;

    if (encodedJsonArr.empty()) {
        return;
    }

    JSONValue jsonValue = JSONCodec::decode(encodedJsonArr);
    if (jsonValue.type() == typeid(JSONValue::JSONNull)) {
        return;
    }
    
    JSONValue::JSONArray shardArr = jsonValue.get<JSONValue::JSONArray>();
    JSONValue::JSONArray shardMetadataArr;    
    JSONValue::JSONArray::const_iterator shardArrIt;
    Notifyable *ntp = NULL;
    for (shardArrIt = shardArr.begin(); 
         shardArrIt != shardArr.end(); 
         ++shardArrIt) {
        shardMetadataArr.clear();
        shardMetadataArr = shardArrIt->get<JSONValue::JSONArray>();
        if (shardMetadataArr.size() != 4) {
            throw InconsistentInternalStateException(
                "unmarshallShards: Impossible that the size of the "
                "shardMetadataArr != 4");
        }
        
        JSONValue::JSONString ntpString = 
            shardMetadataArr[2].get<JSONValue::JSONString>();
        ntp = NULL;
        if (!ntpString.empty()) {
            ntp = getOps()->getNotifyableFromKey(vector<string>(), ntpString);
        }
        m_shardTree.insertNode(
            shardMetadataArr[0].get<JSONValue::JSONUInteger>(),
            shardMetadataArr[1].get<JSONValue::JSONUInteger>(),
            ShardTreeData(shardMetadataArr[3].get<JSONValue::JSONInteger>(),
                          ntp));

        LOG_DEBUG(CL_LOG,
                  "unmarshallShards: Found shard: start=%" PRIu64 
                  " (%s), end=%" PRIu64 " (%s), notifyable key=%s, "
                  "priority=%" PRId64,
                  shardMetadataArr[0].get<JSONValue::JSONUInteger>(),
                  JSONCodec::encode(shardMetadataArr[0]).c_str(),
                  shardMetadataArr[1].get<JSONValue::JSONUInteger>(),
                  JSONCodec::encode(shardMetadataArr[1]).c_str(),
                  (ntp == NULL) ? "NULL" : ntp->getKey().c_str(),
                  shardMetadataArr[3].get<JSONValue::JSONInteger>());

        ++m_shardTreeCount;
    }
}

};	/* End of 'namespace clusterlib' */
