/*
 * signalmap.cc --
 *
 * Implementation of the SignalMap class.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;

namespace clusterlib
{
    
void
SignalMap::addRefPredMutexCond(const string &key)
{
    TRACE(CL_LOG, "addRefPredMutexCond");

    Locker l1(getSignalMapLock());
    LOG_DEBUG(CL_LOG, "addRefPredMutexCond: finding (%s)", key.c_str());
    map<string, PredMutexCond *>::iterator it = m_signalMap.find(key);
    if (it == m_signalMap.end()) {
        m_signalMap.insert(make_pair(key, new PredMutexCond()));
        it = m_signalMap.find(key);
        if (it == m_signalMap.end()) {
            throw InconsistentInternalStateException(
                "addRefPredMutexCond: Key should exist");
        }
    }
    else {
        LOG_WARN(CL_LOG, 
                 "addRefPredMutexCond: Key (%s) already exists and has "
                 "refcount (%d)",
                 key.c_str(),
                 it->second->refCount);
    }
    it->second->refCount++;
}

void SignalMap::removeRefPredMutexCond(const string &key)
{
    TRACE(CL_LOG, "removeRefPredMutexCond");

    Locker l1(getSignalMapLock());
    LOG_DEBUG(CL_LOG, 
              "removeRefPredMutexCond: removing ref on key (%s)", 
              key.c_str());
    map<string, PredMutexCond *>::iterator it = m_signalMap.find(key);
    if (it == m_signalMap.end()) {
        throw InconsistentInternalStateException(
            "removeRefPredMutexCond: Can not find key to remove ref");
    }
    if (it->second->refCount == 0) {
        throw InconsistentInternalStateException(
            string("removeRefPredMutexCond: Ref count is 0 for key ") + key);
    }
    it->second->refCount--;
    if (it->second->refCount == 0) {
        if (it->second == NULL) {
            throw InconsistentInternalStateException(
                string("removeRefPredMutexCond: No PredMutexCond for key ") +
                key);
        }
        delete it->second;
        m_signalMap.erase(it);
    }
}

bool SignalMap::signalPredMutexCond(const string &key)
{
    TRACE(CL_LOG, "signalPredMutexCond");

    Locker l1(getSignalMapLock());
    LOG_DEBUG(CL_LOG, 
              "signalPredMutexCond: signaling key (%s)",
              key.c_str());
    map<string, PredMutexCond *>::iterator it = m_signalMap.find(key);
    if (it == m_signalMap.end()) {
        LOG_WARN(CL_LOG,
                 "signalPredMutexCond: Cannot signal on missing key %s",
                 key.c_str());
        return false;
    }
    if (it->second == NULL) {
        throw InconsistentInternalStateException(
            string("signalPredMutexCond: Key ") + key + 
            " has no PredMutexCond");
    }
    it->second->predSignal();
    return true;
}

bool SignalMap::waitUsecsPredMutexCond(const string &key, int64_t usecTimeout)
{
    TRACE(CL_LOG, "waitUsecsPredMutexCond");

    map<string, PredMutexCond *>::iterator it;
    {
        Locker l1(getSignalMapLock());
        LOG_DEBUG(CL_LOG, 
                  "waitUsecsPredMutexCond: waiting (%s), timeout (%lld)",
                  key.c_str(),
                  usecTimeout);
        it = m_signalMap.find(key);
        if (it == m_signalMap.end()) {
            throw InconsistentInternalStateException(
                string("waitUsecsPredMutexCond: Cannot wait on missing key") +
                key);
        }
        if (it->second->refCount == 0) {
            throw InconsistentInternalStateException(
                string("waitUsecsPredMutexCond: Ref count is 0 for key ") + 
                key);
        }
    }
    return it->second->predWaitUsecs(usecTimeout);
}

};
