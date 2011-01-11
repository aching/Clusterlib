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

shared_ptr<Root>
Shard::getRoot() const
{
    return m_rootSP;
}

HashRange &
Shard::getStartRange() const 
{ 
    if (m_startRange == NULL) {
        throw InvalidMethodException("getStartRange: No start range set.");
    }
    return *m_startRange;
}
    
HashRange &
Shard::getEndRange() const 
{
    if (m_endRange == NULL) {
        throw InvalidMethodException("getEndRange: No end range set.");
    }
    return *m_endRange;
}

shared_ptr<Notifyable>
Shard::getNotifyable() const
{
    if (m_notifyableKey.empty()) {
        return shared_ptr<Notifyable>();
    }
    else {
        return m_rootSP->getNotifyableFromKey(m_notifyableKey);
    }
}

string 
Shard::getNotifyableKey() const 
{
    return m_notifyableKey;
}

int32_t
Shard::getPriority() const
{
    return m_priority;
}

Shard::Shard(const shared_ptr<Root> &rootSP,
             const HashRange &startRange, 
             const HashRange &endRange, 
             string notifyableKey, 
             int32_t priority)
    : m_rootSP(rootSP),
      m_startRange(&(startRange.create())),
      m_endRange(&(endRange.create())),
      m_notifyableKey(notifyableKey),
      m_priority(priority) 
{
    *m_startRange = startRange;
    *m_endRange = endRange;
}

Shard::Shard()
    : m_startRange(NULL),
      m_endRange(NULL),
      m_priority(-1) 
{
}

Shard::Shard(const Shard &other)
    : m_rootSP(other.getRoot()),
      m_startRange(&(other.getStartRange().create())),
      m_endRange(&(other.getEndRange().create())),
      m_notifyableKey(other.getNotifyableKey()),
      m_priority(other.getPriority())
{
    *m_startRange = other.getStartRange();
    *m_endRange = other.getEndRange();
}

Shard::~Shard() 
{
    if (m_startRange != NULL) {
        delete m_startRange;
    }
    if (m_endRange != NULL) {
        delete m_endRange;
    }
}

Shard & 
Shard::operator= (const Shard &other)
{
    if (m_startRange == NULL) {
        m_startRange = &(other.getStartRange().create());
        *m_startRange = other.getStartRange();
    }
    if (m_endRange == NULL) {
        m_endRange = &(other.getEndRange().create());
        *m_endRange = other.getEndRange();
    }
    m_rootSP = other.getRoot();
    m_notifyableKey = other.getNotifyableKey();
    m_priority = other.getPriority();
    
    return *this;
}

}	/* End of 'namespace clusterlib' */

