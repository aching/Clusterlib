/*
 * cachedstateimpl.cc --
 *
 * Implementation of the CachedStateImpl class.
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

CachedStateImpl::CachedStateImpl(NotifyableImpl *notifyable,
                                 StateType stateType)
    : CachedDataImpl(notifyable),
      m_maxHistorySize(5),
      m_stateType(stateType)
{
}

int32_t
CachedStateImpl::publish(bool unconditional)
{
    TRACE(CL_LOG, "publish");

    getNotifyable()->throwIfRemoved();

    string stateKey = 
        NotifyableImpl::createStateJSONArrayKey(
            getNotifyable()->getKey(), m_stateType);

    Locker l(&getCachedDataLock());

    /*
     * Add the time set to each new state and chop down to the max
     * number of historical states if necessary.
     */
    int64_t msecs = TimerService::getCurrentTimeMsecs();
    m_state[ClusterlibStrings::STATE_SET_MSECS] = msecs;
    m_state[ClusterlibStrings::STATE_SET_MSECS_AS_DATE] = 
        TimerService::getMsecsTimeString(msecs);
    m_historyArr.push_front(m_state);
    if (static_cast<int32_t>(m_historyArr.size()) > m_maxHistorySize) {
        m_historyArr.resize(m_maxHistorySize);
    }

    string encodedJsonObject = JSONCodec::encode(m_historyArr);
    LOG_INFO(CL_LOG,
             "Tried to publish state for notifyable %s to %s "
             "with current version %d, unconditional %d\n",
             getNotifyable()->getKey().c_str(), 
             encodedJsonObject.c_str(),
             getVersion(),
             unconditional);

    Stat stat;
    try {
        SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                         stateKey,
                         encodedJsonObject,
                         ((unconditional == false) ? getVersion(): -1),
                         &stat),
                     "Setting of %s failed: %s",
                     stateKey.c_str(),
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
CachedStateImpl::loadDataFromRepository(bool setWatchesOnly)
{
    TRACE(CL_LOG, "loadDataFromRepository");
    
    string stateKey = 
        NotifyableImpl::createStateJSONArrayKey(
            getNotifyable()->getKey(), m_stateType);
    string encodedJsonValue;
    Stat stat;

    Locker l(&getCachedDataLock());

    CachedObjectChangeHandlers::CachedObjectChange change;    
    if (m_stateType == CURRENT_STATE) {
        change = CachedObjectChangeHandlers::CURRENT_STATE_CHANGE;
    }
    else if (m_stateType == DESIRED_STATE) {
        change = CachedObjectChangeHandlers::DESIRED_STATE_CHANGE;
    }
    else {
        throw InconsistentInternalStateException(
            "loadDataFromRepository: No valid stateType");
    }
    CachedObjectEventHandler *handler = 
        getOps()->getCachedObjectChangeHandlers()->getChangeHandler(change);

    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            stateKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            handler,
            &stat),
        getOps()->getRepository()->getNodeData(
            stateKey, encodedJsonValue, NULL, NULL, &stat),
        change,
        stateKey,
        "Loading stateKey %s failed: %s",
        stateKey.c_str(),
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

    m_historyArr = 
        JSONCodec::decode(encodedJsonValue).get<JSONValue::JSONArray>();
    if (m_historyArr.size() > 0) {
        m_state = m_historyArr[0].get<JSONValue::JSONObject>();
    }
}

int32_t
CachedStateImpl::getMaxHistorySizePublished()
{
    TRACE(CL_LOG, "getMaxHistorySizePublished");

    Locker l(&getCachedDataLock());

    return m_maxHistorySize;
}

void
CachedStateImpl::setMaxHistorySizePublished(int32_t maxHistorySize)
{
    TRACE(CL_LOG, "setMaxHistorySizePublished");

    if (maxHistorySize < 1) {
        ostringstream oss;
        oss << "setMaxHistorySizePublished: Tried to set maxHistorySize with "
            << maxHistorySize << " and must be >= 1";
        throw InvalidArgumentsException(oss.str());
    }

    Locker l(&getCachedDataLock());

    m_maxHistorySize = maxHistorySize;
}

int32_t
CachedStateImpl::getHistorySize()
{
    TRACE(CL_LOG, "getHistorySize");

    Locker l(&getCachedDataLock());

    return m_historyArr.size();
}

vector<JSONValue::JSONString> 
CachedStateImpl::getHistoryKeys(int32_t stateIndex)
{
    TRACE(CL_LOG, "getHistoryKeys");

    Locker l(&getCachedDataLock());

    if ((static_cast<int32_t>(m_historyArr.size()) <= stateIndex) || 
        (stateIndex < 0)) {
        ostringstream oss;
        oss << "getHistoryKeys: Invalid index " << stateIndex
            << " with size " << m_historyArr.size();
        throw InvalidArgumentsException(oss.str());
    }

    vector<JSONValue::JSONValue::JSONString> keyVec;
    JSONValue::JSONObject::const_iterator keyIt;
    JSONValue::JSONObject jsonObj = 
        m_historyArr[stateIndex].get<JSONValue::JSONObject>();
    for (keyIt = jsonObj.begin(); keyIt != jsonObj.end(); ++keyIt) {
        keyVec.push_back(keyIt->first);
    }
    
    return keyVec;
}

bool
CachedStateImpl::getHistory(int32_t stateIndex,
                            const string &key, 
                            JSONValue &jsonValue)
{
    TRACE(CL_LOG, "getHistory");

    Locker l(&getCachedDataLock());

    if ((static_cast<int32_t>(m_historyArr.size()) <= stateIndex) || 
        (stateIndex < 0)) {
        ostringstream oss;
        oss << "getHistoryKeys: Invalid index " << stateIndex
            << " with size " << m_historyArr.size();
        throw InvalidArgumentsException(oss.str());
    }

    
    JSONValue::JSONObject jsonObj =
        m_historyArr[stateIndex].get<JSONValue::JSONObject>();
    JSONValue::JSONObject::const_iterator keyIt = jsonObj.find(key);
    if (keyIt == jsonObj.end()) {
        return false;
    }
    else {
        jsonValue = keyIt->second;
        return true;
    }
}

bool
CachedStateImpl::get(const string &key, 
                     json::JSONValue &jsonValue)
{
    TRACE(CL_LOG, "get");

    Locker l(&getCachedDataLock());
    
    JSONValue::JSONObject::const_iterator keyIt = m_state.find(key);
    if (keyIt == m_state.end()) {
        return false;
    }
    else {
        jsonValue = keyIt->second;
        return true;
    }
}   

void
CachedStateImpl::set(const ::json::JSONValue::JSONString &key, 
                     const ::json::JSONValue &value)
{
    TRACE(CL_LOG, "set");

    Locker l(&getCachedDataLock());
    
    m_state[key] = value;
}

bool 
CachedStateImpl::erase(const ::json::JSONValue::JSONString &key)
{
    TRACE(CL_LOG, "erase");

    Locker l(&getCachedDataLock());

    JSONValue::JSONObject::iterator keyIt = m_state.find(key);
    if (keyIt == m_state.end()) {
        return false;
    }
    else {
        m_state.erase(keyIt);
        return true;
    }
}

void
CachedStateImpl::clear()
{
    TRACE(CL_LOG, "clear");

    Locker l(&getCachedDataLock());

    m_state.clear();
}

JSONValue::JSONArray
CachedStateImpl::getHistoryArray()
{
    TRACE(CL_LOG, "getHistoryArray");

    Locker l(&getCachedDataLock());

    return m_historyArr;
}

}	/* End of 'namespace clusterlib' */
