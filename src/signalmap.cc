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
    
void SignalMap::addRefPredMutexCond(const string &key,
                                    PredMutexCond *predMutexCond)
{
    Locker l1(getSignalMapLock());

    map<string, PredMutexCond *>::iterator it = m_signalMap.find(key);
    if (it == m_signalMap.end()) {
        m_signalMap.insert(make_pair(key, predMutexCond));
        it = m_signalMap.find(key);
        if (it == m_signalMap.end()) {
            throw InconsistentInternalStateException(
                "addRefPredMutexCond: Key shuld exist");
        }
    }
    it->second->refCount++;
}

void SignalMap::removeRefPredMutexCond(const string &key)
{
    Locker l1(getSignalMapLock());
    map<string, PredMutexCond *>::iterator it = m_signalMap.find(key);
    if (it == m_signalMap.end()) {
        throw InconsistentInternalStateException(
            "removeRefPredMutexCond: Can not find key to remove ref");
    }
    it->second->refCount--;
    if (it->second->refCount == 0) {
        m_signalMap.erase(it);
    } 
}

void SignalMap::signalPredMutexCond(const string &key)
{
    map<string, PredMutexCond *>::iterator it;
    {
        Locker l1(getSignalMapLock());
        it = m_signalMap.find(key);
        if (it == m_signalMap.end()) {
            throw InconsistentInternalStateException(
                string("waitPredMutexCond: Cannot signal on missing key ") +
                key);
        }
    }
    it->second->predSignal();
}

void SignalMap::waitPredMutexCond(const string &key)
{
    map<string, PredMutexCond *>::iterator it;
    {
        Locker l1(getSignalMapLock());
        it = m_signalMap.find(key);
        if (it == m_signalMap.end()) {
            throw InconsistentInternalStateException(
                string("waitPredMutexCond: Cannot wait on missing key") +
                key);
        }
    }
    it->second->predWait();
}

};
