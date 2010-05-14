/*
 * registerednotifyableimpl.cc
 *
 * Partial implementation of the RegisteredNotifyableImpl class.
 *
 * ===========================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

#include "clusterlibinternal.h"
#include <limits>

using namespace std;
using namespace boost;

namespace clusterlib
{

void
RegisteredNotifyableImpl::setSafeNotifyableMap(
    SafeNotifyableMap &safeNotifyableMap) 
{
    Locker l(&getLock());

    m_safeNotifyableMap = &safeNotifyableMap;
}

SafeNotifyableMap *
RegisteredNotifyableImpl::getSafeNotifyableMap()
{
    Locker l(&getLock());

    return m_safeNotifyableMap;
}

NotifyableImpl *
RegisteredNotifyableImpl::loadNotifyableFromRepository(
    const string &notifyableName,
    const string &notifyableKey,
    NotifyableImpl *parent)
{
    TRACE(CL_LOG, "loadNotifyableFromRepository");

    vector<string> zknodeVec = 
        generateRepositoryList(notifyableName, notifyableKey);
    /* Add some zknodes that are part of every notifyable */
    zknodeVec.push_back(
        NotifyableImpl::createStateJSONArrayKey(
           notifyableKey, CachedStateImpl::CURRENT_STATE));
    zknodeVec.push_back(
        NotifyableImpl::createStateJSONArrayKey(
           notifyableKey, CachedStateImpl::DESIRED_STATE));

    bool exists = false;
    vector<string>::const_iterator zknodeVecIt;
    for (zknodeVecIt = zknodeVec.begin(); 
         zknodeVecIt != zknodeVec.end();
         ++zknodeVecIt) {
        SAFE_CALLBACK_ZK(
            (exists = getOps()->getRepository()->nodeExists(
                *zknodeVecIt,
                getOps()->getZooKeeperEventAdapter(),
                getOps()->getCachedObjectChangeHandlers()->
                getChangeHandler(
                    CachedObjectChangeHandlers::NOTIFYABLE_REMOVED_CHANGE))),
            (exists = getOps()->getRepository()->nodeExists(*zknodeVecIt)),
            CachedObjectChangeHandlers::NOTIFYABLE_REMOVED_CHANGE,
            *zknodeVecIt,
            "Checking existence and establishing watch on value %s failed: %s",
            zknodeVecIt->c_str(),
            false,
            true);
        if (!exists) {
            return NULL;
        }
    }
    return createNotifyable(notifyableName, notifyableKey, parent, *getOps());
}

void
RegisteredNotifyableImpl::createRepositoryObjects(const string &notifyableName,
                                                  const string &notifyableKey)
{
    TRACE(CL_LOG, "createRepositoryObjects");

    vector<string> zknodeVec = 
        generateRepositoryList(notifyableName, notifyableKey);
    /* Add some zknodes that are part of every notifyable */
    zknodeVec.push_back(
        NotifyableImpl::createStateJSONArrayKey(
           notifyableKey, CachedStateImpl::CURRENT_STATE));
    zknodeVec.push_back(
        NotifyableImpl::createStateJSONArrayKey(
           notifyableKey, CachedStateImpl::DESIRED_STATE));

    vector<string>::const_iterator zknodeVecIt;
    for (zknodeVecIt = zknodeVec.begin(); 
         zknodeVecIt != zknodeVec.end();
         ++zknodeVecIt) {
        SAFE_CALL_ZK(getOps()->getRepository()->createNode(
                         *zknodeVecIt, "", 0),
                     "Could not create key %s: %s",
                     zknodeVecIt->c_str(),
                     true,
                     true);
    }
}

bool
RegisteredNotifyableImpl::isValidName(const string &name) const
{
    TRACE(CL_LOG, "isValidName");

    if (!NotifyableKeyManipulator::isValidNotifyableName(name)) {
        LOG_WARN(CL_LOG,
                 "isValidName: Illegal Notifyable name %s",
                 name.c_str());
        return false;
    }
    else {
        return true;
    }
}

bool
RegisteredNotifyableImpl::isValidKey(const string &key)
{
    TRACE(CL_LOG, "isValidKey");
    
    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isValidKey(components, -1);
}

NotifyableImpl *
RegisteredNotifyableImpl::getObjectFromKey(const string &key,
                                           AccessType accessType)
{
    TRACE(CL_LOG, "getObjectFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return getObjectFromComponents(components, -1, accessType);
}

Mutex &
RegisteredNotifyableImpl::getLock()
{
    return m_mutex;
}

};	/* End of 'namespace clusterlib' */
