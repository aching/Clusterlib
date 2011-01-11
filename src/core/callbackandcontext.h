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

#ifndef _CL_CALLBACKANDCONTEXT_H_
#define _CL_CALLBACKANDCONTEXT_H_

namespace clusterlib {

/**
 * Generic struct to store a callback and an associated context
 */
struct CallbackAndContext {
    CallbackAndContext(void *callbackParam, void *contextParam) :
        callback(callbackParam),
        context(contextParam) {}

    /**
     * Various callback signatures.  Add your own if you want to use
     * this object with another callback definition.
     */
    void *callback;

    /**
     * Context passed in and managed by the user.
     */
    void *context;    
};

/**
 * Provide allocation/deallocation of callback and its context.
 */
class CallbackAndContextManager
{
  public:
    /**
     * Allocate the memory and setup a CallbackAndContext.  Track the
     * pointer in this object.
     *
     * @param callback the callback to store
     * @param context the context to store
     */
    CallbackAndContext *createCallbackAndContext(void *callback, 
                                                 void *context);
    /**
     * Free the memory associated with the ContextAndCallback.  Stop
     * tracking it in this object.
     *
     * @param callbackAndContext pointer to the callback to free
     */
    void deleteCallbackAndContext(CallbackAndContext *callbackAndContext);

    /**
     * Equivalent to calling deleteCallbackAndContext on all
     * CallbackAndContext pointers that are kept track of by this
     * object.
     */
    void deleteAllCallbackAndContext();

  private:
    /**
     * Get the lock that makes this object thread-safe
     */
    Mutex *getCallbackAndContextLock() { return &m_lock; }

  private:
    /**
     * Contains all the pointers managed by this object
     */
    std::set<CallbackAndContext *> m_callbackAndContextSet;
    
    /** Makes sure that m_userContextAndListenerSet is thread-safe. */
    Mutex m_lock;
};

}      /* End of 'namespace clusterlib' */

#endif /* _CL_CALLBACKANDCONTEXT_H_ */
