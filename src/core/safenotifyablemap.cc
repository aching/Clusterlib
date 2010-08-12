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
using namespace boost;

namespace clusterlib {
    
shared_ptr<NotifyableImpl>
SafeNotifyableMap::getNotifyable(const string &notifyableKey)
{
    map<string, shared_ptr<NotifyableImpl> >::const_iterator ntpMapIt = 
        m_ntpMap.find(notifyableKey);
    LOG_DEBUG(CL_LOG,
              "getNotifyable: Looking for key=%s",
              notifyableKey.c_str());
    if (ntpMapIt == m_ntpMap.end()) {
        return shared_ptr<NotifyableImpl>();
    }
    else {
        return ntpMapIt->second;
    }
}

void
SafeNotifyableMap::uniqueInsert(const shared_ptr<NotifyableImpl> &notifyableSP)
{
    map<string, shared_ptr<NotifyableImpl> >::const_iterator ntpMapIt = 
        m_ntpMap.find(notifyableSP->getKey());
    if (ntpMapIt != m_ntpMap.end()) {
        ostringstream oss;
        oss << "uniqueInsert: Cache entry already exists for key=" 
            << notifyableSP->getKey();
        throw InconsistentInternalStateException(oss.str());
    }
    else {
        m_ntpMap.insert(make_pair<string, shared_ptr<NotifyableImpl> >(
                            notifyableSP->getKey(), 
                            notifyableSP));
        LOG_DEBUG(CL_LOG,
                  "uniqueInsert: Adding name=%s, key=%s",
                  notifyableSP->getName().c_str(),
                  notifyableSP->getKey().c_str());
    }
}

void
SafeNotifyableMap::erase(const shared_ptr<NotifyableImpl> &notifyableSP)
{
    map<string, shared_ptr<NotifyableImpl> >::iterator ntpMapIt = 
        m_ntpMap.find(notifyableSP->getKey());
    if (ntpMapIt == m_ntpMap.end()) {
        ostringstream oss;
        oss << "uniqueInsert: Cache entry for key=" 
            << notifyableSP->getKey() << " doesn't exist!";
        throw InconsistentInternalStateException(oss.str());
    }
    else {
        m_ntpMap.erase(ntpMapIt);
    }
}

const Mutex &
SafeNotifyableMap::getLock() const
{
    return m_ntpMapLock;
}

SafeNotifyableMap::~SafeNotifyableMap()
{
}

}	/* End of 'namespace clusterlib' */
