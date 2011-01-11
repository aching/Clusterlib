/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#include "clusterlibinternal.h"

DEFINE_LOGGER(LOG, "Thread")

using namespace std;

namespace clusterlib {

/**
 * Set the thread stack size to 1 MB
 */
static int32_t ThreadStackSize = 1024 * 1024;

void Thread::Create(void* ctx, ThreadFunc func)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, ThreadStackSize);
    if (pthread_create(&mThread, &attr, func, ctx) != 0) {
        ostringstream oss;
        oss << "Create: pthread_create failed with error " << strerror(errno);
        throw SystemFailureException(oss.str());
    }
    if (pthread_attr_destroy(&attr) != 0) {
        ostringstream oss;
        oss << "Create: pthread_attr_destroy failed with error " 
            << strerror(errno);
        throw SystemFailureException(oss.str());
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
            ostringstream oss;
            oss << "Join: pthread_join failed with error " << strerror(errno);
            LOG_FATAL(LOG, "pthread_join failed: %s", strerror(errno));
            throw SystemFailureException(oss.str());
        }
    }
}

}	/* End of 'namespace clusterlib' */

