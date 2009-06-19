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

};
