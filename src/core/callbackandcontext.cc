#include "clusterlibinternal.h"

using namespace std;

namespace clusterlib {

CallbackAndContext *
CallbackAndContextManager::createCallbackAndContext(void *callback,
                                                    void *context)
{
    TRACE(CL_LOG, "createCallbackAndContext");
    
    CallbackAndContext *callbackAndContext = new CallbackAndContext(callback,
                                                                    context);

    Locker l1(getCallbackAndContextLock());
    set<CallbackAndContext *>::const_iterator it = 
        m_callbackAndContextSet.find(callbackAndContext);
    if (it != m_callbackAndContextSet.end()) {
        LOG_ERROR(CL_LOG,
                  "createCallbackAndContext: Impossible that context %p"
                  " already exists in the set!",
                  context);
        throw InconsistentInternalStateException(
            "createCallbackAndContext: Unable to add CallbackAndContext to"
            " set since it already exists!");
    }

    m_callbackAndContextSet.insert(callbackAndContext);
    return callbackAndContext;
}

void
CallbackAndContextManager::deleteCallbackAndContext(
    CallbackAndContext *callbackAndContext)
{
    TRACE(CL_LOG, "deleteCallbackAndContext");

    Locker l1(getCallbackAndContextLock());
    set<CallbackAndContext *>::const_iterator it = 
        m_callbackAndContextSet.find(callbackAndContext);
    if (it == m_callbackAndContextSet.end()) {
        LOG_ERROR(CL_LOG,
                  "deleteCallbackAndContext: Impossible that context %p"
                  " doesn't exist in the set!",
                  callbackAndContext);
        throw InconsistentInternalStateException(
            "deleteCallbackAndContext: Unable to "
            "delete CallbackAndContext from set since it doesn't exist!");
    }

    m_callbackAndContextSet.erase(it);
    delete callbackAndContext;
}

void
CallbackAndContextManager::deleteAllCallbackAndContext()
{
    /* 
     * Clean up all memory that was allocated on the heap.
     */
    Locker l1(getCallbackAndContextLock());
    set<CallbackAndContext *>::const_iterator it;
    for (it = m_callbackAndContextSet.begin();
         it != m_callbackAndContextSet.end(); 
         it++) {
        /* 
         * Doesn't use deleteCallbackAndContext since this can
         * lead to invalid iterators.
         */
        delete *it;
    }
    m_callbackAndContextSet.clear();
}

};
