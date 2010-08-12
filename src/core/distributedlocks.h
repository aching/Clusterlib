/*
 * distributedlocks.h --
 *
 * The definition of DistributedLocks
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_DISTRIBUTEDLOCKS_H_
#define	_CL_DISTRIBUTEDLOCKS_H_

namespace clusterlib {

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
     * Lock this Notifyable.  The Notifyable cannot be the
     * Root.  The lock can be used to prevent any thread from
     * interfering with operations on this NotifyableImpl.
     *
     * @param pNotifyableSP is the Notifyable to be locked.
     * @param lockName is the name of the lock
     * @return true if the lock was acquired, false otherwise
     * @throw Exception if the Notifyable doesn't exist
     */
    void acquire(const boost::shared_ptr<Notifyable> &pNotifyableSP, 
                 const std::string &lockName);

    /**
     * Try to lock this Notifyable.  The Notifyable cannot be the
     * Root.  The lock can be used to prevent any thread from
     * interfering with operations on this NotifyableImpl.
     *
     * @param msecTimeout -1 for wait forever, 0 for return immediately, 
     *        otherwise the number of milliseconds to wait for the lock.
     * @param pNotifyableSP is the Notifyable to be locked.
     * @param lockName is the name of the lock
     * @return true if the lock was acquired, false otherwise
     * @throw Exception if the Notifyable doesn't exist
     */
    bool acquireWaitMsecs(int64_t msecTimeout, 
                          const boost::shared_ptr<Notifyable> &pNotifyableSP, 
                          const std::string &lockName);

    /**
     * Try to lock this Notifyable.  The Notifyable cannot be the
     * Root.  The lock can be used to prevent any thread from
     * interfering with operations on this NotifyableImpl.
     *
     * @param usecTimeout -1 for wait forever, 0 for return immediately, 
     *        otherwise the number of microseconds to wait for the lock.
     * @param pNotifyableSP is the Notifyable to be locked.
     * @param lockName is the name of the lock
     * @return true if the lock was acquired, false otherwise
     * @throw Exception if the Notifyable doesn't exist
     */
    bool acquireWaitUsecs(int64_t usecTimeout, 
                          const boost::shared_ptr<Notifyable> &pNotifyableSP, 
                          const std::string &lockName);

    /**
     * Try to unlock this Notifyable.
     *
     * @param pNotifyableSP is the Notifyable to be unlocked.
     * @param lockName is the name of the lock
     * @throw Exception if there is an unrecoverable problem
     */
    void release(const boost::shared_ptr<Notifyable> &pNotifyableSP, 
                 const std::string &lockName);

    /**
     * Does this thread have a lock on this Notifyable?
     *
     * @param pNotifyableSP the Notifyable that is being checked
     * @param lockName the name of the lock
     * @return true if the thread has the lock
     */
    bool hasLock(const boost::shared_ptr<Notifyable> &pNotifyableSP, 
                 const std::string &lockName);

    /**
     * Get the current lock owner information.  This is mainly for
     * debugging, since this information can change at any point.
     *
     * @param pNotifyableSP the Notifyable that is being checked
     * @param lockName the name of the lock
     * @param id if a valid pointer and an owner exists, the id of 
     *        the owner
     * @param msecs if a valid pointer and an owner exists, the msecs
     *        since the epoch when the owner tried to become the owner
     * @return true if there is an owner
     */
    bool getInfo(const boost::shared_ptr<Notifyable> &pNotifyableSP,
                 const std::string &lockName,
                 std::string *id = NULL,
                 int64_t *msecs = NULL);

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

#endif	/* !_CL_DISTRIBUTEDLOCKS_H_ */
