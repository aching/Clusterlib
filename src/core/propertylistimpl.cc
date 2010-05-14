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

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib
{

CachedKeyValues &
PropertyListImpl::cachedKeyValues()
{
    return m_cachedKeyValues;
}

PropertyList *
PropertyListImpl::getPropertyList(const string &name,
                                  AccessType accessType)
{
    throw InvalidMethodException(
        "getPropertyList() called on a PropertyList object!");
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
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::KEYVAL_JSON_OBJECT);

    return res;
}

};	/* End of 'namespace clusterlib' */
