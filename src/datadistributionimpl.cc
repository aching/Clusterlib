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

namespace clusterlib
{

/**********************************************************************/
/* Implementation of class DataDistribution                           */
/**********************************************************************/

/*
 * Constructor.
 */
DataDistributionImpl::DataDistributionImpl(FactoryOps *fp,
                                           const string &key,
                                           const string &name,
                                           GroupImpl *parentGroup)
    : NotifyableImpl(fp, key, name, parentGroup),
      m_version(-2),
      m_shardTreeCount(0)
{
    TRACE(CL_LOG, "DataDistributionImpl");
}

/*
 * Unmarshall a stringified sequence of shards. Each shard
 * is stringified to "begin,end,nodekey,priority;"
 */
void
DataDistributionImpl::unmarshall(const string &marshalledData)
{
    TRACE(CL_LOG, "unmarshall");

    throwIfRemoved();

    vector<string> components;
    vector<string> shardComponents;
    vector<string>::iterator sIt;

    Locker l(getSyncLock());

    clear();
    m_shardTreeCount = 0;
    split(components, marshalledData, is_any_of(";"));
    for (sIt = components.begin(); sIt != components.end() - 1; sIt++) {
        split(shardComponents, *sIt, is_any_of(","));
        if (shardComponents.size() != 4) {
	    stringstream s;
	    s << shardComponents.size();
	    LOG_WARN(CL_LOG,
                     "shardComponents (%d component(s)) = %s", 
                     shardComponents.size(), (*sIt).c_str());
            throw InconsistentInternalStateException(
                "Malformed shard \"" +
                *sIt +
                "\", expecting 4 components " +
                "and instead got " + s.str().c_str());
        }

        Node *node = NULL;
        if (shardComponents[2].empty() == false) {
            node = dynamic_cast<Node *>(
                getOps()->getNotifyableFromKey(shardComponents[2]));
        }

        m_shardTree.insertNode(
            ::atoll(shardComponents[0].c_str()), 
            ::atoll(shardComponents[1].c_str()),
            ShardTreeData(::atoll(shardComponents[3].c_str()), node));
        m_shardTreeCount++;
    }
}

/*
 * Marshall a data distribution to string form.
 */
string
DataDistributionImpl::marshall()
{
    TRACE(CL_LOG, "marshall");

    throwIfRemoved();

    Locker l(getSyncLock());

    stringstream res;
    IntervalTree<HashRange, ShardTreeData>::iterator it;
    for (it = m_shardTree.begin(); it != m_shardTree.end(); it++) {
        res << it->getStartRange() << "," << it->getEndRange() << ",";
        if (it->getData().getNode() != NULL) {
            res << it->getData().getNode()->getKey();
        }
        res << "," << it->getData().getPriority() << ";";
    }

    return res.str();
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
        while (m_shardTree.empty() == false) {
            m_shardTree.deleteNode(m_shardTree.getTreeHead());
        }

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

vector<const Node *> 
DataDistributionImpl::getNodes(const Key &key)
{
    TRACE(CL_LOG, "getNodes");

    /* This is a slow implementation, will be optimized later. */
    vector<Shard> shardVec = getAllShards(NULL, -1);
    /* Sort the shard by priority */
    sort(shardVec.begin(), shardVec.end(), shardPriorityCompare);

    vector<Shard>::const_iterator shardVecIt;
    vector<const Node *> nodeVec;
    for (shardVecIt = shardVec.begin(); 
         shardVecIt != shardVec.end(); 
         shardVecIt++) {
        if ((shardVecIt->getStartRange() <= key.hashKey()) &&
            (shardVecIt->getEndRange() >= key.hashKey())) {
            nodeVec.push_back(shardVecIt->getNode());
        }
    }

    return nodeVec;
}

vector<const Node *> 
DataDistributionImpl::getNodes(HashRange hashedKey)
{
    TRACE(CL_LOG, "getNodes");

    /* This is a slow implementation, will be optimized later. */
    vector<Shard> shardVec = getAllShards(NULL, -1);
    /* Sort the shard by priority */
    sort(shardVec.begin(), shardVec.end(), shardPriorityCompare);

    vector<Shard>::const_iterator shardVecIt;
    vector<const Node *> nodeVec;
    for (shardVecIt = shardVec.begin(); shardVecIt != shardVec.end(); 
         shardVecIt++) {
        if ((shardVecIt->getStartRange() <= hashedKey) &&
            (shardVecIt->getEndRange() >= hashedKey)) {
            nodeVec.push_back(shardVecIt->getNode());
        }
    }

    return nodeVec;
}

vector<Shard> 
DataDistributionImpl::getAllShards(const Node *node, int32_t priority)
{
    TRACE(CL_LOG, "getAllShards");

    throwIfRemoved();

    /* 
     * Get all the shards and then filter based on the node and/or
     * priority.
     */
    vector<Shard> res;
    IntervalTree<HashRange, ShardTreeData>::iterator treeIt;

    for (treeIt = m_shardTree.begin(); treeIt != m_shardTree.end(); treeIt++) {
        res.push_back(Shard(treeIt->getStartRange(),
                            treeIt->getEndRange(),
                            treeIt->getData().getNode(),
                            treeIt->getData().getPriority()));
    }
    vector<Shard>::const_iterator resIt;
    vector<Shard> finalRes;
    for (resIt = res.begin(); resIt != res.end(); resIt++) {
        /* Filter by node if not NULL */
        if ((node == NULL) || (node == resIt->getNode())) {
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
DataDistributionImpl::removeShard(const Shard &shard)
{
    TRACE(CL_LOG, "removeShard");

    IntervalTreeNode<HashRange, ShardTreeData> *treeNode = 
        m_shardTree.nodeSearch(shard.getStartRange(),
                               shard.getEndRange(),
                               ShardTreeData(shard.getPriority(),
                                             shard.getNode()));
    if (treeNode != NULL) {
        
        m_shardTree.deleteNode(treeNode);
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

    return m_shardTreeCount;
}

/*
 * Is the distribution covered? Note that this method is
 * very expensive as it has to look through every node in the tree
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
        LOG_ERROR(CL_LOG, "*** %llu %llu %llu", 
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

vector<HashRange> splitHashRange(int32_t numShards)
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
                                  const Node *node,
                                  int32_t priority) 
{
    TRACE(CL_LOG, "insertShard");

    throwIfRemoved();
    
    Locker l(getSyncLock());

    m_shardTree.insertNode(start, end, ShardTreeData(priority, node));
    m_shardTreeCount++;
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
    
    while (m_shardTree.empty() == false) {
        m_shardTree.deleteNode(m_shardTree.getTreeHead());
    }

    m_shardTreeCount = 0;    
}

/*
 * Publish the data distribution if there were changes.
 */
void
DataDistributionImpl::publish()
{
    TRACE(CL_LOG, "publish");

    throwIfRemoved();

    Locker l(getSyncLock());

    string marshalledShards = marshall();
    int32_t finalVersion;

    LOG_INFO(CL_LOG,
             "Tried to set data distribution for node %s to %s "
             "with version %d\n",
             getKey().c_str(), 
             marshalledShards.c_str(),
             getVersion());

    getOps()->updateDataDistribution(getKey(), 
                                     marshalledShards,
                                     getVersion(),
                                     finalVersion);

    /* Since we should have the lock, the data should be identical to
     * the zk data.  When the lock is released, clusterlib events will
     * try to push this change again.  */
    setVersion(finalVersion);
}

};       /* End of 'namespace clusterlib' */
