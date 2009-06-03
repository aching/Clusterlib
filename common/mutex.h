/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "clusterlibexceptions.h"

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
    Mutex()
    {
        pthread_mutexattr_init(&m_mutexAttr);
        pthread_mutexattr_settype(&m_mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&mutex, &m_mutexAttr);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
        pthread_mutexattr_destroy(&m_mutexAttr);
    }
    void acquire() { lock(); }
    void release() { unlock(); }
    void lock()
    {
        pthread_mutex_lock(&mutex);
    }
    int32_t tryLock()
    {
        return pthread_mutex_trylock(&mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
  private:
    pthread_mutex_t mutex;
    pthread_mutexattr_t m_mutexAttr;
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

    void wait(Mutex& mutex)
    {
        pthread_cond_wait(&m_cond, &mutex.mutex);
    }

    bool wait(Mutex& mutex, uint64_t timeout)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        struct timespec abstime;
        int64_t microSecs = now.tv_sec * 1000000LL + now.tv_usec;
        microSecs += timeout * 1000;
        abstime.tv_sec = microSecs / 1000000LL;
        abstime.tv_nsec = (microSecs % 1000000LL) * 1000;
        if (pthread_cond_timedwait(&m_cond, &mutex.mutex, &abstime) == 
            ETIMEDOUT)
        {
            return false;
        } else {
            return true;
        }
    }
    
    void signal()
    {
        pthread_cond_signal(&m_cond);
    }

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
        m_mutex.lock();
    }
        
    void unlock()
    {
        m_mutex.unlock();
    }
        
    void wait()
    {
        m_cond.wait(m_mutex);
    }

    bool wait(int64_t timeout)
    {
        return m_cond.wait(m_mutex, timeout);
    }
        
    void notify()
    {
        m_cond.signal();
    }

    void lockedWait()
    {
        m_mutex.lock();
        m_cond.wait(m_mutex);
        m_mutex.unlock();
    }

    bool lockedWait(int64_t timeout)
    {
        bool res;

        m_mutex.lock();
        res = m_cond.wait(m_mutex, timeout);
        m_mutex.unlock();

        return res;
    }

    void lockedNotify()
    {
        m_mutex.lock();
        m_cond.signal();
        m_mutex.unlock();
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
    /*
     * Constructor locks the passed mutex.
     */
    Locker(Mutex *mp)
    {
        mp->lock();
        mp_lock = mp;
    }

    /*
     * Destructor unlocks the mutex.
     */
    ~Locker()
    {
        mp_lock->unlock();
    }
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
    RdWrLock()
    {
        int ret = pthread_rwlock_init(&m_rwlock, NULL);
        if (ret != 0) {
            throw SystemFailureException(
                "RdWrLock: constructor failed");
        }
    }
    ~RdWrLock()
    {
        int ret = pthread_rwlock_destroy(&m_rwlock);
        if (ret != 0) {
            throw SystemFailureException(
                "RdWrLock: destructor failed");
        }
    }
    /**
     * Get the read lock
     */
    void acquireRead()
    {
        int ret = pthread_rwlock_rdlock(&m_rwlock);
        if (ret != 0) {
            throw SystemFailureException("acquireRead: failed");
        }
    }
    /**
     * Get the write lock
     */
    void acquireWrite()
    {
        int ret = pthread_rwlock_wrlock(&m_rwlock);
        if (ret != 0) {
            throw SystemFailureException("acquireWrite: failed");
        }
    }
    /**
     * Release lock
     */
    void release()
    {
        int ret = pthread_rwlock_unlock(&m_rwlock);
        if (ret != 0) {
            throw SystemFailureException("release: failed");
        }
    }
  private:
    pthread_rwlock_t m_rwlock;    
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
    /*
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

    /*
     * Destructor unlocks the mutex.
     */
    ~RdWrLocker()
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

};	/* End of 'namespace clusterlib' */
        
#endif /* __MUTEX_H__ */

