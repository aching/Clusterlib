/*
 * clusterlibrpcrequestmanager.h --
 *
 * Manages multiple ClusterlibRPCRequest objects.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef _CL_CLUSTERLIBRPCREQUESTMANAGER_H_
#define _CL_CLUSTERLIBRPCREQUESTMANAGER_H_
 
namespace clusterlib 
{

/**
 * Manages mulitple ClusterlibRPCRequest objects.
 */ 
class ClusterlibRPCBulkRequest
{
  public:
    /**
     * Add a request to the object.  The allocated memory is now owned
     * by this object and will be released during destruction.
     *
     * @param request The heap-allocated request now owned by this object.
     */
    void addRequest(ClusterlibRPCRequest *request);

    /**
     * Send all the requests.  User may have also chosen to have sent
     * the requests themselves and then only called waitAll().
     */
    void sendAll();

    /**
     * Wait for all the responses.  Can be called multiple times after
     * the requests have been sent.
     *
     * @param maxWaitMsecs The maximum msecs to wait for all the responses.
     * @param perRequestWaitMsecs The time to wait for each request (0 
     *        means no wait)
     * @param pollIntervalMsecs Msecs to wait in between retrying all responses
     * @param method If set, will update the status.
     * @return The number of remaining requests (0 is done)
     */
    uint32_t waitAll(int64_t maxWaitMsecs, 
                     int64_t perRequestWaitMsecs,
                     int64_t pollIntervalMsecs,
                     ClusterlibRPCMethod *method = NULL);

    /**
     * Destructor.
     */
    ~ClusterlibRPCBulkRequest();

  private:
    /**
     * Thread safe object.
     */
    RdWrLock m_rdWrLock;

    /**
     * The vector of requests.
     */
    std::vector<ClusterlibRPCRequest *> m_requestVec;
};

}

#endif
