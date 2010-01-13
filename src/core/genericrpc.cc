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

string
GenericRPC::getName()
{
    return ClusterlibStrings::RPC_GENERIC;
}

bool
GenericRPC::checkParams(const JSONValue::JSONArray &paramArr)
{
    try {
        if (paramArr.size() != 1) {
            LOG_ERROR(CL_LOG,
                      "checkParams: Expecting one array element, got %d",
                      paramArr.size());
            return false;
        }
        return true;
    }
    catch (const JSONRPCInvocationException &ex) {
        LOG_WARN(CL_LOG, "checkParams: Failed with %s", ex.what());
        return false;
    }
}

void
GenericRequest::sendRequest(const void *destination)
{
    TRACE(CL_LOG, "sendRequest");

    if (destination == NULL) {
        throw InvalidArgumentsException("sendRequest: Destination is NULL");
    }
    const char *queuePtr = reinterpret_cast<const char *>(destination);
    
    Queue *queue = 
        dynamic_cast<Queue *>(m_root->getNotifyableFromKey(queuePtr));
    if (queue == NULL) {
        throw InvalidArgumentsException(
            string("sendRequest: Invalid queue at key ") + queuePtr);
    }
    else {
        /* Ready the response */
        stringstream idSs;
        idSs << ClientImpl::getHostnamePidTid() 
             << m_client->fetchAndIncrRequestCounter();
        m_id = idSs.str();
        m_client->getOps()->getResponseSignalMap()->addRefPredMutexCond(m_id);

        /* Format according the JSON-RPC 1.0 */
        JSONValue::JSONObject rpcObj;
        rpcObj["method"] = m_requestName;
        rpcObj["params"] = m_paramArr;
        rpcObj["id"] = idSs.str();
        LOG_DEBUG(CL_LOG, 
                  "sendRequest: Putting request (%s) on queue (%s) "
                  "with id (%s)",
                  JSONCodec::encode(rpcObj).c_str(),
                  queue->getKey().c_str(),
                  m_id.c_str());
        queue->put(JSONCodec::encode(rpcObj));
    }
}

}
