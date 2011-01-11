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

namespace clusterlib {

NameList
RootImpl::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createApplicationChildrenKey(getKey()),
        CachedObjectChangeHandlers::APPLICATIONS_CHANGE);
}

bool
RootImpl::getApplicationWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<Application> *pApplicationSP) 
{
    TRACE(CL_LOG, "getApplicationWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        CLString::REGISTERED_APPLICATION_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pApplicationSP = dynamic_pointer_cast<Application>(notifyableSP);
    return completed;
}

shared_ptr<Application>
RootImpl::getApplication(const string &name,
                         AccessType accessType)
{
    shared_ptr<Application> applicationSP;
    getApplicationWaitMsecs(name, accessType, -1, &applicationSP);
    return applicationSP;
}

NotifyableList
RootImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");
    
    throwIfRemoved();
    
    return getOps()->getNotifyableList(
        shared_from_this(),
        CLString::REGISTERED_APPLICATION_NAME,
        getApplicationNames(),
        LOAD_FROM_REPOSITORY);
}

void
RootImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");
}

}	/* End of 'namespace clusterlib' */
