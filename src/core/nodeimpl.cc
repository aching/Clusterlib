/*
 * nodeimpl.cc --
 *
 * Implementation of the NodeImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */
#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace json;

namespace clusterlib
{

#define VAL(str) #str
    
// execute the given code or die if it times out
#define LIVE_OR_DIE(code, timeout)  \
{   \
    LOG_DEBUG(CL_LOG, "Setting up a bomb to go off in %d ms if '%s' deadlocks...", timeout, VAL(code));  \
    TimerId timerId = m_timer.scheduleAfter(timeout, VAL(code));   \
    try {   \
        code;   \
        m_timer.cancelAlarm(timerId); \
    } \
    catch (std::exception &e) {   \
        m_timer.cancelAlarm(timerId); \
        LOG_ERROR(CL_LOG, \
                  "An exception while executing '%s': %s",  \
                  VAL(code), e.what()); \
    } \
    catch (...) { \
        m_timer.cancelAlarm(timerId); \
        LOG_ERROR(CL_LOG, \
                  "Unable to execute '%s', unknown exception", VAL(code));  \
    } \
    LOG_DEBUG(CL_LOG, "...disarming the bomb");  \
}

const string Node::HEALTH_KEY = "_health";

const string Node::HEALTH_GOOD_VALUE = "_healthGood";

const string Node::HEALTH_BAD_VALUE = "_healthBad";

const string Node::HEALTH_SET_MSECS_KEY = "_healthSetMsecs";

const string Node::HEALTH_SET_MSECS_AS_DATE_KEY = "_healthSetMsecsAsDate";

const string Node::ACTIVENODE_SHUTDOWN = "_activeNodeShutdown";

CachedProcessSlotInfo &
NodeImpl::cachedProcessSlotInfo()
{
    return dynamic_cast<CachedProcessSlotInfo &>(m_cachedProcessSlotInfo);
}

NameList 
NodeImpl::getProcessSlotNames()
{
    TRACE(CL_LOG, "getProcessSlotNames");

    throwIfRemoved();

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createProcessSlotChildrenKey(getKey()),
        CachedObjectChangeHandlers::PROCESSSLOTS_CHANGE);
}

ProcessSlot *
NodeImpl::getProcessSlot(const string &processSlotName, 
                         AccessType accessType)
{
    TRACE(CL_LOG, "getProcessSlot");

    throwIfRemoved();

    return dynamic_cast<ProcessSlotImpl *>(
        getOps()->getNotifyable(this,
                                ClusterlibStrings::REGISTERED_PROCESSSLOT_NAME,
                                processSlotName,
                                accessType));
}

string
NodeImpl::createProcessSlotInfoJSONObjectKey(const string &nodeKey)
{
    string res;
    res.append(nodeKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOT_INFO_JSON_OBJECT);

    return res;
}

NodeImpl::~NodeImpl()
{
}

NotifyableList
NodeImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");
    
    throwIfRemoved();
    
    return getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_PROCESSSLOT_NAME,
        getProcessSlotNames(),
        LOAD_FROM_REPOSITORY);
}

void
NodeImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Ensure that the cache contains all the information
     * about this node, and that all watches are established.
     */
    m_cachedProcessSlotInfo.loadDataFromRepository(false);
}

};	/* End of 'namespace clusterlib' */
