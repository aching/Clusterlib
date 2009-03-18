/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "DataDistribution"

#include "clusterlib.h"
#include <boost/regex.hpp>

namespace clusterlib
{

/*
 * Constants for identifying the various parts of a shard
 * specification.
 */
const int32_t DataDistribution::SC_LOWBOUND_IDX		= 0;
const int32_t DataDistribution::SC_HIBOUND_IDX		= 1;
const int32_t DataDistribution::SC_NOTIFYABLEKEY_IDX	= 2;

static HashRange
jenkinsHashImpl(const string &key)
{
    // adapted from jenkin's one-at-a-time hash
    uint32_t hash = 0;
    size_t i;
    
    for (i = 0; i < key.length(); i++) {
        hash += key.at(i);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
};

static HashRange
md5HashImpl(const string &key)
{
    return 0;
}

/**********************************************************************/
/* Implementation of class DataDistribution                           */
/**********************************************************************/

/*
 * The array of hash functions.
 */
HashFunction *
DataDistribution::s_hashFunctions[] =
{
    NULL,		/* DD_HF_USERDEF */
    &md5HashImpl,	/* DD_HF_MD5 */
    &jenkinsHashImpl,	/* DD_HF_JENKINS */
    NULL		/* DD_HF_END */
};

/*
 * Constructor.
 */
DataDistribution::DataDistribution(Group *parentGroup,
                                   const string &name,
                                   const string &key,
                                   FactoryOps *fp,
                                   HashFunction *fnp)
    : Notifyable(fp, key, name, parentGroup),
      m_hashFnIndex(DD_HF_JENKINS),
      mp_hashFnPtr(fnp),
      m_shardsVersion(-2),
      m_manualOverridesVersion(-2)
{
    TRACE(CL_LOG, "DataDistribution");
    
    /*
     * Set up the hash function to use.
     */
    if (m_hashFnIndex == DD_HF_USERDEF) {
        if (mp_hashFnPtr == NULL) {
            mp_hashFnPtr = s_hashFunctions[DD_HF_JENKINS];
        }
    } else {
        mp_hashFnPtr = s_hashFunctions[m_hashFnIndex];
    }
};

/*
 * Unmarshall a string-form of the data distribution.
 */
void
DataDistribution::unmarshall(const string &marshalledData)
{
    TRACE(CL_LOG, "unmarshall");

    vector<string> components;
    
    /*
     * The marshalled form is "shards\nmanualOverrides"
     */
    split(components, marshalledData, is_any_of("\n")); 
    if (components.size() != 2) {
        throw ClusterException("Invalid data. Expecting 2 top "
                               "level components");
    }

    Locker s(getShardsLock());
    Locker m(getManualOverridesLock());

    unmarshallShards(components[0]);
    unmarshallOverrides(components[1]);
};

/*
 * Unmarshall a stringified sequence of shards. Each shard
 * is stringified to "begin,end,appname,groupname,nodename;"
 */
void
DataDistribution::unmarshallShards(const string &marshalledShards)
{
    TRACE(CL_LOG, "unmarshallShards");

    vector<string> components;
    vector<string> shardComponents;
    vector<string>::iterator sIt;
    Shard *nsp;

    /* No shards defined */
    if (marshalledShards.empty()) {
	return;
    }

    split(components, marshalledShards, is_any_of(";"));
    for (sIt = components.begin(); sIt != components.end() - 1; sIt++) {
        split(shardComponents, *sIt, is_any_of(","));
        if (shardComponents.size() != 3) {
	    stringstream s;
	    s << shardComponents.size();
	    LOG_WARN(CL_LOG,
                     "shardComponents (%d component(s)) = %s", 
                     shardComponents.size(), (*sIt).c_str());
            throw ClusterException("Malformed shard \"" +
                                   *sIt +
                                   "\", expecting 3 components " +
				   "and instead got " + s.str().c_str());
        }

        /*
         * Resolve the node lazily -- only load it if we're actually
         * asked to access it via this shard.
         */
        nsp = new Shard(this,
                        shardComponents[SC_NOTIFYABLEKEY_IDX],
                        ::atoll(shardComponents[SC_LOWBOUND_IDX].c_str()),
                        ::atoll(shardComponents[SC_HIBOUND_IDX].c_str()));
        if (nsp == NULL) {
            throw ClusterException("Could not create shard \"" +
                                   *sIt +
                                   "\"");
        }

        /*
         * Add the shard to our cache.
         */
        m_shards.push_back(nsp);
    }
};

/*
 * Unmarshall a stringified sequence of manual overrides.
 * Each override is stringified to "pattern,notifyableKey;".
 */
void
DataDistribution::unmarshallOverrides(const string &marshalledOverrides)
{
    TRACE(CL_LOG, "unmarshallOverrides");

    vector<string> components;
    vector<string> moComponents;
    vector<string>::iterator sIt;
    Notifyable *ntp;

    /* No manual overrides defined */
    if (marshalledOverrides.empty()) {
	return;
    }

    split(components, marshalledOverrides, is_any_of(";"));
    for (sIt = components.begin(); sIt != components.end(); sIt++) {
        split(moComponents, *sIt, is_any_of(","));
        if (moComponents.size() == 2) {
            ntp = getDelegate()->getNotifyableFromKey(moComponents[1]);
            // AC - TO be changed true);
            if (ntp != NULL) {
                /*
                 * Add the manual override to our cache.
                 */
                m_manualOverrides[moComponents[0]] 
                    = new ManualOverride(this,
                                         ntp,
                                         ntp->getKey());
            }
            else {
                throw ClusterException("Could not find Notifyable for manual "
                                       "override \"" +
                                       *sIt +
                                       "\"");
            }
        }
        else {
           throw ClusterException("Malformed manual override \"" +
                                  *sIt +
                                   "\", expecting 2 components");
        }
    }                                   
}

/*
 * Marshall a data distribution to string form.
 */
string
DataDistribution::marshall()
{
    TRACE(CL_LOG, "marshall");

    return marshallShards() + "\n" + marshallOverrides();
};

/*
 * Marshall a data distribution's set of shards to string form.
 * Each shard gets marshalled to "begin,end,notifyableKey;".
 */
string
DataDistribution::marshallShards()
{
    TRACE(CL_LOG, "marshallShards");

    string res = "";
    ShardList::iterator sIt;
    char buf[1024];
    Notifyable *ntp;
    string notifyableKey;

    Locker l(getShardsLock());
    for (sIt = m_shards.begin(); sIt != m_shards.end(); sIt++) {
        /*
         * This shard may forward to another data distribution or
         * point at a node (can figure this out from the key)
         */
        ntp = (*sIt)->getNotifyable();
        if (ntp != NULL) {
            notifyableKey = (*sIt)->getNotifyable()->getKey();
        }
        else {
            notifyableKey = (*sIt)->getKey();
        }
        snprintf(buf,
                 1024,
                 "%lld,%lld,%s;",
                 (*sIt)->beginRange(),
                 (*sIt)->endRange(),
                 notifyableKey.c_str());
        res += buf;
    }

    LOG_DEBUG(CL_LOG, 
              "marshallShards() created shard string %s",
              res.c_str());
    return res;
};

/*
 * Marshall a data distribution's manual overrides to string form.
 * Each manual override is marshalled to
 * "pattern,appname,groupname,nodename;".
 */
string
DataDistribution::marshallOverrides()
{
    TRACE(CL_LOG, "marshallOverrides");

    string res = "";
    ManualOverridesMap::iterator moIt;
    char buf[1024];
    Notifyable *ntp;
    string notifyableKey;

    Locker l(getManualOverridesLock());
    for (moIt = m_manualOverrides.begin();
         moIt != m_manualOverrides.end(); 
         moIt++) {
        /*
         * This manual override forwards to another data
         * distribution or points at a node (can figure this out
         * from the key).
         */
        ntp = (*moIt).second->getNotifyable();
        if (ntp != NULL) {
            notifyableKey = ntp->getKey();
        }
        else {
            notifyableKey = (*moIt).second->getKey();
        }
        snprintf(buf,
                 1024,
                 "%s,%s;",
                 (*moIt).first.c_str(),
                 notifyableKey.c_str());
        res += buf;
    }
    return res;
};

/*
 * Initialize the data distribution from the cluster.
 */
void
DataDistribution::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "updateCachedRepresentation");

    /*
     * Initialize the shards.
     */
    updateShards();

    /*
     * Initialize the manual overrides.
     */
    updateManualOverrides();
}

/*
 * Update the cached shards of the data distribution.
 */
bool
DataDistribution::updateShards()
{
    TRACE(CL_LOG, "udpateShards");

    /*
     * Must lock before asking the repository for the values
     * to ensure that we don't lose intermediate values or
     * values installed after this update.
     */
    Locker s(getShardsLock());

    int32_t shardsVersion;
    string shards = getDelegate()->loadShards(getKey(), shardsVersion);
    ShardList::iterator sIt;

    /* 
     * Only update if this is a newer version.
     */
    if (shardsVersion > getShardsVersion()) {
        for (sIt = m_shards.begin(); sIt != m_shards.end(); sIt++) {
            delete *sIt;
        }
        m_shards.clear();
        unmarshallShards(shards);
        setShardsVersion(shardsVersion);
        return true;
    }

    /*
     * No change.
     */
    return false;
}

/*
 * Update the manual overrides of the data distribution.
 */
bool
DataDistribution::updateManualOverrides()
{
    TRACE(CL_LOG, "updateManualOverrides");

    /*
     * Must lock before asking the repository for the values
     * to ensure that we don't lose intermediate values or
     * values installed after this update.
     */
    Locker m(getManualOverridesLock());

    int32_t overridesVersion;
    string overrides =
        getDelegate()->loadManualOverrides(getKey(), overridesVersion);
    ManualOverridesMap::iterator moIt;

    /* 
     * Only update if this is a newer version.
     */
    if (overridesVersion > getManualOverridesVersion()) {
        for (moIt = m_manualOverrides.begin();
             moIt != m_manualOverrides.end();
             moIt++) {
            delete (*moIt).second;
        }
        m_manualOverrides.clear();
        unmarshallOverrides(overrides);
        setManualOverridesVersion(overridesVersion);
        return true;
    }

    /*
     * No change.
     */
    return false;
}

/*
 * Hash a key to a node using the current
 * hash function for this distribution.
 */
Node *
DataDistribution::map(const string &key)
{
    TRACE(CL_LOG, "map");

    Notifyable *ntp;
    DataDistribution *distp = this;
    uint32_t idx;

    for (;;) {
#ifdef	ENABLING_MANUAL_OVERRIDES
        /*
         * Search the manual overrides for a match.
         */

        /*
         * Needs to be fixed up.
         */
        NodeMap::iterator j;
        cmatch what;

        for (j = m_manualOverrides.begin();
             j != m_manualOverrides.end();
             j++) {
            regex expression((*j).first);

            if (regex_match(key.c_str(), what, expression)) {
                return (*j).second;
            }
        }
#endif

        /*
         * Use the shard mapping, we didn't find a manual
         * override.
         */
        idx = distp->getShardIndex(distp->hashWork(key));
        ntp = distp->m_shards.at(idx)->loadNotifyable();
        if (distp->m_shards.at(idx)->isForwarded()) {
            /*
             * Follow the forwarding chain.
             */
            distp = dynamic_cast<DataDistribution *>(ntp);
        } else {
            return dynamic_cast<Node *>(ntp);
        }
    }

    /*NOTREACHED*/
    return NULL;
};

/*
 * Compute the hash range for this key.
 */
HashRange
DataDistribution::hashWork(const string &key)
{
    if (mp_hashFnPtr == NULL) {
        mp_hashFnPtr = s_hashFunctions[DD_HF_JENKINS];
    }
    return (*mp_hashFnPtr)(key);
};

/*
 * Returns the manual override item that matches this
 * key if one exists (the first one found, in an
 * unspecified order) or the empty string if none.
 */
string
DataDistribution::matchesManualOverride(const string &key)
{
    TRACE(CL_LOG, "matchesManualOverrides");

#ifdef	ENABLING_MANUAL_OVERRIDES
    /*
     * Search the manual overrides for a match.
     */
    NodeMap::iterator j;
    cmatch what;

    for (j = m_manualOverrides.begin();
         j != m_manualOverrides.end();
         j++) {
        regex expression((*j).first);

        if (regex_match(key.c_str(), what, expression)) {
            return (*j).first;
        }
    }
#endif

    return string("");
}

/*
 * Is the distribution covered? Note that this method is
 * very expensive and it loads every Notifyable referenced
 * by the distribution.
 */
bool
DataDistribution::isCovered()
{
    TRACE(CL_LOG, "isCovered");

    ShardList::iterator sIt;

    Locker l(getShardsLock());
    HashRange s;

    for (sIt = m_shards.begin(), s = 0;
         sIt != m_shards.end();
         sIt++) {
        if (s != (*sIt)->beginRange()) {
            return false;
        }
        if ((*sIt)->loadNotifyable() == NULL) {
            return false;
        }
        s = (*sIt)->endRange() + 1;
    }

    return true;
}

/*
 * Assign new shards.
 */
void
DataDistribution::setShards(vector<HashRange> &upperBounds)
{
    TRACE(CL_LOG, "setShards");

    ShardList::iterator sIt;
    vector<HashRange>::iterator vIt;

    Locker l(getShardsLock());
    HashRange s = 0;
    uint32_t oldSize, newSize;

    /*
     * Resize the shard vector to be as large
     * as upperBounds.
     */
    oldSize = m_shards.size();
    newSize = upperBounds.size();
    if (oldSize > newSize) {
        for (sIt = m_shards.begin() + newSize;
             sIt != m_shards.end();
             sIt++) {
            delete (*sIt);
        }
    }
    m_shards.resize(newSize);
    if (newSize > oldSize) {
        for (sIt = m_shards.begin() + oldSize;
             sIt != m_shards.end();
             sIt++) {
            *sIt = new Shard(this, NULL, 0, 0);
        }
    }

    /*
     * Set all lower and upper bounds.
     */
    for (sIt = m_shards.begin(),
             vIt= upperBounds.begin();
         sIt != m_shards.end();
         sIt++, vIt++) {
        if (*vIt <= s) {
            throw ClusterException("Invalid shard boundaries in setShards");
        }
        (*sIt)->setBeginRange(s);
        (*sIt)->setEndRange(*vIt);
        s = (*vIt) + 1;
    }
}

/*
 * Get the shard index for a hash value or key.
 */
uint32_t
DataDistribution::getShardIndex(const string &key)
{
    TRACE(CL_LOG, "getShardIndex");

    return getShardIndex(hashWork(key));
}
uint32_t
DataDistribution::getShardIndex(HashRange hash)
{
    TRACE(CL_LOG, "getShardIndex");

    /*
     * Use the shard mapping. This is a linear search; better efficiency
     * can for sure be obtained by doing a binary search or some other
     * faster algorithm that relies on the shards being sorted in
     * increasing hash range order.
     */
    ShardList::iterator sIt;
    uint32_t j;

    Locker l(getShardsLock());
    for (sIt = m_shards.begin(), j = 0;
         sIt != m_shards.end();
         j++, sIt++) {
        if ((*sIt)->covers(hash)) {
            return j;
        }
    }

    char buf[1024];
    snprintf(buf,
             1024,
             "There is no shard assigned to cover hash value %lld!",
             hash);
    throw ClusterException(buf);
};

/*
 * Retrieve the shard details for a given shard index.
 */
Notifyable *
DataDistribution::getShardDetails(uint32_t shardIndex,
                                  HashRange *lowP,
                                  HashRange *hiP,
                                  bool *isForwardedP)
{
    TRACE(CL_LOG, "getShardDetails");

    Locker l(getShardsLock());
    if (shardIndex >= m_shards.size()) {
        char buf[1024];
        snprintf(buf,
                 1024,
                 "Shard index %d out of range 0-%d",
                 shardIndex,
                 m_shards.size() - 1);
        throw ClusterException(buf);
    }

    /*
     * Fill out the values to be retrieved for those elements
     * for which the caller gave a variable address.
     */
    if (lowP != NULL) {
        *lowP = m_shards[shardIndex]->beginRange();
    }
    if (hiP != NULL) {
        *hiP = m_shards[shardIndex]->endRange();
    }
    if (isForwardedP != NULL) {
        *isForwardedP = m_shards[shardIndex]->isForwarded();
    }
    return m_shards[shardIndex]->loadNotifyable();
}

/*
 * Reassign a shard to a new Notifyable.
 */
void
DataDistribution::reassignShard(uint32_t shardIndex, Notifyable *ntp)
{
    TRACE(CL_LOG, "reassignShard");

    Locker l(getShardsLock());

    if (shardIndex >= m_shards.size()) {
        throw ClusterException("Shard index out of range in reassignShard");
    }
    m_shards[shardIndex]->reassign(ntp);
}
void
DataDistribution::reassignShard(uint32_t shardIndex, const string &key)
{
    TRACE(CL_LOG, "reassignShard");

    Locker l(getShardsLock());

    if (shardIndex >= m_shards.size()) {
        throw ClusterException("Shard index out of range in reassignShard");
    }
    m_shards[shardIndex]->reassign(key);
}

/*
 * (Re)assign a manual override to a new Notifyable.
 */
void
DataDistribution::reassignManualOverride(const string &pattern,
                                         Notifyable *ntp)
{
    TRACE(CL_LOG, "reassignManualOverride");

    Locker l(getManualOverridesLock());
    ManualOverride *mop = m_manualOverrides[pattern];

    if (mop == NULL) {
        mop = m_manualOverrides[pattern] =
            new ManualOverride(this, ntp, "");
    }
    mop->reassign(ntp);
}

void
DataDistribution::reassignManualOverride(const string &pattern,
                                         const string &key)
{
    TRACE(CL_LOG, "reassignManualOverride");

    Locker l(getManualOverridesLock());
    ManualOverride *mop = m_manualOverrides[pattern];

    if (mop == NULL) {
        mop = new ManualOverride(this, NULL, "");
        m_manualOverrides[pattern] = mop;
    }
    mop->reassign(key);
}

/*
 * Publish the data distribution if there were changes.
 */
void
DataDistribution::publish()
{
    TRACE(CL_LOG, "publish");

    Locker s(getShardsLock());
    Locker m(getManualOverridesLock());

    string marshalledShards = marshallShards();
    string marshalledOverrides = marshallOverrides();

    LOG_INFO(CL_LOG,
             "Tried to set data distribution for node %s to (%s, %s) "
             "with versions (%d, %d)\n",
             getKey().c_str(), 
             marshalledShards.c_str(),
             marshalledOverrides.c_str(),
             getShardsVersion(),
             getManualOverridesVersion());

    getDelegate()->updateDataDistribution(getKey(), 
                                      marshalledShards,
                                      marshalledOverrides,
                                      getShardsVersion(),
				      getManualOverridesVersion());

    /* Since we should have the lock, the data should be identical to
     * the zk data.  When the lock is released, clusterlib events will
     * try to push this change again.  */
    setShardsVersion(getShardsVersion() + 1);
    setManualOverridesVersion(getManualOverridesVersion() + 1);

}
/**********************************************************************/
/* Implementation of Shard and ManualOverride classes.                */
/**********************************************************************/

/*
 * Retrieve -- or load -- the Notifyable of this DD
 * element.
 */
Notifyable *
ManualOverride::loadNotifyable()
{
    /*
     * If the notifyable is already loaded, return it.
     */
    if (mp_notifyable != NULL) {
        return mp_notifyable;
    }

    /*
     * Otherwise load and return the notifyable.
     */
    if (isForwarded()) {
        mp_notifyable =
            mp_dist->getDelegate()->getDataDistributionFromKey(m_key, false);
    } else {
        mp_notifyable =
            mp_dist->getDelegate()->getNodeFromKey(m_key, false);
    }
    return mp_notifyable;
}
void
ManualOverride::determineForwarding()
{
    Notifyable *ntp = mp_dist->getDelegate()->getNotifyableFromKey(m_key);

    if (m_key == "") {
        m_isForwarded = false;
    } else if (dynamic_cast<Node *>(ntp)) {
        m_isForwarded = false;
    } else if (dynamic_cast<DataDistribution *>(ntp)) {
        m_isForwarded = true;
    } else {
        throw ClusterException("Key: \"" +
                               m_key +
                               "\" does not denote a node " +
                               "or distribution!");
    }
}

/*
 * Return true iff this shard covers the works represented
 * by this key.
 */
bool
Shard::covers(const string &key)
{
    HashRange hash = getDataDistribution()->hashWork(key);

    return covers(hash);
}
bool
Shard::covers(HashRange hash)
{
    if ((hash >= m_beginRange) && (hash <= m_endRange)) {
        return true;
    }
    return false;
};

};       /* End of 'namespace clusterlib' */
