/*
 * cachedshards.h --
 *
 * Definition of class CachedShards; it represents the cached
 * shards of a DataDistribution.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDSHARDS_H_
#define _CL_CACHEDSHARDS_H_

namespace clusterlib {

/**
 * Definition of class CachedShards
 */
class CachedShards
    : public virtual CachedData
{
  public:
    /**
     * Get the name of the hash range for these shards.
     *
     * @return The name of the HashRange.
     */
    virtual std::string getHashRangeName() = 0;

    /**
     * Find the Notifyables that the key maps to.  Sorted by priority (high
     * to low)
     *
     * @param hashRange The HashRange to find.
     * @return the vector of Notifyable pointer that have this key
     */
    virtual NotifyableList getNotifyables(const HashRange &hashRange) = 0;
    
    /**
     * Return the number of shards in this cached data distribution.
     * 
     * @return The number of shards at this time (could change
     *         immediately after this function return if the distributed 
     *         lock is not held)
     */
    virtual uint32_t getCount() = 0;

    /**
     * Is the distribution covered (at the time of checking)?  This
     * operation is atomic, but the shards may change just after the
     * function returns if the distributed lock is not held.  It is
     * also expensive, since it must go through all the shards and get
     * each Notifyable.
     *
     * @return true if the entire HashRange is covered (could change
     *         immediately after this function return if the distributed 
     *         lock is not held)
     */
    virtual bool isCovered() = 0;

    /**
     * Add a shard to this data distribution.  The changes are not
     * propagated to the repository until after a publish() is
     * successful.
     *
     * @param start the start of the range (inclusive)
     * @param end the end of the range (inclusive)
     * @param notifyableSP Notifyable that will handle this range
     * @param priority the priority of this shard (-1 is reserved, do not use)
     */
    virtual void insert(const HashRange &start,
                        const HashRange &end,
                        const boost::shared_ptr<Notifyable> &notifyableSP,
                        int32_t priority = 0) = 0;
    
    /**
     * Get all the shards in this data distribution ordered by the
     * start range that match the correct notifyable and/or priority if
     * specified.
     *
     * @param notifyableSP Notifyable to filter on (NULL turns off filter)
     * @param priority the priority filter (-1 turns off filter)
     * @return a vector of shards in the distribution
     */
    virtual std::vector<Shard> getAllShards(
        const boost::shared_ptr<Notifyable> &notifyableSP = 
        boost::shared_ptr<Notifyable>(),
        int32_t priority = -1) = 0;

    /**
     * Remove a shard (administrative).  Needs to be published to make
     * the changes pushed to the repository.
     *
     * @param shard the shard to be removed from the data distribution
     * @return true if the shard was removed (false if not found)
     */
    virtual bool remove(Shard &shard) = 0;

    /**
     * Remove all shard from the local cached object (need to
     * publish()) afterward to push this change to the repository. Until
     * publish() is called, this change is only local.
     */
    virtual void clear() = 0;

    /**
     * Destructor.
     */
    virtual ~CachedShards() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDSHARDS_H_ */
