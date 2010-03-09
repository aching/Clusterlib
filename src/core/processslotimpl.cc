/*
 * processslotimpl.cc --
 *
 * Implementation of the ProcessSlotImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

extern char **environ;

#include "clusterlibinternal.h"
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace json;

namespace clusterlib
{

JSONValue
ProcessSlotImpl::getJsonPortVec()
{
    TRACE(CL_LOG, "getJsonPortVec");

    string processSlotPortVecKey = 
        NotifyableKeyManipulator::createProcessSlotPortVecKey(getKey());

    string encodedJsonValue;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotPortVecKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_PORTVEC_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotPortVecKey,
            encodedJsonValue),
        CachedObjectChangeHandlers::PROCESSSLOT_PORTVEC_CHANGE,
        processSlotPortVecKey,
        "Reading the value of %s failed: %s",
        processSlotPortVecKey.c_str(),
        false,
        true);        
    vector<int32_t> portVec;
    if (encodedJsonValue.empty()) {
        return JSONValue();
    }
    return JSONCodec::decode(encodedJsonValue);
}
vector<int32_t>
ProcessSlotImpl::getPortVec()
{
    TRACE(CL_LOG, "getPortVec");

    JSONValue jsonValue = getJsonPortVec();
    JSONValue::JSONArray jsonArr = jsonValue.get<JSONValue::JSONArray>();
    vector<int32_t> portVec;
    portVec.reserve(jsonArr.size());
    for (size_t i = 0; i < jsonArr.size(); i++) {
        portVec.push_back(
            static_cast<int32_t>(jsonArr[i].get<JSONValue::JSONInteger>()));
    }

    return portVec;
}

void 
ProcessSlotImpl::setJsonPortVec(JSONValue jsonValue)
{
    TRACE(CL_LOG, "setJsonPortVec");

    string processSlotPortVecKey = 
        NotifyableKeyManipulator::createProcessSlotPortVecKey(getKey());

    string encodedJsonValue = JSONCodec::encode(jsonValue);
    acquireLock();
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotPortVecKey,
                     encodedJsonValue),
                 "Setting of %s failed: %s",
                 processSlotPortVecKey.c_str(),
                 true,
                 false);
    releaseLock();
}
void 
ProcessSlotImpl::setPortVec(vector<int32_t> portVec)
{
    TRACE(CL_LOG, "setPortVec");

    JSONValue::JSONArray jsonArr;
    portVec.reserve(jsonArr.size());
    for (size_t i = 0; i < portVec.size(); i++) {
        jsonArr.push_back(
            static_cast<int64_t>(portVec[i]));
    }

    setJsonPortVec(jsonArr);
}

void
ProcessSlotImpl::start()
{
    TRACE(CL_LOG, "start");

    string processSlotDesiredStateKey = 
        NotifyableKeyManipulator::createProcessSlotDesiredStateKey(getKey());

    JSONValue::JSONArray jsonArr;
    JSONValue::JSONString jsonValue = ClusterlibStrings::PROCESSSTATE_RUNNING;
    jsonArr.push_back(jsonValue);
    acquireLock();
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotDesiredStateKey,
                     JSONCodec::encode(jsonArr)),
                 "Setting of %s failed: %s",
                 processSlotDesiredStateKey.c_str(),
                 true,
                 false);
    releaseLock();
}

pid_t
ProcessSlotImpl::startLocal()
{
    TRACE(CL_LOG, "startLocal");

    vector<string> addEnv;
    string path;
    string cmd;

    acquireLock();
    getExecArgs(addEnv, path, cmd);
    LOG_DEBUG(CL_LOG, 
              "startLocal: with path=%s, cmd=%s", 
              path.c_str(), cmd.c_str());
    pid_t pid = ProcessThreadService::forkExec(addEnv, path, cmd);
    setRunningExecArgs(addEnv, path, cmd);
    releaseLock();
    if (pid == -1) {
        LOG_ERROR(CL_LOG, "start: Failed to run forkExec");
    }
    
    return pid;
}

JSONValue
ProcessSlotImpl::getJsonExecArgs()
{
    TRACE(CL_LOG, "getJsonExecArgs");

    string processSlotExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotExecArgsKey(getKey());

    string encodedJsonValue;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotExecArgsKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_EXECARGS_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotExecArgsKey,
            encodedJsonValue),
        CachedObjectChangeHandlers::PROCESSSLOT_EXECARGS_CHANGE,
        processSlotExecArgsKey,
        "Read the value of %s failed: %s",
        processSlotExecArgsKey.c_str(),
        false,
        true);    

    if (encodedJsonValue.empty()) {
        return JSONValue();
    }
    else {
        return JSONCodec::decode(encodedJsonValue);
    }
}
bool
ProcessSlotImpl::getExecArgs(vector<string> &addEnv,
                             string &path,
                             string &cmd)
{
    TRACE(CL_LOG, "getExecArgs");

    string processSlotExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotExecArgsKey(getKey());

    cmd.clear();
    path.clear();
    addEnv.clear();
    JSONValue jsonValue = getJsonExecArgs();
    if (jsonValue.type() == typeid(JSONValue::JSONNull)) {
        return false;
    }
    else {
        JSONValue::JSONObject jsonObj = jsonValue.get<JSONValue::JSONObject>();
        if (jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND].type() !=  
            typeid(JSONValue::JSONNull)) {
            cmd = jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND].
                get<JSONValue::JSONString>();
        }
        if (jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH].type() !=  
            typeid(JSONValue::JSONNull)) {
            path = jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH].
                get<JSONValue::JSONString>();
        }
        
        if (jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV].type() != 
            typeid(JSONValue::JSONNull)) {
            JSONValue::JSONArray jsonArrAddEnv = 
                jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV].
                get<JSONValue::JSONArray>();
            addEnv.reserve(jsonArrAddEnv.size());
            for (size_t i = 0; i < jsonArrAddEnv.size(); i++) {
                addEnv.push_back(
                    jsonArrAddEnv[i].get<JSONValue::JSONString>());
            }
        }

        return true;
    }
}

void 
ProcessSlotImpl::setJsonExecArgs(json::JSONValue jsonValue) 
{
    TRACE(CL_LOG, "setJsonExecArgs");
        
    string processSlotExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotExecArgsKey(getKey());

    /* Do a series of checks */
    if (jsonValue.type() != typeid(JSONValue::JSONObject)) {
        throw InvalidArgumentsException(
            "setJsonExecArgs: Should be an object");
    }

    acquireLock();
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotExecArgsKey,
                     JSONCodec::encode(jsonValue)),
                 "Setting of %s failed: %s",
                 processSlotExecArgsKey.c_str(),
                 true,
                 false);
    releaseLock();
}
void ProcessSlotImpl::setExecArgs(const vector<string> &addEnv,
                                  const string &path,
                                  const string &cmd)
{
    TRACE(CL_LOG, "setExecArgs");
        
    string processSlotExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotExecArgsKey(getKey());

    JSONValue::JSONObject jsonObj;
    JSONValue::JSONArray jsonArrAddEnv;
    for (size_t i = 0; i < addEnv.size(); i++) {
        jsonArrAddEnv.push_back(addEnv[i]);
    }
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV] = jsonArrAddEnv;
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH] = path;
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND] = cmd;

    setJsonExecArgs(jsonObj);
}

JSONValue
ProcessSlotImpl::getJsonRunningExecArgs()
{
    TRACE(CL_LOG, "getJsonRunningExecArgs");

    string processSlotRunningExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotRunningExecArgsKey(
            getKey());

    string encodedJsonValue;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotRunningExecArgsKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_RUNNING_EXECARGS_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotRunningExecArgsKey,
            encodedJsonValue),
        CachedObjectChangeHandlers::PROCESSSLOT_RUNNING_EXECARGS_CHANGE,
        processSlotRunningExecArgsKey,
        "Reading the value of %s failed: %s",
        processSlotRunningExecArgsKey.c_str(),
        false,
        true);    

    if (encodedJsonValue.empty()) {
        return JSONValue();
    }
    else {
        return JSONCodec::decode(encodedJsonValue);
    }
}
bool
ProcessSlotImpl::getRunningExecArgs(vector<string> &addEnv,
                                    string &path,
                                    string &cmd)
{
    TRACE(CL_LOG, "getRunningExecArgs");

    string processSlotRunningExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotRunningExecArgsKey(
            getKey());

    JSONValue jsonValue = getJsonRunningExecArgs();
    if (jsonValue.type() == typeid(JSONValue::JSONNull)) {
        return false;
    }
    else {
        JSONValue::JSONObject jsonObj = jsonValue.get<JSONValue::JSONObject>();
        if (jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND].type() !=  
            typeid(JSONValue::JSONNull)) {
            cmd = jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND].
                get<JSONValue::JSONString>();
        }
        if (jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH].type() !=  
            typeid(JSONValue::JSONNull)) {
            path = jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH].
                get<JSONValue::JSONString>();
        }
        
        if (jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV].type() != 
            typeid(JSONValue::JSONNull)) {
            JSONValue::JSONArray jsonArrAddEnv = 
                jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV].
                get<JSONValue::JSONArray>();
            addEnv.reserve(jsonArrAddEnv.size());
            for (size_t i = 0; i < jsonArrAddEnv.size(); i++) {
                addEnv.push_back(
                    jsonArrAddEnv[i].get<JSONValue::JSONString>());
            }
        }

        return true;
    }
}

JSONValue 
ProcessSlotImpl::getJsonPID()
{
    TRACE(CL_LOG, "getJsonPID");

    string processSlotPIDKey = 
        NotifyableKeyManipulator::createProcessSlotPIDKey(getKey());

    string encodedJsonValue;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotPIDKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_PID_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotPIDKey,
            encodedJsonValue),
        CachedObjectChangeHandlers::PROCESSSLOT_PID_CHANGE,
        processSlotPIDKey,
        "Reading thevalue of %s failed: %s",
        processSlotPIDKey.c_str(),
        false,
        true);
    if (encodedJsonValue.empty()) {
        return JSONValue();
    }
    else {        
        return JSONCodec::decode(encodedJsonValue);
    }
}
int32_t 
ProcessSlotImpl::getPID()
{
    TRACE(CL_LOG, "getPID");

    string processSlotPIDKey = 
        NotifyableKeyManipulator::createProcessSlotPIDKey(getKey());

    JSONValue jsonValue = getJsonPID();
    if (jsonValue.type() == typeid(JSONValue::JSONNull)) {
        return -1;
    }
    return static_cast<int32_t>(jsonValue.get<JSONValue::JSONInteger>());
}

void
ProcessSlotImpl::setPID(int32_t pid)
{
    TRACE(CL_LOG, "setPID");

    string processSlotPIDKey = 
        NotifyableKeyManipulator::createProcessSlotPIDKey(getKey());

    JSONValue::JSONInteger jsonInt = static_cast<int64_t>(pid);
    string encodedJsonValue = JSONCodec::encode(jsonInt);
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotPIDKey,
                     encodedJsonValue),
                 "Getting of %s failed: %s",
                 processSlotPIDKey.c_str(),
                 true,
                 false);
}

void
ProcessSlotImpl::stop(int32_t sig)
{
    TRACE(CL_LOG, "stop");

    string processSlotDesiredStateKey = 
        NotifyableKeyManipulator::createProcessSlotDesiredStateKey(getKey());
    
    JSONValue::JSONArray jsonArr;
    jsonArr.push_back(ClusterlibStrings::PROCESSSTATE_STOPPED);
    jsonArr.push_back(static_cast<int64_t>(sig));

    string encodedJsonValue = JSONCodec::encode(jsonArr);
    acquireLock();
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotDesiredStateKey,
                     encodedJsonValue),
                 "Setting of %s failed: %s",
                 processSlotDesiredStateKey.c_str(),
                 true,
                 false);
    releaseLock();
}

void 
ProcessSlotImpl::stopLocal()
{
    TRACE(CL_LOG, "stop");

    int32_t pid = getPID();
    if (pid > 0) {
       int32_t ret = kill(pid, SIGKILL);
       if (ret != 0) {
           stringstream ss; 
           ss << "stop: kill of pid " << pid << " failed.";
           throw SystemFailureException(ss.str());
       }
    }
    else {
        throw InvalidMethodException("stop: pid <= 0");
    }
}

void 
ProcessSlotImpl::setRunningJsonExecArgs(json::JSONValue jsonValue) 
{
    TRACE(CL_LOG, "setRunningJsonExecArgs");
        
    string processSlotRunningExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotRunningExecArgsKey(
            getKey());

    /* Do a series of checks */
    if (jsonValue.type() != typeid(JSONValue::JSONObject)) {
        throw InvalidArgumentsException(
            "setRunningJsonExecArgs: Should be an object");
    }

    acquireLock();
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotRunningExecArgsKey,
                     JSONCodec::encode(jsonValue)),
                 "Setting of %s failed: %s",
                 processSlotRunningExecArgsKey.c_str(),
                 true,
                 false);
    releaseLock();
}
void ProcessSlotImpl::setRunningExecArgs(const vector<string> &addEnv,
                                         const string &path,
                                         const string &cmd)
{
    TRACE(CL_LOG, "setRunningExecArgs");
        
    string processSlotRunningExecArgsKey = 
        NotifyableKeyManipulator::createProcessSlotRunningExecArgsKey(
            getKey());

    JSONValue::JSONObject jsonObj;
    JSONValue::JSONArray jsonArrAddEnv;
    for (size_t i = 0; i < addEnv.size(); i++) {
        jsonArrAddEnv.push_back(addEnv[i]);
    }
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV] = jsonArrAddEnv;
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH] = path;
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND] = cmd;

    setRunningJsonExecArgs(jsonObj);
}

string
ProcessSlotImpl::createDefaultExecArgs()
{
    TRACE(CL_LOG, "createDefaultExecArgs");

    JSONValue::JSONObject jsonObj;
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_ADDENV] = JSONValue();
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_PATH] = JSONValue();
    jsonObj[ClusterlibStrings::JSONOBJECTKEY_COMMAND] = JSONValue();

    return JSONCodec::encode(jsonObj);
}

ProcessSlot::ProcessState 
ProcessSlotImpl::getDesiredProcessState()
{
    TRACE(CL_LOG, "getDesiredProcessState");

    string processSlotDesiredStateKey = 
        NotifyableKeyManipulator::createProcessSlotDesiredStateKey(getKey());

    string encodedJsonValue;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotDesiredStateKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_DESIRED_STATE_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotDesiredStateKey,
            encodedJsonValue),
        CachedObjectChangeHandlers::PROCESSSLOT_DESIRED_STATE_CHANGE,
        processSlotDesiredStateKey,
        "Reading the value of %s failed: %s",
        processSlotDesiredStateKey.c_str(),
        false,
        true);
    if (encodedJsonValue.empty()) {
        return ProcessSlot::UNUSED;
    }
    else {        
        JSONValue::JSONArray jsonArray = 
            JSONCodec::decode(encodedJsonValue).get<JSONValue::JSONArray>();
        JSONValue::JSONString jsonString = 
            jsonArray[0].get<JSONValue::JSONString>();
        return ProcessSlot::getProcessStateFromString(jsonString);
    }
}

ProcessSlot::ProcessState 
ProcessSlotImpl::getCurrentProcessState()
{
    TRACE(CL_LOG, "getCurrentProcessState");

    string processSlotCurrentStateKey = 
        NotifyableKeyManipulator::createProcessSlotCurrentStateKey(getKey());

    string encodedJsonValue;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotCurrentStateKey,
            encodedJsonValue,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_CURRENT_STATE_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotCurrentStateKey,
            encodedJsonValue),
        CachedObjectChangeHandlers::PROCESSSLOT_CURRENT_STATE_CHANGE,
        processSlotCurrentStateKey,
        "Reading the value of %s failed: %s",
        processSlotCurrentStateKey.c_str(),
        false,
        true);
    if (encodedJsonValue.empty()) {
        return ProcessSlot::UNUSED;
    }
    else {        
        string processStateString = 
            JSONCodec::decode(encodedJsonValue).get<JSONValue::JSONString>();
        return ProcessSlot::getProcessStateFromString(processStateString);
    }
}

void
ProcessSlotImpl::setCurrentProcessState(ProcessState processState)
{
    TRACE(CL_LOG, "setCurrentProcessState");

    string processSlotCurrentStateKey = 
        NotifyableKeyManipulator::createProcessSlotCurrentStateKey(getKey());

    JSONValue::JSONString jsonString = 
        ProcessSlot::getProcessStateAsString(processState);
    string encodedJsonValue = JSONCodec::encode(jsonString);
    SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                     processSlotCurrentStateKey,
                     encodedJsonValue),
                 "Getting of %s failed: %s",
                 processSlotCurrentStateKey.c_str(),
                 true,
                 false);
}

string
ProcessSlotImpl::getReservationName()
{
    TRACE(CL_LOG, "getReservationName");

    string processSlotReservationKey = 
        NotifyableKeyManipulator::createProcessSlotReservationKey(getKey());

    string currentReservationName;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotReservationKey,
            currentReservationName,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_RESERVATION_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotReservationKey,
            currentReservationName),
        CachedObjectChangeHandlers::PROCESSSLOT_RESERVATION_CHANGE,
        processSlotReservationKey,
        "Reading the value of %s failed: %s",
        processSlotReservationKey.c_str(),
        false,
        true);
    return currentReservationName;
}


bool 
ProcessSlotImpl::setReservationIfEmpty(string reservationName)
{
    TRACE(CL_LOG, "setReservationIfEmpty");

    string processSlotReservationKey = 
        NotifyableKeyManipulator::createProcessSlotReservationKey(getKey());

    string currentReservationName;
    bool ret = false;
    acquireLock();
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotReservationKey,
            currentReservationName,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_RESERVATION_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotReservationKey,
            currentReservationName),
        CachedObjectChangeHandlers::PROCESSSLOT_RESERVATION_CHANGE,
        processSlotReservationKey,
        "Reading the value of %s failed: %s",
        processSlotReservationKey.c_str(),
        false,
        true);
    if (currentReservationName.empty()) {
        SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                         processSlotReservationKey,
                         reservationName),
                     "Setting of %s failed: %s",
                     processSlotReservationKey.c_str(),
                     true,
                     false);
        ret = true;
    }
    releaseLock();

    return ret;
}

bool 
ProcessSlotImpl::releaseReservationIfMatches(string reservationName)
{
    TRACE(CL_LOG, "releaseReservationIfMatches");

    string processSlotReservationKey = 
        NotifyableKeyManipulator::createProcessSlotReservationKey(getKey());

    string currentReservationName;
    bool ret = false;
    acquireLock();
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeData(
            processSlotReservationKey,
            currentReservationName,
            getOps()->getZooKeeperEventAdapter(),
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::PROCESSSLOT_RESERVATION_CHANGE)),
        getOps()->getRepository()->getNodeData(
            processSlotReservationKey,
            currentReservationName),
        CachedObjectChangeHandlers::PROCESSSLOT_RESERVATION_CHANGE,
        processSlotReservationKey,
        "Reading the value of %s failed: %s",
        processSlotReservationKey.c_str(),
        false,
        true);
    if (currentReservationName == reservationName) {
        string empty;
        SAFE_CALL_ZK(getOps()->getRepository()->setNodeData(
                         processSlotReservationKey,
                         empty),
                     "Setting of %s failed: %s",
                     processSlotReservationKey.c_str(),
                     true,
                     false);
        ret = true;
    }
    releaseLock();

    return ret;
}
void
ProcessSlotImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Ensure that the cache contains all the information about this
     * object, and that all watches are established.
     *
     * This object's members operate completely in zookeeeper, so no
     * initialization is required.
     */
}

void
ProcessSlotImpl::removeRepositoryEntries()
{
    getOps()->removeProcessSlot(this);
}

ProcessSlotImpl::~ProcessSlotImpl()
{
}

};	/* End of 'namespace clusterlib' */
