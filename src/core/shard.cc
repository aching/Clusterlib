/*
 * shard.cc --
 *
 * Implementation of the class Shard; it represents an element in a data
 * distribution.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#include "clusterlibinternal.h"

using namespace std;

namespace clusterlib
{

Root *
Shard::getRoot() const
{
    return m_root;
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

Notifyable *
Shard::getNotifyable() const
{
    if (m_notifyableKey.empty()) {
        return NULL;
    }
    else {
        return m_root->getNotifyableFromKey(m_notifyableKey);
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

Shard::Shard(Root *root,
             const HashRange &startRange, 
             const HashRange &endRange, 
             std::string notifyableKey, 
             int32_t priority)
    : m_root(root),
      m_startRange(&(startRange.create())),
      m_endRange(&(endRange.create())),
      m_notifyableKey(notifyableKey),
      m_priority(priority) 
{
    *m_startRange = startRange;
    *m_endRange = endRange;
}

Shard::Shard()
    : m_root(NULL),
      m_startRange(NULL),
      m_endRange(NULL),
      m_priority(-1) 
{
}

Shard::Shard(const Shard &other)
    : m_root(other.getRoot()),
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
    m_root = other.getRoot();
    m_notifyableKey = other.getNotifyableKey();
    m_priority = other.getPriority();
    
    return *this;
}

}	/* End of 'namespace clusterlib' */

