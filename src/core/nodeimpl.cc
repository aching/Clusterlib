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

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib {

#define VAL(str) #str
    
// execute the given code or die if it times out
#define LIVE_OR_DIE(code, timeout)  \
{   \
    LOG_DEBUG(CL_LOG, "Setting up a bomb to go off in %d ms if '%s' deadlocks...", timeout, VAL(code));  \
    TimerId timerId = m_timer.scheduleAfter(timeout, VAL(code));   \
    try {   \
        code;   \
        m_timer.cancelAlarm(timerId); \
    } \
    catch (std::exception &e) {   \
        m_timer.cancelAlarm(timerId); \
        LOG_ERROR(CL_LOG, \
                  "An exception while executing '%s': %s",  \
                  VAL(code), e.what()); \
    } \
    catch (...) { \
        m_timer.cancelAlarm(timerId); \
        LOG_ERROR(CL_LOG, \
                  "Unable to execute '%s', unknown exception", VAL(code));  \
    } \
    LOG_DEBUG(CL_LOG, "...disarming the bomb");  \
}

const string Node::HEALTH_KEY = "_health";

const string Node::HEALTH_GOOD_VALUE = "_healthGood";

const string Node::HEALTH_BAD_VALUE = "_healthBad";

const string Node::HEALTH_SET_MSECS_KEY = "_healthSetMsecs";

const string Node::HEALTH_SET_MSECS_AS_DATE_KEY = "_healthSetMsecsAsDate";

const string Node::ACTIVENODE_SHUTDOWN = "_activeNodeShutdown";

CachedProcessSlotInfo &
NodeImpl::cachedProcessSlotInfo()
{
    return dynamic_cast<CachedProcessSlotInfo &>(m_cachedProcessSlotInfo);
}

NameList 
NodeImpl::getProcessSlotNames()
{
    TRACE(CL_LOG, "getProcessSlotNames");

    throwIfRemoved();

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createProcessSlotChildrenKey(getKey()),
        CachedObjectChangeHandlers::PROCESSSLOTS_CHANGE);
}

bool
NodeImpl::getProcessSlotWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<ProcessSlot> *pProcessSlotSP) 
{
    TRACE(CL_LOG, "getProcessSlotWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        CLString::REGISTERED_PROCESSSLOT_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pProcessSlotSP = dynamic_pointer_cast<ProcessSlot>(notifyableSP);
    return completed;
}

shared_ptr<ProcessSlot>
NodeImpl::getProcessSlot(const string &name,
                         AccessType accessType)
{
    shared_ptr<ProcessSlot> processSlotSP;
    getProcessSlotWaitMsecs(name, accessType, -1, &processSlotSP);
    return processSlotSP;
}

string
NodeImpl::createProcessSlotInfoJSONObjectKey(const string &nodeKey)
{
    string res;
    res.append(nodeKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::PROCESSSLOT_INFO_JSON_OBJECT);

    return res;
}

NodeImpl::~NodeImpl()
{
}

NotifyableList
NodeImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");
    
    throwIfRemoved();
    
    return getOps()->getNotifyableList(
        shared_from_this(),
        CLString::REGISTERED_PROCESSSLOT_NAME,
        getProcessSlotNames(),
        LOAD_FROM_REPOSITORY);
}

void
NodeImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Ensure that the cache contains all the information
     * about this node, and that all watches are established.
     */
    m_cachedProcessSlotInfo.loadDataFromRepository(false);
}

}	/* End of 'namespace clusterlib' */
