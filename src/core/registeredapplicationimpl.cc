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

const string &
RegisteredApplicationImpl::registeredName() const
{
    return CLString::REGISTERED_APPLICATION_NAME;
}

string
RegisteredApplicationImpl::generateKey(const string &parentKey, 
                                       const string &name) const
{
    return NotifyableKeyManipulator::createApplicationKey(name);
}

bool
RegisteredApplicationImpl::isValidName(const string &name) const
{
    TRACE(CL_LOG, "isValidName");

    if ((name.compare(CLString::DEFAULT_CLI_APPLICATION)) &&
        (!NotifyableKeyManipulator::isValidNotifyableName(name))) {
        LOG_WARN(CL_LOG,
                 "isValidName: Illegal Application name %s",
                 name.c_str());
        return false;
    }
    else {
        return true;
    }
}

shared_ptr<NotifyableImpl>
RegisteredApplicationImpl::createNotifyable(
    const string &notifyableName,
    const string &notifyableKey,
    const shared_ptr<NotifyableImpl> &parent,
    FactoryOps &factoryOps) const
{
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<ApplicationImpl>(new ApplicationImpl(&factoryOps,
                                                        notifyableKey,
                                                        notifyableName,
                                                        parent)));
}

vector<string>
RegisteredApplicationImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        NotifyableKeyManipulator::createGroupChildrenKey(notifyableKey));
    resVec.push_back(
        NotifyableKeyManipulator::createDataDistributionChildrenKey(
            notifyableKey));
    resVec.push_back(
        NotifyableKeyManipulator::createNodeChildrenKey(notifyableKey));

    return resVec;
}

bool
RegisteredApplicationImpl::isValidKey(const vector<string> &components,
                                      int32_t elements)
{
    TRACE(CL_LOG, "isValidKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isValidKey: elements %" PRId32 
                  " > size of components %" PRIuPTR,
                  elements,
                  components.size());
        throw InvalidArgumentsException("isValidKey: elements > size of "
                                        "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (elements != CLNumericInternal::APP_COMPONENTS_COUNT) {
        return false;
    }

    if ((components.at(CLNumericInternal::CLUSTERLIB_INDEX) != 
         CLStringInternal::CLUSTERLIB) ||
        (components.at(CLNumericInternal::ROOT_INDEX) != 
         CLString::ROOT_DIR) ||
        (components.at(CLNumericInternal::APP_INDEX) != 
         CLString::APPLICATION_DIR)) {
        return false;
    } 

    return true;    
}

RegisteredApplicationImpl::RegisteredApplicationImpl(FactoryOps *factoryOps)
    : RegisteredNotifyableImpl(factoryOps) 
{
    setRegisteredParentNameVec(
        vector<string>(1, CLString::REGISTERED_ROOT_NAME));
}

}	/* End of 'namespace clusterlib' */
