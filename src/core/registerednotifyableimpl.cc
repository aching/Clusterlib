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
#include <limits>

using namespace std;
using namespace boost;

namespace clusterlib {

void
RegisteredNotifyableImpl::setSafeNotifyableMap(
    SafeNotifyableMap &safeNotifyableMap) 
{
    WriteLocker l(&getRdWrLock());

    m_safeNotifyableMap = &safeNotifyableMap;
}

SafeNotifyableMap *
RegisteredNotifyableImpl::getSafeNotifyableMap()
{
    ReadLocker l(&getRdWrLock());

    return m_safeNotifyableMap;
}

shared_ptr<NotifyableImpl>
RegisteredNotifyableImpl::loadNotifyableFromRepository(
    const string &notifyableName,
    const string &notifyableKey,
    const shared_ptr<NotifyableImpl> &parent)
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
            return shared_ptr<NotifyableImpl>();
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
    split(components, key, is_any_of(CLString::KEY_SEPARATOR));
    return isValidKey(components, -1);
}

bool
RegisteredNotifyableImpl::getObjectFromKey(
    const string &key,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<NotifyableImpl> *pNotifyableSP)
{
    TRACE(CL_LOG, "getObjectFromKey");

    vector<string> components;
    split(components, key, is_any_of(CLString::KEY_SEPARATOR));
    return getObjectFromComponents(
        components, -1, accessType, msecTimeout, pNotifyableSP);
}

bool
RegisteredNotifyableImpl::getObjectFromComponents(
    const vector<string> &components,
    int32_t elements, 
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<NotifyableImpl> *pNotifyableSP)
{
    TRACE(CL_LOG, "getObjectFromComponents");

    if (pNotifyableSP == NULL) {
        throw InvalidArgumentsException(
            "getObjectFromComponents: NULL pNotifyableSP");
    }
    pNotifyableSP->reset();

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!isValidKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getObjectFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return false;
    }

    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return false;
    }

    shared_ptr<NotifyableImpl> parent;
    bool finished = getOps()->getNotifyableFromComponents(
        getRegisteredParentNameVec(),
        components, 
        parentGroupCount, 
        accessType, 
        msecTimeout,
        &parent);
    if ((finished == false) || (parent == NULL)) {
        LOG_WARN(CL_LOG, "getObjectFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return false;
    }

    LOG_DEBUG(CL_LOG, 
              "getObjectFromComponents: parent key = %s, "
              "application name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());
    return getOps()->getNotifyableWaitMsecs(
        parent,
        registeredName(),
        components.at(elements - 1),
        accessType,
        msecTimeout,
        pNotifyableSP);
}

const vector<string> &
RegisteredNotifyableImpl::getRegisteredParentNameVec() const
{
    TRACE(CL_LOG, "getRegisteredParentNameVec");

    ReadLocker l(getRdWrLock());

    return m_registeredParentNameVec;
}

void
RegisteredNotifyableImpl::setRegisteredParentNameVec(
    const vector<string> &registeredParentNameVec)
{
    TRACE(CL_LOG, "setRegisteredParentNameVec");

    WriteLocker l(getRdWrLock());

    m_registeredParentNameVec = registeredParentNameVec;
}

const RdWrLock &
RegisteredNotifyableImpl::getRdWrLock() const
{
    return m_rdWrLock;
}

}	/* End of 'namespace clusterlib' */
