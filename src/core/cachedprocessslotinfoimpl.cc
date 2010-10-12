/*
 * cachedprocessslotinfoimpl.cc --
 *
 * Implementation of the CachedProcessSlotInfoImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib {

CachedProcessSlotInfoImpl::CachedProcessSlotInfoImpl(
    NotifyableImpl *notifyable)
    : CachedDataImpl(notifyable) 
{
    m_processSlotInfoArr.resize(CachedProcessSlotInfoImpl::MAX_SIZE);
    m_processSlotInfoArr[CachedProcessSlotInfoImpl::ENABLE] = false;
    m_processSlotInfoArr[CachedProcessSlotInfoImpl::MAX_PROCESSES] = -1;
}

int32_t
CachedProcessSlotInfoImpl::publish(bool unconditional)
{
    TRACE(CL_LOG, "publish");

    getNotifyable()->throwIfRemoved();

    string processSlotInfoKey = 
        NodeImpl::createProcessSlotInfoJSONObjectKey(
            getNotifyable()->getKey());

    Locker l(&getCachedDataLock());

    string encodedJsonObject = JSONCodec::encode(m_processSlotInfoArr);
    LOG_DEBUG(CL_LOG,
              "Tried to publish process slot info for notifyable %s to %s "
              "with current version %d, unconditional %d\n",
              getNotifyable()->getKey().c_str(), 
              encodedJsonObject.c_str(),
              getVersion(),
              unconditional);

    Stat stat;
    try {
        SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                         processSlotInfoKey,
                         encodedJsonObject,
                         ((unconditional == false) ? getVersion(): -1),
                         &stat),
                     "Setting of %s failed: %s",
                     processSlotInfoKey.c_str(),
                     false,
                     true);
    } catch (const zk::BadVersionException &e) {
        throw PublishVersionException(e.what());
    }
    
    /* 
     * Since we should have the lock, the data should be identical to
     * the zk data.  When the lock is released, clusterlib events will
     * try to push this change again.  
     */
    setStat(stat);
    return stat.version;
}

bool
CachedProcessSlotInfoImpl::getEnable()
{
    Locker l(&getCachedDataLock());

    return m_processSlotInfoArr[CachedProcessSlotInfoImpl::ENABLE].
        get<JSONValue::JSONBoolean>();
}

void
CachedProcessSlotInfoImpl::setEnable(bool enable)
{
    Locker l(&getCachedDataLock());

    m_processSlotInfoArr[CachedProcessSlotInfoImpl::ENABLE] = enable;
}

int32_t
CachedProcessSlotInfoImpl::getMaxProcessSlots()
{
    Locker l(&getCachedDataLock());

    return m_processSlotInfoArr[CachedProcessSlotInfoImpl::MAX_PROCESSES].
        get<JSONValue::JSONInteger>();
}

void
CachedProcessSlotInfoImpl::setMaxProcessSlots(int32_t maxProcessSlots)
{
    Locker l(&getCachedDataLock());

    m_processSlotInfoArr[CachedProcessSlotInfoImpl::MAX_PROCESSES] = 
        maxProcessSlots;
}

void
CachedProcessSlotInfoImpl::loadDataFromRepository(bool setWatchesOnly)
{
    TRACE(CL_LOG, "loadDataFromRepository");
    
    string processSlotInfoKey = 
        NodeImpl::createProcessSlotInfoJSONObjectKey(
            getNotifyable()->getKey());
    string encodedJsonValue;
    Stat stat;

    Locker l(&getCachedDataLock());

    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotInfoKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::NODE_PROCESS_SLOT_INFO_CHANGE),
            &stat),
        getOps()->getRepository()->getNodeData(
            processSlotInfoKey, encodedJsonValue, NULL, NULL, &stat),
        CachedObjectChangeHandlers::NODE_PROCESS_SLOT_INFO_CHANGE,
        processSlotInfoKey,
        "Loading processSlotInfoKey %s failed: %s",
        processSlotInfoKey.c_str(),
        false,
        true);

    if (setWatchesOnly) {
        return;
    }

    if (!updateStat(stat)) {
        return;
    }

    /* 
     * Default values from the constructor are used when there are
     * empty nodes 
     */
    if (encodedJsonValue.empty()) {
        return;
    }

    m_processSlotInfoArr = 
        JSONCodec::decode(encodedJsonValue).get<JSONValue::JSONArray>();

    if (m_processSlotInfoArr.size() != CachedProcessSlotInfoImpl::MAX_SIZE) {
        ostringstream oss;
        oss << "loadDataFromRepository: Retrieved '" << encodedJsonValue
            << "' from " << processSlotInfoKey << " with size " 
            << m_processSlotInfoArr.size() << " and should be " 
            << CachedProcessSlotInfoImpl::MAX_SIZE;
        throw InconsistentInternalStateException(oss.str());
    }
}

}	/* End of 'namespace clusterlib' */
