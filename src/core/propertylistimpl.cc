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
#include <boost/regex.hpp>

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib {

CachedKeyValues &
PropertyListImpl::cachedKeyValues()
{
    return m_cachedKeyValues;
}

bool
PropertyListImpl::getPropertyListWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<PropertyList> *pPropertyListSP)
{
    throw InvalidMethodException(
        "getPropertyListWaitMsecs() called on a PropertyList object!");
}

PropertyListImpl::PropertyListImpl(FactoryOps *fp,
                                   const string &key,
                                   const string &name,
                                   const shared_ptr<NotifyableImpl> &parent)
    : NotifyableImpl(fp, key, name, parent),
      m_cachedKeyValues(this)
{
}

NotifyableList
PropertyListImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");

    throwIfRemoved();
    
    return NotifyableList();
}

void 
PropertyListImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    m_cachedKeyValues.loadDataFromRepository(false);
}

string
PropertyListImpl::createKeyValJsonObjectKey(const string &propertyListKey)
{
    string res;
    res.append(propertyListKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::KEYVAL_JSON_OBJECT);

    return res;
}

}	/* End of 'namespace clusterlib' */
