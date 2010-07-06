/*
 * cachedprocessinfoimpl.cc --
 *
 * Implementation of the CachedProcessInfoImpl class.
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

CachedProcessInfoImpl::CachedProcessInfoImpl(NotifyableImpl *ntp)
    : CachedDataImpl(ntp) 
{
}

int32_t
CachedProcessInfoImpl::publish(bool unconditional)
{
    TRACE(CL_LOG, "publish");

    getNotifyable()->throwIfRemoved();

    string processInfoKey = 
        ProcessSlotImpl::createProcessInfoJsonArrKey(
            getNotifyable()->getKey());

    Locker l(&getCachedDataLock());

    string encodedJsonObject = JSONCodec::encode(m_portArr);
    LOG_INFO(CL_LOG,
             "Tried to publish process info for notifyable %s to %s "
             "with current version %d, unconditional %d\n",
             getNotifyable()->getKey().c_str(), 
             encodedJsonObject.c_str(),
             getVersion(),
             unconditional);

    Stat stat;
    try {
        SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                         processInfoKey,
                         encodedJsonObject,
                         ((unconditional == false) ? getVersion(): -1),
                         &stat),
                     "Setting of %s failed: %s",
                     processInfoKey.c_str(),
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

void
CachedProcessInfoImpl::loadDataFromRepository(bool setWatchesOnly)
{
    TRACE(CL_LOG, "loadDataFromRepository");
    
    string processInfoKey = ProcessSlotImpl::createProcessInfoJsonArrKey(
        getNotifyable()->getKey());
    string encodedJsonValue;
    Stat stat;

    Locker l(&getCachedDataLock());

    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processInfoKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_PROCESSINFO_CHANGE),
            &stat),
        getOps()->getRepository()->getNodeData(
            processInfoKey, encodedJsonValue, NULL, NULL, &stat),
        CachedObjectChangeHandlers::PROCESSSLOT_PROCESSINFO_CHANGE,
        processInfoKey,
        "Loading processInfoKey %s failed: %s",
        processInfoKey.c_str(),
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

    m_portArr = 
        JSONCodec::decode(encodedJsonValue).get<JSONValue::JSONArray>();
}

JSONValue::JSONArray
CachedProcessInfoImpl::getHostnameArr()
{
    Locker l(&getCachedDataLock());

    return m_hostnameArr;
}

void
CachedProcessInfoImpl::setHostnameArr(const JSONValue::JSONArray &hostnameArr)
{
    Locker l(&getCachedDataLock());

    m_hostnameArr = hostnameArr;
}

JSONValue::JSONArray
CachedProcessInfoImpl::getPortArr()
{
    Locker l(&getCachedDataLock());

    return m_portArr;
}

void
CachedProcessInfoImpl::setPortArr(const JSONValue::JSONArray &portArr)
{
    Locker l(&getCachedDataLock());

    m_portArr = portArr;
}

JSONValue::JSONArray
CachedProcessInfoImpl::marshal()
{
    TRACE(CL_LOG, "marshal");

    getNotifyable()->throwIfRemoved();
    
    Locker l(&getCachedDataLock());

    JSONValue::JSONArray jsonArr;
    jsonArr.push_back(m_hostnameArr);
    jsonArr.push_back(m_portArr);
    
    return jsonArr;
}

void
CachedProcessInfoImpl::unmarshal(const string &encodedJsonArr)
{
    TRACE(CL_LOG, "unmarshal");

    getNotifyable()->throwIfRemoved();
    ostringstream oss;

    Locker l(&getCachedDataLock());
    
    JSONValue::JSONArray jsonArr = 
        JSONCodec::decode(encodedJsonArr).get<JSONValue::JSONArray>();
    if (jsonArr.size() != 2) {
        oss.str();
        oss << "unmarshal: Size should be 2 and is " << jsonArr.size();
        throw InconsistentInternalStateException(oss.str());
    }
    m_hostnameArr = jsonArr[0].get<JSONValue::JSONArray>();
    m_portArr = jsonArr[1].get<JSONValue::JSONArray>();
}

};	/* End of 'namespace clusterlib' */
