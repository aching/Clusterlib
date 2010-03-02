/*
 * datadistribution.h --
 *
 * Interface of class DataDistribution; it represents a data
 * distribution (mapping from a key or a HashRange to a notifyable) in
 * clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_DATADISTRIBUTION_H_
#define _CL_DATADISTRIBUTION_H_

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
     * Find the Notifyables that the key maps to.  Sorted by priority (high
     * to low)
     *
     * @param key the key to find.
     * @return the vector of notifyable pointer that have this key
     */
    virtual std::vector<const Notifyable *> getNotifyables(const Key &key) = 0;

    /**
     * Find the Notifyables that the hashedKey maps to.  Sorted by priority
     * (high to low)
     *
     * @param hashedKey the hashed key to find
     * @return the vector of notifyable pointer that have this hashed key
     */
    virtual std::vector<const Notifyable *> getNotifyables(
        HashRange hashedKey) = 0;
    
    /**
     * Return the number of shards in this cached data distribution.
     * 
     * @return the number of shards at this time (could change
     *         immediately after this function return if the distributed 
     *         lock is not held)
     */
    virtual uint32_t getShardCount() = 0;

    /**
     * Is the distribution covered (at the time of checking)?  This
     * operation is atomic, but the shards may change just after the
     * function returns if the distributed lock is not held.
     *
     * @return true if the entire HashRange is covered (could change
     *         immediately after this function return if the distributed 
     *         lock is not held)
     */
    virtual bool isCovered() = 0;

    /**
     * Split the HashRange into a fixed number of shards.  Users can
     * use this helper function in association with insertShard to
     * create a simple data distribution.  It is meant as a
     * convenience function (shards do not have to be the same size).
     *
     * @param numShards the number of shards to split the HashRange range into
     * @return the vector of lower bound HashRange.  The upper bound
     *         is the next lower bound HashRange in the vector minus 1.
     */
    virtual std::vector<HashRange> splitHashRange(int32_t numShards) = 0;

    /**
     * Add a shard to this data distribution.  The changes are not
     * propagated to the repository until after a publish() is
     * successful.
     *
     * @param start the start of the range (inclusive)
     * @param end the end of the range (inclusive)
     * @param notifyable the notifyable that will handle this range
     * @param priority the priority of this shard (-1 is reserved, do not use)
     */
    virtual void insertShard(HashRange start,
                             HashRange end,
                             Notifyable *ntp,
                             int32_t priority = 0) = 0;
    
    /**
     *  Publish any changes to the clusterlib repository.  An
     *  exception will be thrown if any of the Shards have NULL
     *  notifyables.  PublishVersionException will be thrown if the
     *  versions do not match.  In that case, the user should catch
     *  the exception, unlock the DataDistribution and try again.
     */
    virtual void publish() = 0;

    /**
     * Get all the shards in this data distribution ordered by the
     * start range that match the correct notifyable and/or priority if
     * specified.
     *
     * @param notifyable the notifyable to filter on (NULL turns off filter)
     * @param priority the priority filter (-1 turns off filter)
     * @return a vector of shards in the distribution
     */
    virtual std::vector<Shard> getAllShards(const Notifyable *ntp = NULL,
                                            int32_t priority = -1) = 0;

    /**
     * Remove a shard (administrative).  Needs to be published to make
     * the changes pushed to the repository.
     *
     * @param shard the shard to be removed from the data distribution
     * @return true if the shard was removed (false if not found)
     */
    virtual bool removeShard(Shard &shard) = 0;

    /**
     * Remove all shard from the local cached object (need to
     * publish()) afterward to push this change to the repository.
     */
    virtual void clear() = 0;

    /**
     * Get the current version of the shards (useful for out-of-band
     * communication with other clients)
     *
     * @return the version of the shards
     */
    virtual int32_t getVersion() = 0;

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistribution() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_DATADISTRIBUTION_H_ */
