/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#include "clusterlib.h"

DEFINE_LOGGER( LOG, "Thread" )

namespace clusterlib {

void Thread::Create(void* ctx, ThreadFunc func)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024*1024);
    if (pthread_create(&mThread, &attr, func, ctx) != 0) {
        LOG_FATAL(LOG, "pthread_create failed: %s", strerror(errno));
    }
    if (pthread_attr_destroy(&attr) != 0) {
        LOG_FATAL(LOG, "pthread_attr_destroy failed: %s", strerror(errno));
        ::abort();
    }
    _ctx = ctx;
    _func = func;
}

void Thread::Join()
{
    //avoid SEGFAULT because of unitialized mThread
    //in case Create(...) was never called
    if (_func != NULL) {
        if (pthread_join(mThread, 0) != 0) {
            LOG_FATAL(LOG, "pthread_join failed: %s", strerror(errno));
            ::abort();
        }
    }
}

}	/* End of 'namespace clusterlib' */

