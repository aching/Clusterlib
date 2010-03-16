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
    return waitUsecs(mutex, msecTimeout * 1000);
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


Locker::Locker(Mutex *mp)
{
    TRACE(CL_LOG, "Locker");

    mp->lock();
    mp_lock = mp;
}

Locker::~Locker()
{
    mp_lock->unlock();
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
                  "predWaitUsecs: About to wait for a maximum of %lld usecs", 
                  usecTimeout);
        if (usecTimeout == -1) {
            cond.wait(mutex);
        }
        else {
            /* Don't let curUsecTimeout go negative. */
            curUsecTimeout = max(
                maxUsecs - TimerService::getCurrentTimeUsecs(), 0LL);
            LOG_TRACE(CL_LOG, 
                      "predWaitUsecs: Going to wait for %lld usecs (%lld "
                      "usecs originally)", 
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

    return predWaitUsecs(msecTimeout * 1000);
}

}
