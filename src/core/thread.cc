/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#include <sstream>
#include "clusterlibinternal.h"

DEFINE_LOGGER(LOG, "Thread")

using namespace std;

namespace clusterlib {

void Thread::Create(void* ctx, ThreadFunc func)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 1024*1024);
    if (pthread_create(&mThread, &attr, func, ctx) != 0) {
        stringstream ss;
        ss << "Create: pthread_create failed with error " << strerror(errno);
        throw SystemFailureException(ss.str());
    }
    if (pthread_attr_destroy(&attr) != 0) {
        stringstream ss;
        ss << "Create: pthread_attr_destroy failed with error " 
           << strerror(errno);
        throw SystemFailureException(ss.str());
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
            stringstream ss;
            ss << "Join: pthread_join failed with error " << strerror(errno);
            LOG_FATAL(LOG, "pthread_join failed: %s", strerror(errno))
;            throw SystemFailureException(ss.str());
        }
    }
}

}	/* End of 'namespace clusterlib' */

