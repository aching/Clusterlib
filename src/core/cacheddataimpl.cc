/*
 * cachedataimpl.cc --dataLengthralOwner
 * Implementation of the CachedDataImpl class.
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

namespace clusterlib
{

int32_t
CachedDataImpl::getVersion()
{
    Locker l(&getCachedDataLock());

    return m_stat.version;
}

void
CachedDataImpl::getStats(int64_t *czxid,
                         int64_t *mzxid,
                         int64_t *ctime,
                         int64_t *mtime,
                         int32_t *version,
                         int32_t *cversion,
                         int32_t *aversion,
                         int64_t *ephemeralOwner,
                         int32_t *dataLength,
                         int32_t *numChildren,
                         int64_t *pzxid)
{
    Locker l(&getCachedDataLock());

    if (czxid != NULL) {
        *czxid = m_stat.czxid;
    }
    if (mzxid != NULL) {
        *mzxid = m_stat.mzxid;
    }
    if (ctime != NULL) {
        *ctime = m_stat.ctime;
    }
    if (mtime != NULL) {
        *mtime = m_stat.mtime;
    }
    if (version != NULL) {
        *version = m_stat.version;
    }
    if (cversion != NULL) {
        *cversion = m_stat.cversion;
    }
    if (aversion != NULL) {
        *aversion = m_stat.aversion;
    }
    if (ephemeralOwner != NULL) {
        *ephemeralOwner = m_stat.ephemeralOwner;
    }
    if (dataLength != NULL) {
        *dataLength = m_stat.dataLength;
    }
    if (numChildren != NULL) {
        *numChildren = m_stat.numChildren;
    }
    if (pzxid != NULL) {
        *pzxid = m_stat.pzxid;
    }
}

CachedDataImpl::CachedDataImpl(NotifyableImpl *ntp)
    : m_notifyable(ntp) 
{
    Locker l(&getCachedDataLock());

    memset(&m_stat, 0, sizeof(m_stat));
    m_stat.version = ClusterlibInts::INITIAL_ZK_VERSION;
}

FactoryOps *
CachedDataImpl::getOps()
{
    return m_notifyable->getOps();
}

bool
CachedDataImpl::updateStat(const Stat &stat)
{
    TRACE(CL_LOG, "updateStat");

    ostringstream oss;
    if (stat.version < 0) {
        oss << "updateStat: Bad stat version " << stat.version;
        throw InvalidArgumentsException(oss.str());
    }

    LOG_DEBUG(CL_LOG, 
              "updateStat: old version = %d, new version = %d",
              m_stat.version,
              stat.version);

    /*
     * If the current version of the stat is INITIAL_ZK_VERSION, then
     * accept the new stat.  If the stat is older than our stat, this
     * is impossible to received updates about the past.
     */
    if ((m_stat.version == ClusterlibInts::INITIAL_ZK_VERSION) ||
        (m_stat.version < stat.version)) {
        m_stat = stat;
        return true;
    }
    else if (m_stat.version > stat.version) {
        oss << "updateStat: Impossible that new version is "
            << stat.version << " and old version is " << m_stat.version;
        throw InconsistentInternalStateException(oss.str());
    }
    else {
        /* Don't do anything if equal */
        return false;
    }
}

};	/* End of 'namespace clusterlib' */
