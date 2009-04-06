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
 * Grouping of a predicate, mutex, and a conditional.
 */
struct PredMutexCond
{
    PredMutexCond() : pred(false) {}

    /**
     * Signal another thread that is waiting on a predicate.  This is
     * a one time signal .  Make sure that the predicate is set to
     * false before calling either the predWait and predSignal
     * functions on either thread.
     */
    void predSignal()
    {
        Locker l1(&(mutex));
        pred = true;
        cond.signal();
    }

    /**
     * Wait on a predicate to be changed by another thread.  This is a
     * one time wait.  Make sure that the predicate is set to false
     * before calling either the predWait and predSignal functions on
     * either thread.
     */
    void predWait()
    {
        mutex.acquire();
        while (pred == false) {
            cond.wait(mutex);
        }
        mutex.release();
    }

    /**
     * Has the predicate been satified?
     */
    bool pred;

    /**
     * Used with conditional
     */
    Mutex mutex;

    /**
     * Used to signal betwen threads
     */
    Cond cond;
};

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
     * Try to lock this NotifyableImpl.  The NotifyableImpl cannot be the
     * Root.  The lock can be used to prevent any process from
     * interfering with operations on this NotifyableImpl.
     *
     * @param ntp is the NotifyableImpl to be locked.
     * @throw ClusterException if the NotifyableImpl doesn't exist
     */
    void acquire(NotifyableImpl *ntp);

    /**
     * Try to unlock this NotifyableImpl.
     *
     * @param ntp is the NotifyableImpl to be unlocked.
     * @throw ClusterException if there is an unrecoverable problem
     */
    void release(NotifyableImpl *ntp);

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
