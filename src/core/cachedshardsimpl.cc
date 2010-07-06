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
      m_shardTree(NULL),
      m_shardTreeCount(0),
      m_hashRange(NULL)
{
    /* 
     * Start with the UnknownHashRange and initialize to the correct
     * one later. 
     */
    m_hashRange = &(getOps()->getHashRange(UnknownHashRange::name()));

    /*
     * Do this for consistency.
     */
    m_shardTree = new IntervalTree<HashRange &, ShardTreeData>(
        *m_hashRange, ShardTreeData());
}

CachedShardsImpl::~CachedShardsImpl()
{
    Locker l(&getCachedDataLock());
    
    /*
     * Cannot simply call "clear()" since the repository object may have
     * been removed.
     */
    IntervalTreeNode<HashRange &, ShardTreeData> *node = NULL;
    while (m_shardTree->empty() == false) {
        node = m_shardTree->getTreeHead();
        
        node = m_shardTree->deleteNode(node);
        delete &(node->getStartRange());
        delete &(node->getEndRange());
        delete &(node->getEndRangeMax());
        delete node;
    }
    delete m_shardTree;
    m_shardTree = NULL;
    m_shardTreeCount = 0;

    delete m_hashRange;
    m_hashRange = NULL;
}

int32_t
CachedShardsImpl::publish(bool unconditional)
{
    TRACE(CL_LOG, "publish");

    getNotifyable()->throwIfRemoved();

    string shardsKey = DataDistributionImpl::createShardJsonObjectKey(
        getNotifyable()->getKey());

    Locker l(&getCachedDataLock());

    string encodedJsonObject = JSONCodec::encode(marshalShards());
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

    unmarshalShards(encodedJsonValue);
}

string
CachedShardsImpl::getHashRangeName()
{
    Locker l(&getCachedDataLock());

    return m_hashRange->getName();
}

vector<Notifyable *> 
CachedShardsImpl::getNotifyables(const HashRange &hashPoint)
{
    TRACE(CL_LOG, "getNotifyables");

    getNotifyable()->throwIfRemoved();

    /* This is a slow implementation, will be optimized later. */
    vector<Shard> shardVec = getAllShards(NULL, -1);
    vector<Notifyable *> ntpVec;

    /* Allow this to success if nothing exists */
    if (shardVec.empty()) {
        return ntpVec;
    }

    throwIfUnknownHashRange();

    /* Sort the shard by priority */
    sort(shardVec.begin(), shardVec.end(), shardPriorityCompare);

    vector<Shard>::iterator shardVecIt;

    for (shardVecIt = shardVec.begin(); shardVecIt != shardVec.end(); 
         ++shardVecIt) {
        if ((shardVecIt->getStartRange() <= hashPoint) &&
            (shardVecIt->getEndRange() >= hashPoint)) {
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
    throwIfUnknownHashRange();

    Locker l(&getCachedDataLock());

    IntervalTree<HashRange &, ShardTreeData>::iterator it
        = m_shardTree->begin();
    if (it->getStartRange().isBegin() == false) {
        return false;
    }
    HashRange &end = m_hashRange->create();
    end = it->getStartRange();
    for (; it != m_shardTree->end(); ++it) {
        LOG_DEBUG(
            CL_LOG, 
            "isCovered: end=%s startRange=%s endRange=%s",
            JSONCodec::encode(end.toJSONValue()).c_str(), 
            JSONCodec::encode(it->getStartRange().toJSONValue()).c_str(), 
            JSONCodec::encode(it->getEndRange().toJSONValue()).c_str());
        if (it->getStartRange() > end) {
            delete &end;
            return false;
        }
        else if (it->getEndRange() >= end) {
            if (it->getEndRange().isEnd()) {
                delete &end;
                return true;
            }
            else {
                end = it->getEndRange();
                ++end;
            }
        }
    }

    delete &end;
    return false;
}

void 
CachedShardsImpl::insert(const HashRange &start,
                         const HashRange &end,
                         Notifyable *ntp,
                         int32_t priority) 
{
    TRACE(CL_LOG, "insert");

    getNotifyable()->throwIfRemoved();
    
    Locker l(&getCachedDataLock());

    /*
     * Ensure that the HashRange objects are the same type and not
     * UnknownHashRange.  The only time they need not be the same as
     * the m_hashRange is if m_shardTreeCount == 0.  Then m_hashRange
     * is set to this type.
     */
    if ((start.getName() == UnknownHashRange::name()) ||
        (end.getName() == UnknownHashRange::name())) {
        throw InvalidArgumentsException(
            "insert: Either start or end has HashRange == UnknownHashRange");
    }

    if ((m_shardTreeCount == 0) && 
        (start.getName() != getHashRangeName())) {
        clear();
        delete m_hashRange;
        m_hashRange = &(start.create());
        delete m_shardTree;
        m_shardTree = new IntervalTree<HashRange &, ShardTreeData>(
            *m_hashRange, ShardTreeData());
    }

    if ((typeid(start) != typeid(*m_hashRange)) ||
        (typeid(start) != typeid(end))) {
        throw InvalidArgumentsException(
            "insert: The types of start, end and "
            "the set hash range are not in agreement.");
    }

    /* Alllocate the data that will be put into the tree. */
    HashRange &finalStart = m_hashRange->create();
    HashRange &finalEnd = m_hashRange->create();
    HashRange &finalEndMax = m_hashRange->create();
    finalStart = start;
    finalEnd = end;

    m_shardTree->insertNode(
        finalStart, 
        finalEnd, 
        finalEndMax, 
        ShardTreeData(priority, (ntp == NULL) ? string() : ntp->getKey()));
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
    IntervalTree<HashRange &, ShardTreeData>::iterator treeIt;

    Locker l(&getCachedDataLock());

    if (getHashRangeName() == UnknownHashRange::name()) {
        res = m_unknownShardArr;
    }
    else {
        for (treeIt = m_shardTree->begin(); 
             treeIt != m_shardTree->end(); 
             ++treeIt) {
            res.push_back(
                Shard(dynamic_cast<Root *>(
                          getOps()->getNotifyable(
                              NULL,
                              ClusterlibStrings::REGISTERED_ROOT_NAME,
                              ClusterlibStrings::ROOT,
                              CACHED_ONLY)),
                      treeIt->getStartRange(),
                      treeIt->getEndRange(),
                      treeIt->getData().getNotifyableKey(),
                      treeIt->getData().getPriority()));
        }
    }

    vector<Shard>::iterator resIt;
    vector<Shard> finalRes;
    for (resIt = res.begin(); resIt != res.end(); ++resIt) {
        /* Filter by notifyable if not NULL */
        if ((ntp == NULL) || (ntp->getKey() == resIt->getNotifyableKey())) {
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
    throwIfUnknownHashRange();

    Locker l(&getCachedDataLock());

    //AC-debug
    m_shardTree->printDepthFirstSearch();

    IntervalTreeNode<HashRange &, ShardTreeData> *node = 
        m_shardTree->nodeSearch(shard.getStartRange(),
                                shard.getEndRange(),
                                ShardTreeData(shard.getPriority(),
                                              shard.getNotifyableKey()));
    if (node != NULL) {
        node = m_shardTree->deleteNode(node);
        delete &(node->getStartRange());
        delete &(node->getEndRange());
        delete &(node->getEndRangeMax());
        delete node;

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
    
    IntervalTreeNode<HashRange &, ShardTreeData> *node = NULL;
    while (m_shardTree->empty() == false) {
        node = m_shardTree->getTreeHead();

        node = m_shardTree->deleteNode(node);
        delete &(node->getStartRange());
        delete &(node->getEndRange());
        delete &(node->getEndRangeMax());
        delete node;
    }
    m_shardTreeCount = 0;

    m_unknownShardArr.clear();
}

JSONValue::JSONArray
CachedShardsImpl::marshalShards()
{
    TRACE(CL_LOG, "marshalShards");

    getNotifyable()->throwIfRemoved();
    
    Locker l(&getCachedDataLock());

    IntervalTree<HashRange &, ShardTreeData>::iterator it;
    JSONValue::JSONArray shardArr;
    JSONValue::JSONArray shardMetadataArr;

    /* Shard format: [ "<HashRange name>",[<shard0],[shard1],...] */
    if (m_shardTreeCount != 0) {
        if (m_hashRange->getName() == UnknownHashRange::name()) {
            throw InvalidMethodException(
                "marshalShards: Cannot marshal shards if the HashRange is "
                "UnknownHashRange and the number of elements is > 0");
        }

        shardArr.push_back(m_hashRange->getName());
        for (it = m_shardTree->begin(); it != m_shardTree->end(); it++) {
            shardMetadataArr.clear();
            shardMetadataArr.push_back(it->getStartRange().toJSONValue());
            shardMetadataArr.push_back(it->getEndRange().toJSONValue());
            shardMetadataArr.push_back(it->getData().getNotifyableKey());
            shardMetadataArr.push_back(it->getData().getPriority());
            shardArr.push_back(shardMetadataArr);
        }
    }

    LOG_DEBUG(CL_LOG, 
              "marshalShards: Generated string (%s)", 
              JSONCodec::encode(shardArr).c_str());

    return shardArr;
}

void
CachedShardsImpl::unmarshalShards(const string &encodedJsonArr)
{
    TRACE(CL_LOG, "unmarshalShards");

    getNotifyable()->throwIfRemoved();

    vector<string> components;
    vector<string> shardComponents;
    vector<string>::iterator sIt;

    LOG_DEBUG(CL_LOG, 
              "unmarshalShards: Got encodedJsonArr '%s'", 
              encodedJsonArr.c_str());

    Locker l(&getCachedDataLock());

    clear();
    m_shardTreeCount = 0;

    if (encodedJsonArr.empty()) {
        return;
    }

    JSONValue::JSONArray jsonArr = 
        JSONCodec::decode(encodedJsonArr).get<JSONValue::JSONArray>();
    if (jsonArr.empty()) {
        return;
    }

    JSONValue::JSONArray shardMetadataArr;    
    JSONValue::JSONArray::const_iterator jsonArrIt;

    /* 
     * First array element should be the HashRange name and should
     * match the *m_hashRange. 
     */
    JSONValue::JSONString hashRangeName = 
        jsonArr.front().get<JSONValue::JSONString>();
    jsonArr.pop_front();
    delete m_hashRange;
    m_hashRange = &(getOps()->getHashRange(hashRangeName));
    delete m_shardTree;
    m_shardTree = new IntervalTree<HashRange &, ShardTreeData>(
        *m_hashRange, ShardTreeData());

    bool unknownHashRange = false;
    if (m_hashRange->getName() == UnknownHashRange::name()) {
        unknownHashRange = true;
    }

    /*
     * If this is UnknownHashRange data, put in m_shardArr, otherwise
     * put in m_shardTree.
     */
    for (jsonArrIt = jsonArr.begin(); 
         jsonArrIt != jsonArr.end(); 
         ++jsonArrIt) {
        shardMetadataArr.clear();
        shardMetadataArr = jsonArrIt->get<JSONValue::JSONArray>();
        if (shardMetadataArr.size() != 4) {
            throw InconsistentInternalStateException(
                "unmarshalShards: Impossible that the size of the "
                "shardMetadataArr != 4");
        }
        
        JSONValue::JSONString ntpKey = 
            shardMetadataArr[2].get<JSONValue::JSONString>();

        if (unknownHashRange) {
            m_unknownShardArr.push_back(
                Shard(dynamic_cast<Root *>(
                          getOps()->getNotifyable(
                              NULL,
                              ClusterlibStrings::REGISTERED_ROOT_NAME,
                              ClusterlibStrings::ROOT,
                              CACHED_ONLY)),
                      UnknownHashRange(shardMetadataArr[0]),
                      UnknownHashRange(shardMetadataArr[1]),
                      ntpKey,
                      shardMetadataArr[3].get<JSONValue::JSONInteger>()));
        }
        else {
            HashRange &start = m_hashRange->create();
            HashRange &end = m_hashRange->create();
            HashRange &endMax = m_hashRange->create();
            start.set(shardMetadataArr[0]);
            end.set(shardMetadataArr[1]);
            
            m_shardTree->insertNode(
                start,
                end,
                endMax,
                ShardTreeData(
                    shardMetadataArr[3].get<JSONValue::JSONInteger>(),
                    ntpKey));
        }

        LOG_DEBUG(CL_LOG,
                  "unmarshalShards: Found shard with HashRange name %s: "
                  "start=%s end=%s, notifyable key=%s, priority=%" PRId64,
                  m_hashRange->getName().c_str(),
                  JSONCodec::encode(shardMetadataArr[0]).c_str(),
                  JSONCodec::encode(shardMetadataArr[1]).c_str(),
                  ntpKey.c_str(),
                  shardMetadataArr[3].get<JSONValue::JSONInteger>());

        ++m_shardTreeCount;
    }
}

void
CachedShardsImpl::throwIfUnknownHashRange()
{
    Locker l(&getCachedDataLock());

    if (getHashRangeName() == UnknownHashRange::name()) {
        throw InvalidMethodException("throwIfUnknownHashRange: This method is "
                                     "not available for UnknownHashRange");
    }
}

}	/* End of 'namespace clusterlib' */
