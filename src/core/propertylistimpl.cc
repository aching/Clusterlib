/*
 * propertylistimpl.cc --
 *
 * Implementation of the PropertysListImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
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
