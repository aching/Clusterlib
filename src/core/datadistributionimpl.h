/*
 * datadistributionimpl.h --
 *
 * Definition of class DataDistribution; it represents a data
 * distribution (mapping from a key to a node) in clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_DATADISTRIBUTIONIMPL_H_
#define _DATADISTRIBUTIONIMPL_H_

namespace clusterlib
{

/**
 * The object stored in the interval tree with each node.
 */
class ShardTreeData 
{
  public:
    ShardTreeData()
        : m_priority(-1),
          m_node(NULL) {}

    ShardTreeData(int32_t priority, const Node *node) 
        : m_priority(priority),
          m_node(node) {}

    int32_t getPriority() const { return m_priority; }

    const Node *getNode() const { return m_node; }

    bool operator==(const ShardTreeData &rhs) const
    {
        if ((getPriority() == rhs.getPriority()) &&
            (getNode() == rhs.getNode())) {
            return true;
        }
        else {
            return false;
        }
    }
  private:
    /** The priority of this shard */
    int32_t m_priority;

    /** The pointer to the Node */
    const Node *m_node;
};


/**
 * Definition of class DataDistribution.
 */
class DataDistributionImpl
    : public virtual DataDistribution, 
      public virtual NotifyableImpl
{
  public:
    virtual std::vector<const Node *> getNodes(const Key &key);

    virtual std::vector<const Node *> getNodes(HashRange hashedKey);

    virtual uint32_t getShardCount();

    virtual bool isCovered();


    virtual std::vector<HashRange> splitHashRange(int32_t numShards) 
    {
        return std::vector<HashRange>();
    }

    virtual void insertShard(HashRange start,
                             HashRange end,
                             const Node *node,
                             int32_t priority = 0);

    virtual void publish();

    virtual std::vector<Shard> getAllShards(const Node *node = NULL,
                                            int32_t priority = -1);

    virtual bool removeShard(const Shard &shard);

    virtual void clear();

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
     * Retrieve the current version number of the
     * shards in this data distribution.
     */
    int32_t getVersion() 
    {
        throwIfRemoved();

	return m_version; 
    }

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

};	/* End of 'namespace clusterlib' */

#endif	/* !_DATADISTRIBUTIONIMPL_H_ */
