/*
 * distributedlocks.h --
 *
 * The definition of DistributedLocks
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_DISTRIBUTEDLOCKS_H_
#define	_DISTRIBUTEDLOCKS_H_

namespace clusterlib
{

/**
 * Definition of DistributedLocks
 */
class DistributedLocks
{
  public:
    /**
     * Constructor used by FactoryOps.
     */
    DistributedLocks(FactoryOps *factoryOps) 
        : mp_ops(factoryOps) {}

    /**
     * Try to lock this Notifyable.  The Notifyable cannot be the
     * Root.  The lock can be used to prevent any process from
     * interfering with operations on this NotifyableImpl.
     *
     * @param ntp is the Notifyable to be locked.
     * @param lockName is the name of the lock
     * @throw Exception if the Notifyable doesn't exist
     */
    void acquire(Notifyable *ntp, const std::string &lockName);

    /**
     * Try to unlock this Notifyable.
     *
     * @param ntp is the Notifyable to be unlocked.
     * @param lockName is the name of the lock
     * @throw Exception if there is an unrecoverable problem
     */
    void release(Notifyable *ntp, const std::string &lockName);

    /*
     * Does this thread have a lock on this Notifyable?
     *
     * @param ntp the Notifyable that is being checked
     * @param lockName the name of the lock
     */
    bool hasLock(Notifyable *ntp, const std::string &lockName);

    /**
     * Get the map that is used to signal threads trying to acquire locks.
     *
     * @return pointer to m_waitMap
     */
    WaitMap *getWaitMap() 
    {
        return &m_waitMap; 
    }
    
    /**
     * Get the lock that protets m_waitMap.
     *
     * @return pointer to m_waitMapLock
     */
    Mutex *getWaitMapLock() { return &m_waitMapLock; }

  private:
    /**
     * Private access to mp_ops
     */
    FactoryOps *getOps() { return mp_ops; }
    
  private:
    /**
     * Does all the factory operations
     */
    FactoryOps *mp_ops;

    /**
     * Map where key is full lock key to whether ready (true == ready).
     */
    WaitMap m_waitMap;
    
    /**
     * Lock that protects m_waitMap
     */
    Mutex m_waitMapLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_DISTRIBUTEDLOCKS_H_ */
