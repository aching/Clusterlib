/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
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

class CallbackAndContextManager {
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

};   /* end of 'namespace clusterlib' */

#endif /* _CL_CALLBACKANDCONTEXT_H_ */
