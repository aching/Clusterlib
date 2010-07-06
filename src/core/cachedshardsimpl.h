/*
 * cachedshardsimpl.h --
 *
 * Implementation of class CachedShardsImpl; it represents the cached
 * shards of a DataDistribution.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDSHARDSIMPL_H_
#define _CL_CACHEDSHARDSIMPL_H_

namespace clusterlib
{

/**
 * The object stored in the interval tree with each notifyable.
 */
class ShardTreeData 
{
  public:
    ShardTreeData()
        : m_priority(-1) {}

    ShardTreeData(int32_t priority, const std::string &ntpKey) 
        : m_priority(priority),
          m_notifyableKey(ntpKey) {}

    int32_t getPriority() const { return m_priority; }

    const std::string &getNotifyableKey() const { return m_notifyableKey; }

    bool operator< (const ShardTreeData &rhs) const
    {
        if (getPriority() < rhs.getPriority()) {
            return true;
        }
        else if (getPriority() == rhs.getPriority()) {
            if (getNotifyableKey() < rhs.getNotifyableKey()) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }

    bool operator==(const ShardTreeData &rhs) const
    {
        if ((getPriority() == rhs.getPriority()) &&
            (getNotifyableKey() == rhs.getNotifyableKey())) {
            return true;
        }
        else {
            return false;
        }
    }

    bool operator> (const ShardTreeData &rhs) const
    {
        if (!(*this < rhs) && (!(*this == rhs))) {
            return true;
        }
        else {
            return false;
        }
    }

    /**
     * Print out useful data from <<.
     */
    friend std::ostream & operator<< (std::ostream &stream, 
                                      const ShardTreeData &other)
    {
        stream << "ShardTreeData(priority=" << other.getPriority()
               << ",key=" << other.getNotifyableKey() << ")";

        return stream;
    }

  private:
    /** The priority of this shard */
    int32_t m_priority;

    /** The Notifyable key */
    std::string m_notifyableKey;
};

/**
 * Definition of class CachedShardsImpl
 */
class CachedShardsImpl
    : public virtual CachedDataImpl,
      public virtual CachedShards
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    virtual std::string getHashRangeName();

    virtual std::vector<Notifyable *> getNotifyables(
        const HashRange &hashedKey);

    virtual uint32_t getCount();

    virtual bool isCovered();

    virtual void insert(const HashRange &start,
                        const HashRange &end,
                        Notifyable *ntp,
                        int32_t priority = 0);
    
    virtual std::vector<Shard> getAllShards(const Notifyable *ntp = NULL,
                                            int32_t priority = -1);

    virtual bool remove(Shard &shard);

    virtual void clear();

    /**
     * Constructor.
     */
    explicit CachedShardsImpl(NotifyableImpl *ntp);

    /**
     * Destructor.
     */
    virtual ~CachedShardsImpl();

  private:
    /**
     * Marshal the shards into a JSONValue for publishing.
     *
     * @return An JSON array of the shards.
     */
    json::JSONValue::JSONArray marshalShards();

    /**
     * Unmarshal a stringified sequence of shards into this
     * object. The shards are stored as a JSONArray of JSONArray
     * objects (begin, end, notifyablekey, priority), with an initial
     * JSONString at the front of the JSONArray to denote the
     * HashRange.
     *
     * @param encodedJsonArr The encoded JSON array of shards (each shard 
     *        is a JSON array as well)
     */
    void unmarshalShards(const std::string &encodedJsonArr);

    /**
     * Throw an InvalidMethodException() is the HashRange is
     * UnknownHashRange.
     */
    void throwIfUnknownHashRange();
    
  private:
    /**
     * Stores all the Shard objects for HashRange types that are not
     * UnknownHashRange.  Must be a pointer since the HashRange may
     * change.
     */
    IntervalTree<HashRange &, ShardTreeData> *m_shardTree;

    /**
     * The number of shards in the tree.
     */
    int32_t m_shardTreeCount;

    /**
     * Unsorted storage for UnknownHashRange Shard objects.
     */
    std::vector<Shard> m_unknownShardArr;

    /**
     * Registered HashRange (set implicitly when read, or used).  Can
     * only be set when Shard objects are loaded or during an insert()
     * when there are no Shard objects.
     */
    HashRange *m_hashRange;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDSHARDSIMPL_H_ */
