/* 
 * stopprocessmethod.cc --
 *
 * Implementation of the GenericMethod class.

 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace clusterlib;
using namespace json;
using namespace json::rpc;
using namespace log4cxx;
using namespace std;

namespace clusterlib {

const string &
GenericRPC::getName() const
{
    return m_requestName;
}

void
GenericRPC::checkParams(const JSONValue::JSONArray &paramArr)
{
    if (paramArr.size() != 1) {
        throw JSONRPCInvocationException(
            "checkParams: Expecting one array element");
    }
}

JSONValue::JSONArray
GenericRequest::marshalParams()
{
    TRACE(CL_LOG, "marshalParams");

    return getRPCParams();
}

}
