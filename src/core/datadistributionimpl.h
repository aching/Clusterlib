/*
 * datadistributionimpl.h --
 *
 * Definition of class DataDistribution; it represents a data
 * distribution (mapping from a key to a notifyable) in clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_DATADISTRIBUTIONIMPL_H_
#define _CL_DATADISTRIBUTIONIMPL_H_

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
 * Definition of class DataDistribution.
 */
class DataDistributionImpl
    : public virtual DataDistribution, 
      public virtual NotifyableImpl
{
  public:
    virtual std::vector<const Notifyable *> getNotifyables(const Key &key);

    virtual std::vector<const Notifyable *> getNotifyables(HashRange hashedKey);

    virtual uint32_t getShardCount();

    virtual bool isCovered();

    virtual std::vector<HashRange> splitHashRange(int32_t numShards);

    virtual void insertShard(HashRange start,
                             HashRange end,
                             Notifyable *ntp,
                             int32_t priority = 0);

    virtual void publish();

    virtual std::vector<Shard> getAllShards(
        const Notifyable *ntp = NULL,
        int32_t priority = -1);

    virtual bool removeShard(Shard &shard);

    virtual void clear();

    virtual int32_t getVersion() 
    {
        throwIfRemoved();

	return m_version; 
    }

  public:
    /**
     * Constructor used by Factory.
     */
    DataDistributionImpl(FactoryOps *fp,
                         const std::string &key,
                         const std::string &name,
                         GroupImpl *parentGroup);

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistributionImpl();

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries();


    /**
     * Set the version of this object
     *
     * @param version the version to set
     */
    void setVersion(int32_t version) 
    { 
        throwIfRemoved();

	m_version = version; 
    }

    /**
     * Update the cached data distribution from the repository. Return
     * true if there was actually a change in the value (as determined
     * by comparing stored and new version).
     */
    bool update();

  private:
    /**
     * Make the default constructor private so it cannot be called.
     */
    DataDistributionImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException(
            "Someone called the DataDistributionImpl "
            "default constructor!");
    }

  private:
    /**
     * Unmarshall a string into this data distribution.
     */
    void unmarshall(const std::string &marshalledDist);

    /**
     * Marshall a data distribution into a string.
     */
    std::string marshall();

    /**
     * The version number of this DataDistribution
     */
    int32_t m_version;

    /**
     * Stores all the shards in this object
     */
    IntervalTree<HashRange, ShardTreeData> m_shardTree;

    /**
     * The number of shards in the tree
     */
    int32_t m_shardTreeCount;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_DATADISTRIBUTIONIMPL_H_ */
