/*
 * cacheddataimpl.h --
 *
 * Definition of class CachedDataImpl; it implements the common
 * functionality for data that is stored in the clusterlib repository.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDDATAIMPL_H_
#define _CL_CACHEDDATAIMPL_H_

namespace clusterlib {

/**
 * Definition of class CachedDataImpl
 */
class CachedDataImpl
    : public virtual CachedData
{
  public:
    virtual void reset() 
    {
        loadDataFromRepository(false);
    }

    virtual int32_t getVersion();

    virtual void getStats(int64_t *czxid = NULL,
                          int64_t *mzxid = NULL,
                          int64_t *ctime = NULL,
                          int64_t *mtime = NULL,
                          int32_t *version = NULL,
                          int32_t *cversion = NULL,
                          int32_t *aversion = NULL,
                          int64_t *ephemeralOwner = NULL,
                          int32_t *dataLength = NULL,
                          int32_t *numChildren = NULL,
                          int64_t *pzxid = NULL);
    
  public:
    /**
     * Constructor.
     * 
     * @param notifyable Notifyable that is associated with this cached data
     */
    explicit CachedDataImpl(NotifyableImpl *notifyable);

    /**
     * Destructor.
     */
    virtual ~CachedDataImpl() {}

    /**
     * Get data from the repository and put it in the cached
     * (replacing anything already there).  Set the watches for any
     * changes in the repository data.
     *
     * @param setWatchesOnly if set, do load copy the data from the repository
     *        into the cache, just set the watches only
     */
    virtual void loadDataFromRepository(bool setWatchesOnly) = 0;

    /**
     * Get the lock for cached data.
     */
    Mutex &getCachedDataLock() 
    {
        return m_cachedDataLock;
    }

    /**
     * Get the notifyable that contains this cached data
     */
    boost::shared_ptr<NotifyableImpl> getNotifyable();

    /**
     * Get the factory ops
     */
    FactoryOps *getOps();

    /**
     * Set the stat object.
     *
     * @param stat The new Zookeeper stats for this object.
     */
    void setStat(Stat stat)
    {
        m_stat = stat;
    }

    /**
     * Should the update happen?  Our current version and our new
     * version should indicate whether this is the case.  If the Stat
     * object if "newer" it will be updated.  Otherwise, this will
     * return false.
     *
     * @param stat The new candidate Stat object
     * @return True if the candidate stat object is accepted, false otherwise.
     */
    bool updateStat(const Stat &stat);

  private:
    /**
     * No copying.
     */
    CachedDataImpl(const CachedDataImpl &);

    /**
     * No assigning.
     */
    CachedDataImpl &operator=(const CachedDataImpl &);

  private:
    /**
     * A mutex to protect any cached data.
     */
    Mutex m_cachedDataLock;

    /**
     * The pointer to the NotifyableImpl associated with this cached
     * data.  This cannot be a boost::weak_ptr or boost::shared_ptr
     * since this is initialized as part of the constructor.
     * "There must exist at least one shared_ptr instance p that owns t"
     */
    NotifyableImpl *m_notifyable;
    
    /**
     * Statistics of this cached data
     */
    Stat m_stat;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NODE_H_ */
