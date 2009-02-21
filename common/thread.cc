/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#include "clusterlib.h"

DEFINE_LOGGER(LOG, "Thread")

namespace clusterlib {

void Thread::Create(void* ctx, ThreadFunc func)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024*1024);
    int ret = pthread_create(&mThread, &attr, func, ctx);
    if(ret != 0) {
        LOG_FATAL(LOG, "pthread_create failed: %s", strerror(errno));
    }
    // pthread_attr_destroy(&attr); 
    _ctx = ctx;
    _func = func;
}

}	/* End of 'namespace clusterlib' */

