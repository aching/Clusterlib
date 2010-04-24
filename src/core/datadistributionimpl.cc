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

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "DataDistribution"

#include <limits>
#include "clusterlibinternal.h"
#include <boost/regex.hpp>

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib
{

/**********************************************************************/
/* Implementation of class DataDistributionImpl                       */
/**********************************************************************/

/*
 * Constructor.
 */
DataDistributionImpl::DataDistributionImpl(FactoryOps *fp,
                                           const string &key,
                                           const string &name,
                                           GroupImpl *parentGroup)
    : NotifyableImpl(fp, key, name, parentGroup),
      m_version(ClusterlibInts::INITIAL_ZK_VERSION),
      m_shardTreeCount(0)
{
    TRACE(CL_LOG, "DataDistributionImpl");
}

/*
 * Unmarshall a stringified sequence of shards. The shards are stored
 * as a JSONArray of JSONArrays (begin, end, notifyablekey, priority)
 */
void
DataDistributionImpl::unmarshall(const string &marshalledData)
{
    TRACE(CL_LOG, "unmarshall");

    throwIfRemoved();

    vector<string> components;
    vector<string> shardComponents;
    vector<string>::iterator sIt;

    LOG_DEBUG(CL_LOG, 
              "unmarshall: Got marshalledData string (%s)", 
              marshalledData.c_str());

    Locker l(getSyncLock());

    clear();
    m_shardTreeCount = 0;

    if (marshalledData.empty()) {
        return;
    }

    JSONValue jsonValue = JSONCodec::decode(marshalledData);
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
                "unmarshall: Impossible that the size of the shardMetadataArr"
                " != 4 ");
        }
        
        JSONValue::JSONString ntpString = 
            shardMetadataArr[2].get<JSONValue::JSONString>();
        ntp = NULL;
        if (!ntpString.empty()) {
            ntp = getOps()->getNotifyableFromKey(ntpString);
        }
        m_shardTree.insertNode(
            shardMetadataArr[0].get<JSONValue::JSONUInteger>(),
            shardMetadataArr[1].get<JSONValue::JSONUInteger>(),
            ShardTreeData(shardMetadataArr[3].get<JSONValue::JSONInteger>(),
                          ntp));

        LOG_DEBUG(CL_LOG,
                  "unmarshall: Found shard: start=%" PRIu64 " (%s), end=%"
                  PRIu64 " (%s), notifyable key=%s, priority=%" PRId64,
                  shardMetadataArr[0].get<JSONValue::JSONUInteger>(),
                  JSONCodec::encode(shardMetadataArr[0]).c_str(),
                  shardMetadataArr[1].get<JSONValue::JSONUInteger>(),
                  JSONCodec::encode(shardMetadataArr[1]).c_str(),
                  (ntp == NULL) ? "NULL" : ntp->getKey().c_str(),
                  shardMetadataArr[3].get<JSONValue::JSONInteger>());

        ++m_shardTreeCount;
    }
}

/*
 * Marshall a data distribution to string form with JSON.
 */
string
DataDistributionImpl::marshall()
{
    TRACE(CL_LOG, "marshall");

    throwIfRemoved();

    Locker l(getSyncLock());

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
              "marshall: Generated string (%s)", 
              JSONCodec::encode(shardArr).c_str());

    return JSONCodec::encode(shardArr);
}


void
DataDistributionImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Initialize the shards.
     */
    update();
}

void
DataDistributionImpl::removeRepositoryEntries()
{
    getOps()->removeDataDistribution(this);
}

/*
 * Update the cached shards of the data distribution.
 */
bool
DataDistributionImpl::update()
{
    TRACE(CL_LOG, "update");

    throwIfRemoved();

    /*
     * Must lock before asking the repository for the values
     * to ensure that we don't lose intermediate values or
     * values installed after this update.
     */
    Locker l(getSyncLock());

    int32_t version;
    string dataDistribution = getOps()->loadShards(getKey(), version);
    ShardList::iterator sIt;

    /* 
     * Only update if this is a newer version.
     */
    if (version > getVersion()) {
        clear();
        unmarshall(dataDistribution);
        setVersion(version);
        return true;
    }

    /*
     * No change.
     */
    return false;
}

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

vector<Notifyable *> 
DataDistributionImpl::getNotifyables(const Key &key)
{
    TRACE(CL_LOG, "getNotifyables");

    throwIfRemoved();

    /* This is a slow implementation, will be optimized later. */
    vector<Shard> shardVec = getAllShards(NULL, -1);
    /* Sort the shard by priority */
    sort(shardVec.begin(), shardVec.end(), shardPriorityCompare);

    vector<Shard>::iterator shardVecIt;
    vector<Notifyable *> ntpVec;
    for (shardVecIt = shardVec.begin(); 
         shardVecIt != shardVec.end(); 
         shardVecIt++) {
        if ((shardVecIt->getStartRange() <= key.hashKey()) &&
            (shardVecIt->getEndRange() >= key.hashKey())) {
            ntpVec.push_back(shardVecIt->getNotifyable());
        }
    }

    return ntpVec;
}

vector<Notifyable *> 
DataDistributionImpl::getNotifyables(HashRange hashedKey)
{
    TRACE(CL_LOG, "getNotifyables");

    throwIfRemoved();

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

vector<Shard> 
DataDistributionImpl::getAllShards(const Notifyable *ntp, int32_t priority)
{
    TRACE(CL_LOG, "getAllShards");

    throwIfRemoved();

    /* 
     * Get all the shards and then filter based on the notifyable and/or
     * priority.
     */
    vector<Shard> res;
    IntervalTree<HashRange, ShardTreeData>::iterator treeIt;

    Locker l(getSyncLock());
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
DataDistributionImpl::removeShard(Shard &shard)
{
    TRACE(CL_LOG, "removeShard");

    throwIfRemoved();

    Locker l(getSyncLock());

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

uint32_t
DataDistributionImpl::getShardCount()
{
    TRACE(CL_LOG, "getShardCount");

    throwIfRemoved();

    Locker l(getSyncLock());
    return m_shardTreeCount;
}

/*
 * Is the distribution covered? Note that this method is
 * very expensive as it has to look through every notifyable in the tree
 */
bool
DataDistributionImpl::isCovered()
{
    TRACE(CL_LOG, "isCovered");

    throwIfRemoved();

    Locker l(getSyncLock());

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
DataDistributionImpl::splitHashRange(int32_t numShards)
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
DataDistributionImpl::insertShard(HashRange start,
                                  HashRange end,
                                  Notifyable *ntp,
                                  int32_t priority) 
{
    TRACE(CL_LOG, "insertShard");

    throwIfRemoved();
    
    Locker l(getSyncLock());

    m_shardTree.insertNode(start, end, ShardTreeData(priority, ntp));
    ++m_shardTreeCount;
}


void
DataDistributionImpl::clear()
{
    TRACE(CL_LOG, "clear");
    
    throwIfRemoved();
    
    Locker l(getSyncLock());
    
    while (m_shardTree.empty() == false) {
        m_shardTree.deleteNode(m_shardTree.getTreeHead());
    }
    m_shardTreeCount = 0;
}

DataDistributionImpl::~DataDistributionImpl()
{
    Locker l(getSyncLock());

    /*
     * Cannot simply call "clear()" since the repository object may have
     * been removed.
     */
    while (m_shardTree.empty() == false) {
        m_shardTree.deleteNode(m_shardTree.getTreeHead());
    }
    m_shardTreeCount = 0;
}

void
DataDistributionImpl::publishShards(bool unconditional)
{
    TRACE(CL_LOG, "publishShards");

    throwIfRemoved();

    Locker l(getSyncLock());

    string marshalledShards = marshall();
    int32_t finalVersion;

    LOG_INFO(CL_LOG,
             "Tried to set data distribution for notifyable %s to %s "
             "with version %d, unconditional %d\n",
             getKey().c_str(), 
             marshalledShards.c_str(),
             getVersion(),
             unconditional);

    try {
        updateShards(marshalledShards,
                     ((unconditional == false) ? getVersion() : -1),
                     finalVersion);
    }
    catch (const zk::BadVersionException &e) {
        throw PublishVersionException(e.what());
    }

    /* Since we should have the lock, the data should be identical to
     * the zk data.  When the lock is released, clusterlib events will
     * try to push this change again.  */
    setVersion(finalVersion);
}

void
DataDistributionImpl::updateShards(const string &encodedShards,
                                   int32_t version,
                                   int32_t &finalVersion)
{
    TRACE(CL_LOG, "updateShards");
    
    string shardsKey = NotifyableKeyManipulator::createShardsKey(getKey());
    Stat stat;
    bool exists = false;

    /*
     * Update the shards.
     */
    SAFE_CALL_ZK((exists = getOps()->getRepository()->nodeExists(shardsKey)),
                 "Could not determine whether key %s exists: %s",
                 shardsKey.c_str(),
                 false,
                 true);
    if (!exists) {
        SAFE_CALL_ZK(getOps()->getRepository()->createNode(
                         shardsKey, encodedShards, 0),
                     "Creation of %s failed: %s",
                     shardsKey.c_str(),
                     true,
                     true);
    }

    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(shardsKey, 
                                                        encodedShards, 
                                                        version,
                                                        &stat),
                 "Setting of %s failed: %s",
                 shardsKey.c_str(),
                 false,
                 true);

    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->nodeExists(
            shardsKey,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::SHARDS_CHANGE)),
        ,
        CachedObjectChangeHandlers::SHARDS_CHANGE,
        shardsKey,
        "Reestablishing watch on value of %s failed: %s",
        shardsKey.c_str(),
        true,
        true);

    finalVersion = stat.version;
}


};       /* End of 'namespace clusterlib' */
