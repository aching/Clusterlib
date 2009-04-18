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
    PredMutexCond() 
        : pred(false),
          refCount(0) {}

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

    /**
     * Could be more than one thread waiting on this conditional
     */
    int32_t refCount;
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
     * Try to lock this Notifyable.  The Notifyable cannot be the
     * Root.  The lock can be used to prevent any process from
     * interfering with operations on this NotifyableImpl.
     *
     * @param ntp is the Notifyable to be locked.
     * @throw Exception if the Notifyable doesn't exist
     */
    void acquire(Notifyable *ntp);

    /**
     * Try to unlock this Notifyable.
     *
     * @param ntp is the Notifyable to be unlocked.
     * @throw Exception if there is an unrecoverable problem
     */
    void release(Notifyable *ntp);

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
