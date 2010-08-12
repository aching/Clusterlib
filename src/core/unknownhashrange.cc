/*
 * unknownhashrange.cc --
 *
 * Implementation of the UnknownHashRange class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace json;

namespace clusterlib {

string
UnknownHashRange::name()
{
    return "UnknownHashRange";
}

string
UnknownHashRange::getName() const
{
    return UnknownHashRange::name();
}

bool
UnknownHashRange::isBegin()
{
    throw InvalidMethodException("isBegin: Not allowed for UnknownHashRange");
}

bool
UnknownHashRange::isEnd()
{
    throw InvalidMethodException("isEnd: Not allowed for UnknownHashRange");
}

bool 
UnknownHashRange::operator< (const HashRange &other) const
{
    throw InvalidMethodException("<: Not allowed for UnknownHashRange");
}

bool 
UnknownHashRange::operator== (const HashRange &other) const
{
    throw InvalidMethodException("==: Not allowed for UnknownHashRange");
}

HashRange & 
UnknownHashRange::operator= (const HashRange &other)
{
    m_jsonValue = other.toJSONValue();
    return *this;
}

JSONValue 
UnknownHashRange::toJSONValue() const
{
    return m_jsonValue;
}

void
UnknownHashRange::set(const JSONValue &jsonValue)
{
    m_jsonValue = jsonValue;
}

HashRange &
UnknownHashRange::create() const
{
    return *(dynamic_cast<HashRange *>(new UnknownHashRange()));
}

UnknownHashRange &
UnknownHashRange::operator++ ()
{
    throw InvalidMethodException("++: Not allowed for UnknownHashRange");
}

}	/* End of 'namespace clusterlib' */
