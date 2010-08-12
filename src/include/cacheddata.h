/*
 * cacheddata.h --
 *
 * Definition of class CachedData; it represents data that is stored
 * in the clusterlib repository.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDDATA_H_
#define _CL_CACHEDDATA_H_

namespace clusterlib {

/**
 * Definition of class CachedData
 */
class CachedData
{
  public:
    /**
     * Reset the cached data with the current repository data.
     */
    virtual void reset() = 0;

    /**
     * No changes to the CachedData will be pushed to the repository
     * (visible to other clusterlib clients) until the data has been
     * successfully published.  It is possible that an exception from
     * clusterlib may be thrown if the versions don't match
     * (PublishVersionException) and <code>unconditional==
     * false</code>.  In this case, the user should catch the
     * exception, release the lock and wait until the CachedData is
     * updated (either through polling or waiting on events).  Then
     * they should try to set their CachedData again under a lock and
     * publish again.
     *
     * @param unconditional If true, publish the data from this object
     *        even if it is not the latest version.
     * @return the published version
     */
    virtual int32_t publish(bool unconditional = false) = 0;

    /**
     * Simpler interface to get the version of this repository data.
     *
     * @return the current cached version
     */
    virtual int32_t getVersion() = 0;

    /**
     * Get cached statistics about the repository data.  If any of the
     * parameters are NULL, they will not be filled in.
     *
     * @param czxid The zxid of the change that caused this znode
     *        to be created.
     * @param mzxid The zxid of the change that last modified this znode.
     * @param ctime The time in milliseconds from epoch when this znode 
     *        was created.
     * @param mtime The time in milliseconds from epoch when this znode was 
     *        last modified.
     * @param version The number of changes to the data of this znode.
     * @param cversion The number of changes to the children of this znode.
     * @param aversion The number of changes to the ACL of this znode.
     * @param ephemeralOwner The session id of the owner of this znode if
     *        the znode is an ephemeral node. If it is not an ephemeral 
     *        node, it will be zero.
     * @param dataLength The length of the data field of this znode.
     * @param numChildren The number of children of this znode.
     * @param pzxid Unknown.
     */
    virtual void getStats(int64_t *czxid = NULL,
                          int64_t *mzxid = NULL,
                          int64_t *ctime = NULL,
                          int64_t *mtime = NULL,
                          int32_t *version = NULL,
                          int32_t *cversion = NULL,
                          int32_t *aversion = NULL,
                          int64_t *ephemeralOwner = NULL,
                          int32_t *dataLength = NULL,
                          int32_t *numChildren = NULL,
                          int64_t *pzxid = NULL) = 0;
    
    /**
     * Destructor.
     */
    virtual ~CachedData() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NODE_H_ */
