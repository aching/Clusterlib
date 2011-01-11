/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_DISTRIBUTEDLOCKS_H_
#define	_CL_DISTRIBUTEDLOCKS_H_

namespace clusterlib {

/**
 * All distributed locks are managed here.
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
     * @param distributedLockType Lock type
     * @return true if the lock was acquired, false otherwise
     * @throw Exception if the Notifyable doesn't exist
     */
    void acquire(const boost::shared_ptr<Notifyable> &pNotifyableSP,
                 const std::string &lockName,
                 DistributedLockType distributedLockType);

    /**
     * Try to lock this Notifyable.  The Notifyable cannot be the
     * Root.  The lock can be used to prevent any thread from
     * interfering with operations on this NotifyableImpl.
     *
     * @param msecTimeout -1 for wait forever, 0 for return immediately, 
     *        otherwise the number of milliseconds to wait for the lock.
     * @param pNotifyableSP is the Notifyable to be locked.
     * @param lockName is the name of the lock
     * @param distributedLockType Lock type
     * @return true if the lock was acquired, false otherwise
     * @throw Exception if the Notifyable doesn't exist
     */
    bool acquireWaitMsecs(int64_t msecTimeout, 
                          const boost::shared_ptr<Notifyable> &pNotifyableSP, 
                          const std::string &lockName,
                          DistributedLockType distributedLockType);

    /**
     * Try to lock this Notifyable.  The Notifyable cannot be the
     * Root.  The lock can be used to prevent any thread from
     * interfering with operations on this NotifyableImpl.
     *
     * @param usecTimeout -1 for wait forever, 0 for return immediately, 
     *        otherwise the number of microseconds to wait for the lock.
     * @param pNotifyableSP is the Notifyable to be locked.
     * @param lockName is the name of the lock
     * @param distributedLockType Lock type
     * @return true if the lock was acquired, false otherwise
     * @throw Exception if the Notifyable doesn't exist
     */
    bool acquireWaitUsecs(int64_t usecTimeout, 
                          const boost::shared_ptr<Notifyable> &pNotifyableSP, 
                          const std::string &lockName,
                          DistributedLockType distributedLockType);

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
     * @return DistributedLockType or -1 if it does not have the lock
     */
    bool hasLock(
        const boost::shared_ptr<Notifyable> &pNotifyableSP, 
        const std::string &lockName,
        DistributedLockType *pDistributedLockType);

    /**
     * Get the current lock owner information.  This is mainly for
     * debugging, since this information can change at any point.
     *
     * @param pNotifyableSP the Notifyable that is being checked
     * @param lockName the name of the lock
     * @param pId If a valid pointer and an owner exists, the id of 
     *        the owner
     * @param pDistributedLockType DistributedLockType of owner if exists
     *        since the epoch when the owner tried to become the owner
     * @param pMsecs if a valid pointer and an owner exists, the msecs
     *        since the epoch when the owner tried to become the owner
     * @return true if there is an owner of the lock
     */
    bool getInfo(const boost::shared_ptr<Notifyable> &pNotifyableSP,
                 const std::string &lockName,
                 std::string *pId = NULL,
                 DistributedLockType *pDistributedLockType = NULL,
                 int64_t *pMsecs = NULL);

    /**
     * Get the map that is used to signal threads trying to acquire
     * locks.  Note that this should be used in conjunction with
     * getWaitMapLock.
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
    const Mutex &getWaitMapLock() const;

  private:
    /**
     * Private access to mp_ops
     */
    FactoryOps *getOps();

    /**
     * The sequenceKey is a string that has a DistributedLockType in
     * it and two SEQUENCE_SPLIT separating it.
     *
     * @param sequenceKey Input ket to find the DistributedLockType
     * @return DistributedLockType of the sequenceKey
     */
    DistributedLockType getDistributedLockType(const std::string &sequenceKey);

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

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_DISTRIBUTEDLOCKS_H_ */
