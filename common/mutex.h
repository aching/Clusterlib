/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

namespace clusterlib {

/*
 * Forward decl of Cond.
 */
class Cond;

class Mutex
{
    friend class Cond;
  public:
    Mutex()
    {
        pthread_mutexattr_init( &m_mutexAttr );
        pthread_mutexattr_settype( &m_mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP );
        pthread_mutex_init( &mutex, &m_mutexAttr );
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
        pthread_mutexattr_destroy( &m_mutexAttr );
    }
    void Acquire() { Lock(); }
    void Release() { Unlock(); }
    void Lock()
    {
        pthread_mutex_lock(&mutex);
    }
    int32_t  TryLock()
    {
        return pthread_mutex_trylock(&mutex);
    }
    void Unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
  private:
    pthread_mutex_t mutex;
    pthread_mutexattr_t m_mutexAttr;
};

class AutoLock {
  public:
    AutoLock(Mutex& mutex)
        : _mutex(mutex)
    {
        mutex.Lock();
    }
    ~AutoLock()
    {
        _mutex.Unlock();
    }
  private:
    Mutex& _mutex;
};

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
        pthread_cond_init(&_cond, &attr);
    }
    ~Cond()
    {
        pthread_cond_destroy(&_cond);
    }

    void Wait(Mutex& mutex)
    {
        pthread_cond_wait(&_cond, &mutex.mutex);
    }

    bool Wait(Mutex& mutex, uint64_t timeout)
    {
        struct timeval now;
        gettimeofday( &now, NULL );
        struct timespec abstime;
        int64_t microSecs = now.tv_sec * 1000000LL + now.tv_usec;
        microSecs += timeout * 1000;
        abstime.tv_sec = microSecs / 1000000LL;
        abstime.tv_nsec = (microSecs % 1000000LL) * 1000;
        if (pthread_cond_timedwait(&_cond, &mutex.mutex, &abstime) == 
            ETIMEDOUT)
        {
            return false;
        } else {
            return true;
        }
    }
    
    void Signal()
    {
        pthread_cond_signal(&_cond);
    }

  private:
    pthread_cond_t            _cond;
};

/**
 * A wrapper class for {@link Mutex} and {@link Cond}.
 */
class Lock
{
  public:
    void lock()
    {
        m_mutex.Lock();
    }
        
    void unlock()
    {
        m_mutex.Unlock();
    }
        
    void wait()
    {
        m_cond.Wait( m_mutex );
    }

    bool wait(int64_t timeout)
    {
        return m_cond.Wait( m_mutex, timeout );
    }
        
    void notify()
    {
        m_cond.Signal();
    }

    void lockedWait()
    {
        m_mutex.Lock();
        m_cond.Wait(m_mutex);
        m_mutex.Unlock();
    }

    bool lockedWait(int64_t timeout)
    {
        bool res;

        m_mutex.Lock();
        res = m_cond.Wait(m_mutex, timeout);
        m_mutex.Unlock();

        return res;
    }

    void lockedNotify()
    {
        m_mutex.Lock();
        m_cond.Signal();
        m_mutex.Unlock();
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

/*
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
        mp->Lock();
        mp_lock = mp;
    }

    /*
     * Destructor unlocks the mutex.
     */
    ~Locker()
    {
        mp_lock->Unlock();
    }
  private:
    /*
     * Make the default constructor private so it
     * cannot be called.
     */
    Locker()
    {
        throw ClusterException("Someone called the "
                               "Locker default constructor!");
    }

  private:
    /*
     * Hold onto the mutex being locked.
     */
    Mutex *mp_lock;
};

};	/* End of 'namespace clusterlib' */
        
#endif /* __MUTEX_H__ */

