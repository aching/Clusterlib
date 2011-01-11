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
RegisteredProcessSlotImpl::registeredName() const
{
    return CLString::REGISTERED_PROCESSSLOT_NAME;
}

string
RegisteredProcessSlotImpl::generateKey(const string &parentKey, 
                                 const string &name) const
{
    return NotifyableKeyManipulator::createProcessSlotKey(parentKey, name);
}

shared_ptr<NotifyableImpl>
RegisteredProcessSlotImpl::createNotifyable(
    const string &notifyableName,
    const string &notifyableKey,
    const shared_ptr<NotifyableImpl> &parent,
    FactoryOps &factoryOps) const
{
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<ProcessSlotImpl>(new ProcessSlotImpl(&factoryOps,
                                                        notifyableKey,
                                                        notifyableName,
                                                        parent)));
}

vector<string>
RegisteredProcessSlotImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        ProcessSlotImpl::createProcessInfoJsonArrKey(notifyableKey));

    return resVec;
}

bool
RegisteredProcessSlotImpl::isValidKey(const vector<string> &components,
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

    /*
     * Make sure that we have enough elements to have a process slot
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < CLNumericInternal::PROCESSSLOT_COMPONENTS_MIN_COUNT) ||
        (((elements - CLNumericInternal::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent node are valid.
     */
    vector<string> nameVec;
    nameVec.push_back(CLString::REGISTERED_NODE_NAME);
    if (!getOps()->isValidKey(nameVec, components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is PROCESSSLOTS
     * that the distribution name is not empty.
     */
    if ((components.at(elements - 2) != CLString::PROCESSSLOT_DIR) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;        
}

RegisteredProcessSlotImpl::RegisteredProcessSlotImpl(FactoryOps *factoryOps)
    : RegisteredNotifyableImpl(factoryOps)
{
    setRegisteredParentNameVec(
        vector<string>(1, CLString::REGISTERED_NODE_NAME));
}

}	/* End of 'namespace clusterlib' */
