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

/*
 * Definition of class DataDistribution.
 */
class DataDistribution
    : public virtual Notifyable
{
  public:
    /*
     * Enum of hash functions to use.
     */
    enum HashFunctions {
        DD_HF_USERDEF	= -1,
        DD_HF_MD5	= 0,
        DD_HF_JENKINS	= 1,
        DD_HF_END	= 2
    };

    /*
     * Constants for identifying the various parts of a shard definition.
     */
    static const int SC_LOWBOUND_IDX;
    static const int SC_HIBOUND_IDX;
    static const int SC_APPNAME_IDX;
    static const int SC_GROUPNAME_IDX;
    static const int SC_NODENAME_IDX;

    /*
     * Define the types associated with a hash function.
     */
    typedef unsigned long long HashRange;
    typedef int32_t            HashFunctionId;
    typedef HashRange (HashFunction)(const string &key);

    /*
     * Retrieve the application object in which this
     * distribution is contained.
     */
    Application *getApplication() { return mp_app; }

    /*
     * Find node the work identified by the key or
     * hash value belongs to.
     */
    Node *map(const string &key) throw(ClusterException);
    Node *map(HashRange hash) throw(ClusterException);

    /*
     * Hash a key.
     */
    HashRange hashWork(const string &key);

    /*
     * Return the manual override string that matches this
     * key if one exists (returns the first one found, in
     * an unspecified order) or the empty string if none.
     */
    string matchesManualOverride(const string &key)
        throw(ClusterException);

    /*
     * Return the number of shards in this data distribution.
     */
    uint32_t getShardCount() { return m_shards.size(); }

    /*
     * Is the distribution covered (at the time of checking)?
     */
    bool isCovered() throw(ClusterException);

    /*
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

    /*
     * Assign new shards.
     */
    void setShards(vector<unsigned long long> &upperBounds)
        throw(ClusterException);

    /*
     * Get the shard index for a work item, or for a hash value.
     */
    uint32_t getShardIndex(const string &workItem)
        throw(ClusterException);
    uint32_t getShardIndex(HashRange v)
        throw(ClusterException);

    /*
     * Get all info out of a shard.
     */
    Notifyable *getShardDetails(uint32_t shardIndex,
                                unsigned long long *low = NULL,
                                unsigned long long *hi = NULL,
                                bool *isForwarded = NULL)
        throw(ClusterException);

    /*
     * Assign a node to a shard.
     */
    void assignNodeToShard(uint32_t shardIndex,
                           Node *np)
        throw(ClusterException);

    /*
     * Forward a shard to a different data distribution.
     */
    void forwardShard(uint32_t shardIndex,
                      DataDistribution *dp)
        throw(ClusterException);

    /*
     * Set a manual override for a node.
     */
    void setManualOverride(const string &pattern,
                           Node *np)
        throw(ClusterException);

    /*
     * Set a manual override for a distribution.
     */
    void forwardManualOverride(const string &pattern,
                               DataDistribution *dp)
        throw(ClusterException);

    /*
     * Remove a manual override.
     */
    void removeManualOverride(const string &pattern)
        throw(ClusterException);

    /*
     * Is this data distribution modified, i.e. does it need
     * to be published to the clusterlib repository?
     */
    bool isModified() { return m_modified; }

  protected:
    /*
     * Friend declaration of Factory so that it can call the
     * protected constructor.
     */
    friend class Factory;

    /*
     * Constructor used by Factory.
     */
    DataDistribution(Application *app,
                     const string &name,
                     const string &key,
                     FactoryOps *f,
                     HashFunction *fn = NULL);

    /*
     * Update the distribution.
     */
    void updateCachedRepresentation() throw(ClusterException);

  private:
    /*
     * Forward declaration of class Shard.
     */
    class Shard;

    /*
     * A list of shards.
     */
    typedef vector<Shard *> ShardList;

    /*
     * Make the default constructor private so it cannot be called.
     */
    DataDistribution()
        : Notifyable(NULL, "", "")
    {
        throw ClusterException("Someone called the DataDistribution "
                               "default constructor!");
    }

    /*
     * Unmarshall a string into this data distribution.
     */
    void unmarshall(const string &marshalledDist) throw(ClusterException);

    /*
     * Unmarshall a stringified sequence of shards.
     */
    void unmarshallShards(const string &marshalledShards, ShardList &l)
        throw(ClusterException);

    /*
     * Unmarshall a stringified sequence of manual overrides.
     */
    void unmarshallOverrides(const string &marshalledOverrides, NodeMap &m)
        throw(ClusterException);

    /*
     * Marshall a data distribution into a string.
     */
    string marshall();

    /*
     * Marshall shards into a string.
     */
    string marshallShards();

    /*
     * Marshall manual overrides into a string.
     */
    string marshallOverrides();

  private:
    /*
     * Definition of class Shard.
     */
    class Shard
    {
      public:
        /*
         * Get the info out of the shard.
         */
        DataDistribution *getDistribution() { return mp_dist; }
        HashRange beginRange() { return m_beginRange; }
        HashRange endRange() { return m_endRange; }
        string getNodeKey() { return m_nodeKey; }

        /*
         * Get the node, load it if it's not yet loaded.
         */
        Node *getNode();

        /*
         * Return the Node * if the work belongs to this shard,
         * NULL otherwise.
         */
        Node *contains(HashRange hash);

        /*
         * Decide whether this piece of work belongs to
         * this shard.
         */
        bool covers(const string &key);

        /*
         * Constructor.
         */
        Shard(DataDistribution *dist,
              Node *node,
              HashRange beginRange,
              HashRange endRange)
            : mp_dist(dist),
              mp_node(node),
              m_beginRange(beginRange),
              m_endRange(endRange),
              m_nodeKey(string(""))
        {
        }

        /*
         * Constructor.
         */
        Shard(DataDistribution *dist,
              string nodeKey,
              HashRange beginRange,
              HashRange endRange)
            : mp_dist(dist),
              mp_node(NULL),
              m_beginRange(beginRange),
              m_endRange(endRange),
              m_nodeKey(nodeKey)
        {
        }

        /*
         * Reassign this shard to a different node.
         */
        void reassign(Node *newNode) 
        {
            mp_node = newNode;
            if (mp_node == NULL) {
                m_nodeKey = "";
            } else {
                m_nodeKey = mp_node->getKey();
            }
        }
        void reassign(const string nodeKey)
        {
            mp_node = NULL;
            m_nodeKey = nodeKey;
        }

      private:
        /*
         * Make the default constructor private so it cannot be called.
         */
        Shard()
        {
            throw ClusterException("Someone called the Shard "
                                   "default constructor!");
        }

      private:
        /*
         * The data distribution this shard belongs to.
         */
        DataDistribution *mp_dist;

        /*
         * The node that this shard is assigned to.
         */
        Node *mp_node;

        /*
         * The bounds for this shard. The range is beginRange <-> endRange.
         */
        HashRange m_beginRange;
        HashRange m_endRange;

        /*
         * The key of the node -- used in case the pointer is not available.
         */
        string m_nodeKey;
    };

    /*
     * The application object for the application that contains
     * this distribution.
     */
    Application *mp_app;

    /*
     * The manual overrides for this data distribution.
     */
    NodeMap m_manualOverrides;

    /*
     * Which hash function to use.
     */
    int m_hashFnIndex;

    /*
     * If using a user supplied hash function, store
     * a pointer to it here. If using a built-in
     * hash function, store it here from the class-static
     * array.
     */
    HashFunction *mp_hashFnPtr;

    /*
     * Class-static variable holding the array of
     * hash functions.
     */
    static HashFunction *s_hashFunctions[];

    /*
     * The shards in this data distribution.
     */
    ShardList m_shards;

    /*
     * Is this data distribution modified?
     */
    bool m_modified;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_DATADISTRIBUTION_H_ */
