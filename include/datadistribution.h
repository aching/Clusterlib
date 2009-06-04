/*
 * datadistribution.h --
 *
 * Interface of class DataDistribution; it represents a data
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
    virtual Node *map(const std::string &key) = 0;
    
    /**
     * Hash a key.
     */
    virtual HashRange hashWork(const std::string &key) = 0;

    /**
     * Return the manual override string that matches this
     * key if one exists (returns the first one found, in
     * an unspecified order) or the empty string if none.
     */
    virtual std::string matchesManualOverride(const std::string &key) = 0;

    /**
     * Return the number of shards in this data distribution.
     */
    virtual uint32_t getShardCount() = 0;

    /**
     * Is the distribution covered (at the time of checking)?
     */
    virtual bool isCovered() = 0;

    /**
     * Get/set the hash function to use.
     */
    virtual HashFunctionId getHashFunctionIndex() = 0;
    virtual void setHashFunctionIndex(HashFunctionId idx) = 0;
    virtual void setHashFunction(HashFunction *fn) = 0;
        
    /**
     * Assign new shards.
     */
    virtual void setShards(std::vector<HashRange> &upperBounds) = 0;
    
    /**
     * Get the shard index for a work item, or for a hash value.
     */
    virtual uint32_t getShardIndex(const std::string &workItem) = 0;
    virtual uint32_t getShardIndex(HashRange v) = 0;

    /**
     * Get all info out of a shard.
     */
    virtual Notifyable *getShardDetails(uint32_t shardIndex,
                                        HashRange *low = NULL,
                                        HashRange *hi = NULL,
                                        bool *isForwarded = NULL) = 0;

    /**
     * Reassign a shard to a different notifyable.
     */
    virtual void reassignShard(uint32_t shardIndex, Notifyable *ntp) = 0;
    virtual void reassignShard(uint32_t shardIndex, 
                               const std::string &key) = 0;

    /**
     * Assign - or reassign - a manual override to a different
     * notifyable.
     */
    virtual void reassignManualOverride(const std::string &pattern,
                                        Notifyable *ntp) = 0;
    virtual void reassignManualOverride(const std::string &pattern,
                                        const std::string &key) = 0;

#if TO_BE_IMPLEMENTED
    /**
     * Remove a manual override.
     */
    virtual bool removeManualOverride(const std::string &pattern) = 0;
#endif

    /**
     *  Publish any changes to the clusterlib repository.
     */
    virtual void publish() = 0;

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistribution() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_DATADISTRIBUTION_H_ */
