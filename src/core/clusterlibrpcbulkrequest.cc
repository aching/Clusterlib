/*
 * clusterlibrpcbulkrequest.cc --
 *
 * Implementation of the ClusterlibRPCBulkRequest class.
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
using namespace json::rpc;

namespace clusterlib
{

void
ClusterlibRPCBulkRequest::addRequest(ClusterlibRPCRequest *request) 
{
    TRACE(CL_LOG, "addRequest");

    WriteLocker l(&m_rdWrLock);

    m_requestVec.push_back(request);
}

void
ClusterlibRPCBulkRequest::sendAll()
{
    TRACE(CL_LOG, "sendAll");

    ReadLocker l(&m_rdWrLock);

    vector<ClusterlibRPCRequest *>::iterator requestVecIt;
    for (requestVecIt = m_requestVec.begin(); 
         requestVecIt != m_requestVec.end();
         ++requestVecIt) {
        (*requestVecIt)->sendRequest();
    }
}

uint32_t
ClusterlibRPCBulkRequest::waitAll(
    int64_t maxWaitMsecs, 
    int64_t perRequestWaitMsecs, 
    int64_t pollIntervalMsecs,
    ClusterlibRPCMethod *method)
{
    TRACE(CL_LOG, "sendAll");

    ReadLocker l(&m_rdWrLock);

    ostringstream oss;
    int64_t currentMsecs = TimerService::getCurrentTimeMsecs();
    int64_t maxMsecs = currentMsecs + maxWaitMsecs;
    vector<ClusterlibRPCRequest *>::const_iterator requestVecIt;
    size_t remainingRequestCount = 0;
    bool finishedReq = false;
    while (currentMsecs < maxMsecs) {
        remainingRequestCount = m_requestVec.size();
        for (requestVecIt = m_requestVec.begin();
             requestVecIt != m_requestVec.end();
             ++requestVecIt) {
            finishedReq = (*requestVecIt)->waitMsecsResponse(
                perRequestWaitMsecs);
            if (finishedReq) {
                --remainingRequestCount;
                if ((*requestVecIt)->getResponseError().type() !=
                    typeid(JSONValue::JSONNull)) {
                    oss.str("");
                    oss << "waitAll: request to recving queue "
                        << JSONCodec::encode((*requestVecIt)->getDestination())
                        << " failed: "
                        << JSONCodec::encode((*requestVecIt)->getResponse());
                    throw JSONRPCInvocationException(oss.str());
                }
            }
        }

        if (method != NULL) {
            oss.str("");
            oss << "waitAll: Waiting for " << remainingRequestCount
                << " of " << m_requestVec.size() << " total responses";
            LOG_INFO(CL_LOG, "%s", oss.str().c_str());
            method->setMethodStatus(oss.str());
        }

        if (remainingRequestCount == 0) {
            break;
        }
        else {
            usleep(pollIntervalMsecs*1000);
        }
        currentMsecs = TimerService::getCurrentTimeMsecs();
    }

    return remainingRequestCount;
}

ClusterlibRPCBulkRequest::~ClusterlibRPCBulkRequest()
{
    WriteLocker l(&m_rdWrLock);

    vector<ClusterlibRPCRequest *>::const_iterator requestVecIt;
    for (requestVecIt = m_requestVec.begin();
         requestVecIt != m_requestVec.end();
         ++requestVecIt) {
        delete *requestVecIt;
    }
}

}	/* End of 'namespace clusterlib' */
