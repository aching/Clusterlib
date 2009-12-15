/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

namespace clusterlib
{

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
PredMutexCond::predWait(const uint64_t timeout)
{
    TRACE(CL_LOG, "predWait");

    int64_t microSecs = 0;
    int64_t maxMicroSecs = 0;
    mutex.acquire();
    if (timeout != 0) {
        maxMicroSecs = 
            TimerService::getCurrentTimeUsecs() + timeout * 1000;
    }
    while (pred == false) {
        LOG_DEBUG(CL_LOG,
                  "predWait: About to wait for %lld msecs", 
                  timeout);
        if (timeout == 0) {
            cond.wait(mutex);
        }
        else {
            if (TimerService::compareTimeUsecs(maxMicroSecs) >= 0) {
                mutex.release();
                return false;
            }
            microSecs = maxMicroSecs - TimerService::getCurrentTimeUsecs();
            LOG_DEBUG(CL_LOG, 
                      "predWait: Going to wait for %lld usecs (%lld msecs)", 
                      microSecs,
                      microSecs / 1000);
            cond.wait(mutex, microSecs / 1000);
        }
    }
    mutex.release();
    return true;
}

};
