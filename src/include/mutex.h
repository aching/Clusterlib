/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#ifndef _CL_MUTEX_H_
#define _CL_MUTEX_H_

#include "clusterlibexceptions.h"
#include "timerservice.h"

namespace clusterlib {

/*
 * Forward decl of Cond.
 */
class Cond;

/**
 * Encapsulates a mutex object.
 */
class Mutex
{
    friend class Cond;

  public:
    /**
     * Constructor.
     */
    Mutex() 
        : refCount(0)
    {
        pthread_mutexattr_init(&m_mutexAttr);
        pthread_mutexattr_settype(&m_mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&mutex, &m_mutexAttr);
    }

    /**
     * Destructor.
     */
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
        pthread_mutexattr_destroy(&m_mutexAttr);
    }

    /**
     * Acquire the mutex.
     */
    void acquire();

    /**
     * Release the mutex.
     */
    void release();
    int32_t tryLock()
    {
        return pthread_mutex_trylock(&mutex);
    }
    int32_t getRefCount() const
    {
        return refCount;
    }
    void incrRefCount()
    {
        refCount++;
    }
    void decrRefCount()
    {
        refCount--;
    }
    
  private:
    void lock();

    void unlock();

    /**
     * No copy constructor allowed.
     */
    Mutex(const Mutex &);

    /**
     * No assignment allowed.
     */
    Mutex & operator=(const Mutex &);

  private:
    pthread_mutex_t mutex;
    pthread_mutexattr_t m_mutexAttr;
    int32_t refCount;
};

/**
 * Encapsulates a conditional variable object.
 */
class Cond
{
  public:
    Cond()
    {
        static pthread_condattr_t attr;
        static bool inited = false;
        if(!inited) {
            inited = true;
            pthread_condattr_init(&attr);
        }
        pthread_cond_init(&m_cond, &attr);
    }
    ~Cond()
    {
        pthread_cond_destroy(&m_cond);
    }

    /**
     * Wait unconditionally for the conditional to be signaled.
     *
     * @param mutex the mutex to wait on
     */
    void wait(Mutex& mutex);

    /**
     * Wait for the conditional to be signaled.
     * 
     * @param mutex the mutex to wait on
     * @param usecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * return true if the conditional was signaled, false if timed out
     */
    bool waitUsecs(Mutex& mutex, int64_t usecTimeout);
    
    /**
     * Wait for the conditional to be signaled.
     * 
     * @param mutex the mutex to wait on
     * @param msecTimeout the amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * return true if the conditional was signaled, false if timed out
     */
    bool waitMsecs(Mutex& mutex, int64_t msecTimeout);
    
    void signal()
    {
        pthread_cond_signal(&m_cond);
    }

    void signal_all()
    {
        pthread_cond_broadcast(&m_cond);
    }

  private:
    /**
     * No copy constructor allowed.
     */
    Cond(const Cond &);

    /**
     * No assignment allowed.
     */
    Cond & operator=(const Cond &);

  private:
    pthread_cond_t m_cond;
};

/**
 * A wrapper class for {@link Mutex} and {@link Cond}.
 */
class Lock
{
  public:
    void lock()
    {
        m_mutex.acquire();
    }
        
    void unlock()
    {
        m_mutex.release();
    }
        
    void wait()
    {
        m_cond.wait(m_mutex);
    }

    /**
     * Wait for the conditional to be signaled.
     * 
     * @param usecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * return true if the conditional was signaled, false if timed out
     */
    bool waitUsecs(int64_t usecTimeout)
    {
        return m_cond.waitUsecs(m_mutex, usecTimeout);
    }

    /**
     * Wait for the conditional to be signaled.
     * 
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * return true if the conditional was signaled, false if timed out
     */
    bool waitMsecs(int64_t msecTimeout)
    {
        return m_cond.waitMsecs(m_mutex, msecTimeout);
    }
        
    void notify()
    {
        m_cond.signal();
    }

    void lockedWait()
    {
        m_mutex.acquire();
        m_cond.wait(m_mutex);
        m_mutex.release();
    }

    bool lockedWaitUsecs(int64_t usecTimeout)
    {
        bool res;

        m_mutex.acquire();
        res = m_cond.waitUsecs(m_mutex, usecTimeout);
        m_mutex.release();

        return res;
    }

    void lockedNotify()
    {
        m_mutex.acquire();
        m_cond.signal();
        m_mutex.release();
    }

  private:
    /**
     * The mutex.
     */
    Mutex m_mutex;
        
    /**
     * The condition associated with this lock's mutex.
     */
    Cond m_cond;         
};

/**
 * A class that locks a mutex on construction and
 * unlocks the mutex on destruction.
 */
class Locker
{
  public:
    /**
     * Constructor locks the passed mutex.
     * 
     * @param mp pointer to the underlying Mutex
     */
    Locker(Mutex *mp);

    /*
     * Destructor unlocks the mutex.
     */
    ~Locker();

  private:
    /*
     * Make the default constructor private so it
     * cannot be called.
     */
    Locker()
    {
        throw InvalidMethodException("Someone called the "
                                       "Locker default constructor!");
    }

  private:
    /*
     * Hold onto the mutex being locked.
     */
    Mutex *mp_lock;
};

/**
 * Encapsulates a read-write lock object.
 */
class RdWrLock
{
  public:
    /**
     * Constructor.
     */
    RdWrLock();

    /**
     * Destructor.
     */
    ~RdWrLock();

    /**
     * Get the read lock
     */
    void acquireRead();

    /**
     * Get the write lock
     */
    void acquireWrite();

    /**
     * Release lock
     */
    void release();

  private:
    /**
     * No copy constructor allowed.
     */
    RdWrLock(const RdWrLock &);

    /**
     * No assignment allowed.
     */
    RdWrLock & operator=(const RdWrLock &);

  private:
    /**
     * The lock.
     */
    pthread_rwlock_t m_rwlock;    

    /**
     * Lock attributes
     */
    pthread_rwlockattr_t m_rwlockAttr;
};

/**
 * A class that locks a read write lock on construction and unlocks
 * the mutex on destruction.
 */
class RdWrLocker
{
  public:
    /**
     * Type of lock to acquire
     */
    enum RdWrType 
    {
        READLOCK,
        WRITELOCK
    };

    /**
     * Constructor locks the passed mutex.
     */
    RdWrLocker(RdWrLock *lock, RdWrType type) 
        : mp_lock(lock)
    {
        if ((mp_lock == NULL) ||
            ((type != READLOCK) && (type != WRITELOCK))) {
            throw SystemFailureException(
                "RdWrLocker: lock or type invalid!");
        }

        if (type == READLOCK) {
            mp_lock->acquireRead();
        }
        else {
            mp_lock->acquireWrite();
        }
    }

    /**
     * Destructor unlocks the mutex.
     */
    virtual ~RdWrLocker()
    {
        mp_lock->release();
    }

  private:
    /*
     * Make the default constructor private so it
     * cannot be called.
     */
    RdWrLocker()
    {
        throw InvalidMethodException("Someone called the "
                                     "RdWrLocker default constructor!");
    }

  private:
    /**
     * The read write lock
     */
    RdWrLock *mp_lock;
};

class ReadLocker : public RdWrLocker
{
  public:
    ReadLocker(RdWrLock *lock) : RdWrLocker(lock, RdWrLocker::READLOCK) {}

    virtual ~ReadLocker() {}
};

class WriteLocker : public RdWrLocker
{
  public:
    WriteLocker(RdWrLock *lock) : RdWrLocker(lock, RdWrLocker::WRITELOCK) {}

    virtual ~WriteLocker() {}
};

/**
 * Grouping of a predicate, mutex, and a conditional.
 */
class PredMutexCond
{
  public:
    /**
     * Constructor.
     */
    PredMutexCond();

    /**
     * Signal another thread that is waiting on a predicate.  This is
     * a one time signal .  Make sure that the predicate is set to
     * false before calling either the predWait and predSignal
     * functions on either thread.
     */
    void predSignal();

    /**
     * Wait on a predicate to be changed by another thread.  This is a
     * one time wait.  Make sure that the predicate is set to false
     * before calling either the predWait and predSignal functions on
     * either thread.
     *
     * @param usecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @return false if the function timed out, true if predicate changed
     *         (always true if it returns and the timeout == -1)
     */
    bool predWaitUsecs(int64_t usecTimeout);

    /**
     * Wait on a predicate to be changed by another thread.  This is a
     * one time wait.  Make sure that the predicate is set to false
     * before calling either the predWait and predSignal functions on
     * either thread.
     *
     * @param msecTimeout the amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @return false if the function timed out, true if predicate changed
     *         (always true if it returns and the timeout == -1)
     */
    bool predWaitMsecs(int64_t msecTimeout);

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
 * RAII for Notifyable locks.
 */
class NotifyableLocker
{
  public:
    /**
     * Constructor that acquires the lock.
     * 
     * @param notifyable The notifyable that will be locked
     * @param waitMsecs Amount of time to wait for the lock. -1 means forever
     *        0 means do not wait at all.  If -1, this will only finish if the 
     *        lock was acquired.
     */
    NotifyableLocker(Notifyable *notifyable, int64_t waitMsecs = -1);

    /**
     * Try to get the lock.  If it was already acquired, throws an exception.
     *
     * @param waitMsecs Amount of time to wait for the lock. -1 means forever
     *        0 means do not wait at all.  If -1, this will only finish if the 
     *        lock was acquired.
     * @return True is the lock was acquired this time.
     * @throws InvalidMethodException
     */
    bool acquireLock(int64_t waitMsecs = -1);

    /**
     * Does it have the lock?  Only useful when a timeout was used in
     * the constructor.
     *
     * @return True if it has the lock, false otherwise.
     */
    bool hasLock();

    /**
     * Get the Notifyable back.
     *
     * @return Return the internal Notifyable pointer.
     */
    Notifyable *getNotifyable();

    /**
     * Destructor that releases the lock.
     */
    ~NotifyableLocker();

  private:
    /**
     * The notifyable that is being locked.
     */
    Notifyable *m_notifyable;

    /**
     * Was the lock acquired?
     */
    bool m_hasLock;
};


}	/* End of 'namespace clusterlib' */
        
#endif /* _CL_MUTEX_H_ */

