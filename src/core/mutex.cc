/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"
#include <sstream>

using namespace std;

namespace clusterlib
{

void
Cond::wait(Mutex &mutex)
{
    TRACE(CL_LOG, "wait");
    
    int ret = pthread_cond_wait(&m_cond, &mutex.mutex);
    if (ret) {
        stringstream ss;
        ss << "wait: pthread_cond_wait failed with " << ret;
        throw SystemFailureException(ss.str());
    }
}

bool
Cond::waitUsecs(Mutex &mutex, int64_t usecTimeout)
{
    TRACE(CL_LOG, "waitUsecs");

    if (usecTimeout < -1) {
        stringstream ss;
        ss << "wait: Cannot have usecTimeout < -1 (" << usecTimeout << ")";
        throw InvalidArgumentsException(ss.str());
    }
    else if (usecTimeout == -1) {
        wait(mutex);
        return true;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    struct timespec abstime;
    int64_t usecs = now.tv_sec * 1000000LL + now.tv_usec;
    usecs += usecTimeout;
    abstime.tv_sec = usecs / 1000000LL;
    abstime.tv_nsec = (usecs % 1000000LL) * 1000;
    if (pthread_cond_timedwait(&m_cond, &mutex.mutex, &abstime) == 
        ETIMEDOUT)
    {
        return false;
    } 
    else {
        return true;
    }
}

bool
Cond::waitMsecs(Mutex &mutex, int64_t msecTimeout)
{
    TRACE(CL_LOG, "waitMsecs");

    if (msecTimeout == -1) {
        return waitUsecs(mutex, -1);
    }
    else {
        return waitUsecs(mutex, msecTimeout * 1000);
    }
}

void
Mutex::acquire()
{
    TRACE(CL_LOG, "acquire");

    lock();
}

void
Mutex::release()
{
    TRACE(CL_LOG, "release");

    unlock();
}

void
Mutex::lock()
{
    TRACE(CL_LOG, "lock");

    int ret = pthread_mutex_lock(&mutex);
    if (ret != 0) {
        throw InconsistentInternalStateException(
            "lock: Failed to lock");
    }
}

void
Mutex::unlock()
{
    TRACE(CL_LOG, "unlock");

    int ret = pthread_mutex_unlock(&mutex); 
    if (ret != 0) {
        throw InconsistentInternalStateException(
            "unlock: Failed to unlock");
    }
}

Locker::Locker(Mutex *mp)
{
    TRACE(CL_LOG, "Locker");

    mp->acquire();
    mp_lock = mp;
}

Locker::~Locker()
{
    mp_lock->release();
}

RdWrLock::RdWrLock()
{
    int ret = pthread_rwlockattr_init(&m_rwlockAttr);
    if (ret != 0) {
        ostringstream oss;
        oss << "RdWrLock: pthread_rwlockattr_init failed with ret=" << ret
            << ", errno=" << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
    ret = pthread_rwlockattr_setpshared(&m_rwlockAttr, PTHREAD_PROCESS_SHARED);
    if (ret != 0) {
        ostringstream oss;
        oss << "RdWrLock: pthread_rwlockattr_setpshared failed with ret="
            << ret << ", errno=" << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
    
    ret = pthread_rwlock_init(&m_rwlock, &m_rwlockAttr);
    if (ret != 0) {
        ostringstream oss;
        oss << "RdWrLock: pthread_rwlock_init failed with ret=" << ret
            << ", errno=" << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
}

RdWrLock::~RdWrLock()
{
    int ret = pthread_rwlock_destroy(&m_rwlock);
    if (ret != 0) {
        ostringstream oss;
        oss << "RdWrLock: pthread_rwlock_destroy failed with ret=" << ret
            << ", errno=" << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }

    ret = pthread_rwlockattr_destroy(&m_rwlockAttr);
    if (ret != 0) {
        ostringstream oss;
        oss << "RdWrLock: pthread_rwlockattr_destroy failed with ret=" << ret
            << ", errno=" << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
}

void
RdWrLock::acquireRead()
{        
    int ret = pthread_rwlock_rdlock(&m_rwlock);
    if (ret != 0) {
        ostringstream oss;
        oss << "acquireRead: Failed with ret=" << ret << ", errno=" 
            << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
}

void 
RdWrLock::acquireWrite()
{
    int ret = pthread_rwlock_wrlock(&m_rwlock);
    if (ret != 0) {
        ostringstream oss;
        oss << "acquireWrite: Failed with ret=" << ret << ", errno=" 
            << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
}

void 
RdWrLock::release()
{
    int ret = pthread_rwlock_unlock(&m_rwlock);
    if (ret != 0) {
        ostringstream oss;
        oss << "release: Failed with ret=" << ret << ", errno=" 
            << errno << ", strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
}

PredMutexCond::PredMutexCond()         
    : pred(false),
      refCount(0) 
{
    TRACE(CL_LOG, "PredMutexCond");
}

void
PredMutexCond::predSignal()
{
    TRACE(CL_LOG, "predSignal");

    Locker l1(&mutex);
    pred = true;
    cond.signal();
}

bool
PredMutexCond::predWaitUsecs(int64_t usecTimeout)
{
    TRACE(CL_LOG, "predWaitUsecs");

    if (usecTimeout < -1) {
        stringstream ss;
        ss << "predWaitUsecs: Cannot have usecTimeout < -1 (" 
           << usecTimeout << ")";
        throw InvalidArgumentsException(ss.str());
    }

    int64_t curUsecTimeout = 0;
    int64_t maxUsecs = 0;
    if (usecTimeout != -1) {
        maxUsecs = TimerService::getCurrentTimeUsecs() + usecTimeout;
    }
    mutex.acquire();
    while (pred == false) {
        LOG_TRACE(CL_LOG,
                  "predWaitUsecs: About to wait for a maximum of %" PRId64
                  " usecs", 
                  usecTimeout);
        if (usecTimeout == -1) {
            cond.wait(mutex);
        }
        else {
            /* Don't let curUsecTimeout go negative. */
            curUsecTimeout = max(
                maxUsecs - TimerService::getCurrentTimeUsecs(), 
                static_cast<int64_t>(0));
            LOG_TRACE(CL_LOG, 
                      "predWaitUsecs: Going to wait for %" PRId64 
                      " usecs (%" PRId64 " secs originally)", 
                      curUsecTimeout,
                      usecTimeout);
            cond.waitUsecs(mutex, curUsecTimeout);
            if (TimerService::compareTimeUsecs(maxUsecs) >= 0) {
                mutex.release();
                return false;
            }
        }
    }
    mutex.release();
    return true;
}

bool
PredMutexCond::predWaitMsecs(int64_t msecTimeout)
{
    TRACE(CL_LOG, "predWaitMsecs");
    
    if (msecTimeout == -1) {
        return predWaitUsecs(-1);
    }
    else {
        return predWaitUsecs(msecTimeout * 1000);
    }
}

NotifyableLocker::NotifyableLocker(Notifyable *notifyable, int64_t waitMsecs)
    : m_notifyable(notifyable),
      m_hasLock(false)
{
    if (m_notifyable == NULL) {
        throw InvalidArgumentsException("NotifyableLocker: NULL notifyable");
    }

    acquireLock(waitMsecs);
}

bool
NotifyableLocker::acquireLock(int64_t waitMsecs)
{
    if (m_hasLock) {
        throw InvalidMethodException("acquireLock: Already has the lock");
    }

    m_hasLock = m_notifyable->acquireLockWaitMsecs(waitMsecs);
    return m_hasLock;
}

bool
NotifyableLocker::hasLock()
{
    return m_hasLock;
}

NotifyableLocker::~NotifyableLocker()
{
    if (m_hasLock) {
        m_notifyable->releaseLock();
    }
}

}
