/*
 * datadistribution.h --
 *
 * Definition of class DataDistribution; it represents a data
 * distribution (mapping from a key to a node) in clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_DATADISTRIBUTION_H_
#define _DATADISTRIBUTION_H_

namespace clusterlib
{

/**
 * Base class for Shard, also used directly for manual overrides.
 */
class ManualOverride
{
  public:
    /**
     * Get info out.
     */
    DataDistribution *getDataDistribution() { return mp_dist; }
    const string &getKey() { return m_key; }
    bool isForwarded() { return m_isForwarded; }
    Notifyable *getNotifyable() { return mp_notifyable; }
    Notifyable *loadNotifyable();

    /**
     * Reassign this shard to a different Notifyable
     */
    void reassign(Notifyable *np)
    {
        mp_notifyable = np;
        if (mp_notifyable == NULL) {
            m_key = "";
        } else {
            m_key = np->getKey();
        }
        determineForwarding();
    }
    void reassign(const string &key)
    {
        mp_notifyable = NULL;
        m_key = key;
        determineForwarding();
    }

    /**
     * Constructor.
     */
    ManualOverride(DataDistribution *dp,
                   Notifyable *np,
                   const string &key)
        : mp_dist(dp),
          mp_notifyable(np),
          m_key(key)
    {
        if (mp_notifyable != NULL) {
            m_key = mp_notifyable->getKey();
        }
        determineForwarding();
    }

  private:
    /***
     * Determine whether this is a forwarding or final reference.
     */
    void determineForwarding();

  private:
    /**
     * The data distribution this shard belongs to.
     */
    DataDistribution *mp_dist;

    /**
     * The Notifyable that this data distribution element is assigned to.
     */
    Notifyable *mp_notifyable;

    /**
     * The key of the Notifyable -- used in case the
     * pointer is not available.
     */
    string m_key;

    /**
     * Is this element forwarded? If yes, then the Notifyable denotes
     * a DataDistribution, otherwise it is a Node.
     */
    bool m_isForwarded;
};

/**
 * Definition of class Shard.
 */
class Shard
    : public virtual ManualOverride
{
  public:
    /**
     * Get the info out of the shard.
     */
    HashRange beginRange() { return m_beginRange; }
    HashRange endRange() { return m_endRange; }

    /**
     * Assign fields into the shard.
     */
    void setBeginRange(HashRange br) { m_beginRange = br; }
    void setEndRange(HashRange er) { m_endRange = er; }

    /**
     * Decide whether this piece of work belongs to
     * this shard.
     */

    bool covers(const string &key);
    bool covers(HashRange hash);

    /**
     * Constructor.
     */
    Shard(DataDistribution *dist,
          Notifyable *notifyable,
          HashRange beginRange,
          HashRange endRange)
        : ManualOverride(dist, notifyable, string("")),
          m_beginRange(beginRange),
          m_endRange(endRange)
    {
    }

    /**
     * Constructor.
     */
    Shard(DataDistribution *dist,
          const string &key,
          HashRange beginRange,
          HashRange endRange)
        : ManualOverride(dist, NULL, key),
          m_beginRange(beginRange),
          m_endRange(endRange)
    {
    }

  private:
    /**
     * The bounds for this shard. The range is beginRange <-> endRange.
     */
    HashRange m_beginRange;
    HashRange m_endRange;
};

/**
 * Definition of class DataDistribution.
 */
class DataDistribution
    : public virtual Notifyable
{
  public:
    /**
     * Enum of hash functions to use.
     */
    enum HashFunctions {
        DD_HF_USERDEF	= -1,
        DD_HF_MD5	= 0,
        DD_HF_JENKINS	= 1,
        DD_HF_END	= 2
    };

    /**
     * Constants for identifying the various parts of a shard definition.
     */
    static const int32_t SC_LOWBOUND_IDX;
    static const int32_t SC_HIBOUND_IDX;
    static const int32_t SC_NOTIFYABLEKEY_IDX;

    /**
     * Find the Node that the key maps to (recursively
     * following forwards).
     */
    Node *map(const string &key);

    /**
     * Hash a key.
     */
    HashRange hashWork(const string &key);

    /**
     * Return the manual override string that matches this
     * key if one exists (returns the first one found, in
     * an unspecified order) or the empty string if none.
     */
    string matchesManualOverride(const string &key);

    /**
     * Return the number of shards in this data distribution.
     */
    uint32_t getShardCount() { return m_shards.size(); }

    /**
     * Is the distribution covered (at the time of checking)?
     */
    bool isCovered();

    /**
     * Get/set the hash function to use.
     */
    HashFunctionId getHashFunctionIndex() { return m_hashFnIndex; }
    void setHashFunctionIndex(HashFunctionId idx) { m_hashFnIndex = idx; }
    void setHashFunction(HashFunction *fn)
    {
        if (fn == NULL) {
            m_hashFnIndex = DD_HF_JENKINS;
            mp_hashFnPtr = s_hashFunctions[DD_HF_JENKINS];
        } else {
            m_hashFnIndex = DD_HF_USERDEF;
            mp_hashFnPtr = fn;
        }
    }

    /*** 
     * \brief Get the data distribution locks.
     *
     * Guarantees that the this object's members will not be modified by
     * any process that does not own the lock (including the internal
     * clusterlib event system).
     */
    void acquireLock()
    {
	getShardsLock()->Acquire();
	getManualOverridesLock()->Acquire();
    }

    /*** 
     * \brief Releases the data distribution locks.
     *
     * Releases the lock so that other processes that do not own the
     * lock (including the internal clusterlib event system).
     */
    void releaseLock()
    {
	getShardsLock()->Release();
	getManualOverridesLock()->Release();
    }
        
    /**
     * Assign new shards.
     */
    void setShards(vector<HashRange> &upperBounds);

    /**
     * Get the shard index for a work item, or for a hash value.
     */
    uint32_t getShardIndex(const string &workItem);
    uint32_t getShardIndex(HashRange v);

    /**
     * Get all info out of a shard.
     */
    Notifyable *getShardDetails(uint32_t shardIndex,
                                HashRange *low = NULL,
                                HashRange *hi = NULL,
                                bool *isForwarded = NULL);

    /**
     * Reassign a shard to a different notifyable.
     */
    void reassignShard(uint32_t shardIndex, Notifyable *ntp);
    void reassignShard(uint32_t shardIndex, const string &key);

    /**
     * Assign - or reassign - a manual override to a different
     * notifyable.
     */
    void reassignManualOverride(const string &pattern,
                                Notifyable *ntp);
    void reassignManualOverride(const string &pattern,
                                const string &key);

    /**
     * Remove a manual override.
     */
    bool removeManualOverride(const string &pattern);

    /**
     *  Publish any changes to the clusterlib repository.
     */
    void publish();

  protected:
    /**
     * Friend declaration of Factory so that it can call the
     * protected constructor.
     */
    friend class Factory;

    /**
     * Shard and ManualOverride need access to getDelegate().
     */
    friend class Shard;
    friend class ManualOverride;

    /**
     * Constructor used by Factory.
     */
    DataDistribution(Group *parentGroup, 
                     const string &name,
                     const string &key,
                     FactoryOps *f,
                     HashFunction *fn = NULL);

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistribution() 
    {
	for (ManualOverridesMap::iterator moIt = m_manualOverrides.begin(); 
	     moIt != m_manualOverrides.end(); moIt++) {
	    delete moIt->second;
	}
	for (ShardList::iterator sIt = m_shards.begin(); 
	     sIt != m_shards.end(); sIt++) {
	    delete *sIt;
	}
    }

    /**
     * Initialize the cached representation of the distribution.
     */
    void initializeCachedRepresentation();

    /**
     * Retrieve the current version number of the
     * shards in this data distribution.
     */
    int32_t getShardsVersion() 
    {
	return m_shardsVersion; 
    }

    void setShardsVersion(int32_t version) 
    { 
	m_shardsVersion = version; 
    }

    /**
     * Retrieve the current version number of the
     * manualoverrides in this data distribution.
     */
    int32_t getManualOverridesVersion() 
    {
	return m_manualOverridesVersion; 
    }

    void setManualOverridesVersion(int32_t version) 
    {
	m_manualOverridesVersion = version; 
    }

    /**
     * Update the cached shards or manual overrides from
     * the repository. Return true if there was actually
     * a change in the value (as determined by comparing
     * stored and new version).
     */
    bool updateShards();
    bool updateManualOverrides();

  private:
    /**
     * Make the default constructor private so it cannot be called.
     */
    DataDistribution()
        : Notifyable(NULL, "", "", NULL)
    {
        throw ClusterException("Someone called the DataDistribution "
                               "default constructor!");
    }

    /**
     * Unmarshall a string into this data distribution.
     */
    void unmarshall(const string &marshalledDist);

    /**
     * Unmarshall a stringified sequence of shards.
     */
    void unmarshallShards(const string &marshalledShards);

    /**
     * Unmarshall a stringified sequence of manual overrides.
     */
    void unmarshallOverrides(const string &marshalledOverrides);

    /**
     * Marshall a data distribution into a string.
     */
    string marshall();

    /**
     * Marshall shards into a string.
     */
    string marshallShards();

    /**
     * Marshall manual overrides into a string.
     */
    string marshallOverrides();

    /**
     * Retrieve a pointer to the shards lock and
     * manual overrides lock.
     */
    Mutex *getShardsLock() { return &m_shardsLock; }
    Mutex *getManualOverridesLock() { return &m_manualOverridesLock; }

  private:
    /**
     * The manual overrides for this data distribution.
     */
    ManualOverridesMap m_manualOverrides;
    Mutex m_manualOverridesLock;

    /**
     * Which hash function to use.
     */
    int32_t m_hashFnIndex;

    /**
     * If using a user supplied hash function, store
     * a pointer to it here. If using a built-in
     * hash function, store it here from the class-static
     * array.
     */
    HashFunction *mp_hashFnPtr;

    /**
     * Class-static variable holding the array of
     * hash functions.
     */
    static HashFunction *s_hashFunctions[];

    /**
     * The shards in this data distribution.
     */
    ShardList m_shards;
    Mutex m_shardsLock;

    /**
     * The version number of the shards
     */
    int32_t m_shardsVersion;

    /**
     * The version number of the overrides
     */
    int32_t m_manualOverridesVersion;


};

};	/* End of 'namespace clusterlib' */

#endif	/* !_DATADISTRIBUTION_H_ */
