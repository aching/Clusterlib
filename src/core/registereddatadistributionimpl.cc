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
RegisteredDataDistributionImpl::registeredName() const
{
    return CLString::REGISTERED_DATADISTRIBUTION_NAME;
}

string
RegisteredDataDistributionImpl::generateKey(const string &parentKey, 
                                            const string &name) const
{
    return NotifyableKeyManipulator::createDataDistributionKey(
        parentKey, name);
}

shared_ptr<NotifyableImpl>
RegisteredDataDistributionImpl::createNotifyable(
    const string &notifyableName,
    const string &notifyableKey,
    const shared_ptr<NotifyableImpl> &parent,
    FactoryOps &factoryOps) const
{
    shared_ptr<GroupImpl> group = dynamic_pointer_cast<GroupImpl>(parent);
    if (group == NULL) {
        throw InvalidArgumentsException("createNotifyable: Got impossible "
                                        "parent that is not a group");
    }
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<DataDistributionImpl>(new DataDistributionImpl(
                                             &factoryOps,
                                             notifyableKey,
                                             notifyableName,
                                             group)));
}

vector<string>
RegisteredDataDistributionImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        DataDistributionImpl::createShardJsonObjectKey(notifyableKey));

    return resVec;
}

bool
RegisteredDataDistributionImpl::isValidKey(const vector<string> &components, 
                                           int32_t elements)
{
    TRACE(CL_LOG, "isValidKey");
    
    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isValidKey: elements %" PRId32 
                  " > size of components %" PRIuPTR,
                  elements,
                  components.size());
        throw InvalidArgumentsException(
            "isValidKey: elements > size of components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a distribution
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < CLNumericInternal::DIST_COMPONENTS_MIN_COUNT) ||
        (((elements - CLNumericInternal::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent group are valid.
     */
    vector<string> nameVec;
    nameVec.push_back(CLString::REGISTERED_APPLICATION_NAME);
    nameVec.push_back(CLString::REGISTERED_GROUP_NAME);
    if (!getOps()->isValidKey(nameVec, components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is DISTRIBUTIONS and
     * that the distribution name is not empty.
     */
    if ((components.at(elements - 2) != CLString::DATADISTRIBUTION_DIR) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

}	/* End of 'namespace clusterlib' */
