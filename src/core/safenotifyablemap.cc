/*
 * safenotifyablemap.cc --
 *
 * Implementation of the SafeNotifyableMap class.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"
#include "safenotifyablemap.h"

using namespace std;

namespace clusterlib
{
    
NotifyableImpl *
SafeNotifyableMap::getNotifyable(const string &notifyableKey)
{
    map<string, NotifyableImpl *>::const_iterator ntpMapIt = 
        m_ntpMap.find(notifyableKey);
    if (ntpMapIt == m_ntpMap.end()) {
        return NULL;
    }
    else {
        return ntpMapIt->second;
    }
}

void
SafeNotifyableMap::uniqueInsert(NotifyableImpl &notifyable)
{
    map<std::string, NotifyableImpl *>::const_iterator ntpMapIt = 
        m_ntpMap.find(notifyable.getKey());
    if (ntpMapIt != m_ntpMap.end()) {
        ostringstream oss;
        oss << "uniqueInsert: Cache entry already exists for key=" 
            << notifyable.getKey();
        throw InconsistentInternalStateException(oss.str());
    }
    else {
        m_ntpMap[notifyable.getKey()] = &notifyable;
    }
}

void
SafeNotifyableMap::erase(NotifyableImpl &notifyable)
{
    map<std::string, NotifyableImpl *>::iterator ntpMapIt = 
        m_ntpMap.find(notifyable.getKey());
    if (ntpMapIt == m_ntpMap.end()) {
        ostringstream oss;
        oss << "uniqueInsert: Cache entry for key=" 
            << notifyable.getKey() << " doesn't exist!";
        throw InconsistentInternalStateException(oss.str());
    }
    else {
        m_ntpMap.erase(ntpMapIt);
    }
}

Mutex &
SafeNotifyableMap::getLock()
{
    return m_ntpMapLock;
}

SafeNotifyableMap::~SafeNotifyableMap()
{
    Locker l(&getLock());

    map<string, NotifyableImpl *>::iterator ntpMapIt;
    while (!m_ntpMap.empty()) {
        ntpMapIt = m_ntpMap.begin();
        delete ntpMapIt->second;
        m_ntpMap.erase(ntpMapIt);
    }
}

};	/* End of 'namespace clusterlib' */
