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
        : m_priority(-1),
          m_notifyable(NULL) {}

    ShardTreeData(int32_t priority, Notifyable *ntp) 
        : m_priority(priority),
          m_notifyable(ntp) {}

    int32_t getPriority() const { return m_priority; }

    Notifyable *getNotifyable() { return m_notifyable; }

    bool operator==(ShardTreeData &rhs)
    {
        if ((getPriority() == rhs.getPriority()) &&
            (getNotifyable() == rhs.getNotifyable())) {
            return true;
        }
        else {
            return false;
        }
    }
  private:
    /** The priority of this shard */
    int32_t m_priority;

    /** The pointer to the Notifyable */
    Notifyable *m_notifyable;
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
    virtual std::vector<Notifyable *> getNotifyables(const Key &key);

    virtual std::vector<Notifyable *> getNotifyables(HashRange hashedKey);

    virtual uint32_t getCount();

    virtual bool isCovered();

    virtual std::vector<HashRange> splitHashRange(int32_t numShards);

    virtual void insert(HashRange start,
                        HashRange end,
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
     * Marshall the shards into a JSONValue for publishing.
     *
     * @return An JSON array of the shards.
     */
    json::JSONValue::JSONArray marshallShards();

    /**
     * unmarshall a stringified sequence of shards into this
     * object. The shards are stored as a JSONArray of JSONArrays
     * (begin, end, notifyablekey, priority)
     *
     * @param encodedJsonArr The encoded JSON array of shards (each shard 
     *        is a JSON array as well)
     */
    void unmarshallShards(const std::string &encodedJsonArr);
    
  private:
    /**
     * Stores all the shards in this object
     */
    IntervalTree<HashRange, ShardTreeData> m_shardTree;

    /**
     * The number of shards in the tree
     */
    int32_t m_shardTreeCount;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDSHARDSIMPL_H_ */
