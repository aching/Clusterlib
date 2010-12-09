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

#include "clusterlibinternal.h"
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

using namespace std;
using namespace boost;
using namespace json;

namespace clusterlib {

const string ProcessSlot::DESIRED_PROCESS_STATE_SET_MSECS_KEY = 
    "_desiredProcessStateSetMsecs";

const string ProcessSlot::EXEC_ENV_KEY = "_executableEnvironment";

const string ProcessSlot::EXEC_PATH_KEY = "_executablePath";

const string ProcessSlot::EXEC_COMMAND_KEY = "_executableCommand";

const string ProcessSlot::PROCESS_STATE_KEY = "_processState";

const string ProcessSlot::PROCESS_STATE_RUNNING_VALUE = "_processStateRunning";
const string ProcessSlot::PROCESS_STATE_RUN_ONCE_VALUE = 
    "_processStateRunOnce";
const string ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE = 
    "_processStateRunContinuously";
const string ProcessSlot::PROCESS_STATE_EXIT_VALUE = "_processStateExit";
const string ProcessSlot::PROCESS_STATE_CLEANEXIT_VALUE = 
    "_processStateCleanexit";
const string ProcessSlot::PROCESS_STATE_FAILURE_VALUE = "_processStateFailure";

const string ProcessSlot::PROCESS_STATE_TOTAL_STARTS_KEY = 
    "_processStateTotalStarts";

const string ProcessSlot::PROCESS_STATE_MAX_STARTS_KEY = 
    "_processStateMaxStarts";

const string ProcessSlot::PROCESS_STATE_MSG_KEY = 
    "_processStateMsg";

const string ProcessSlot::PROCESS_STATE_SET_MSECS_KEY = 
    "_processStateSetMsecs";

const string ProcessSlot::PROCESS_STATE_SET_MSECS_AS_DATE_KEY = 
    "_processStateSetMsecsAsDate";

const string ProcessSlot::BINARY_STATE_KEY = "_binaryState";

const string ProcessSlot::BINARY_STATE_NONE_VALUE = 
    "_binaryStateNone";
const string ProcessSlot::BINARY_STATE_PREPARING_VALUE = 
    "_binaryStatePreparing";
const string ProcessSlot::BINARY_STATE_READY_VALUE = 
    "_binaryStateReady";
const string ProcessSlot::BINARY_STATE_BUSY_VALUE = 
    "_binaryStateBusy";
const string ProcessSlot::BINARY_STATE_HALTING_VALUE = 
    "_binaryStateHalting";

bool
ProcessSlot::isCurrentStateFromDesiredState(
    const shared_ptr<ProcessSlot> &processSlotSP)
{
    TRACE(CL_LOG, "isCurrentStateFromDesiredState");

    JSONValue desiredStateSetValue;
    bool foundDesiredStateSetMsecs = processSlotSP->cachedCurrentState().get(
        ProcessSlot::DESIRED_PROCESS_STATE_SET_MSECS_KEY, 
        desiredStateSetValue);
    if (foundDesiredStateSetMsecs == false) {
        return false;
    }

    JSONValue processStateSetValue;
    bool foundProcessStateSetMsecs = processSlotSP->cachedDesiredState().get(
        ProcessSlot::PROCESS_STATE_SET_MSECS_KEY,
        processStateSetValue);
    if (foundProcessStateSetMsecs == false) {
        return false;
    }
    
    if (desiredStateSetValue.get<JSONValue::JSONInteger>() ==
        processStateSetValue.get<JSONValue::JSONInteger>()) {
        return true;
    }

    return false;
}

CachedProcessInfo &
ProcessSlotImpl::cachedProcessInfo()
{
    return m_cachedProcessInfo;
}

string
ProcessSlotImpl::createProcessInfoJsonArrKey(const string &processSlotKey)
{
    string res;
    res.append(processSlotKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::PROCESSINFO_JSON_OBJECT);

    return res;
}

void
ProcessSlotImpl::getExecArgs(CachedState &cachedState,
                             vector<string> &addEnv,
                             string &path,
                             string &command)
{
    TRACE(CL_LOG, "getExecArgs");
    
    bool found;
    JSONValue jsonValue;
    found = cachedState.get(ProcessSlot::EXEC_ENV_KEY, jsonValue);
    if (found == true) {
        addEnv.clear();
        JSONValue::JSONArray jsonArr = jsonValue.get<JSONValue::JSONArray>();
        JSONValue::JSONArray::const_iterator jsonArrIt;
        for (jsonArrIt = jsonArr.begin(); 
             jsonArrIt != jsonArr.end(); 
             ++jsonArrIt) {
            addEnv.push_back(jsonArrIt->get<JSONValue::JSONString>());
        }
    }

    found = cachedState.get(ProcessSlot::EXEC_PATH_KEY, jsonValue);
    if (found == true) {
        path = jsonValue.get<JSONValue::JSONString>();
    }
    
    found = cachedState.get(ProcessSlot::EXEC_COMMAND_KEY, jsonValue);
    if (found == false) {
        throw RepositoryDataMissingException(
            "getExecArgs: No EXEC_COMMAND_KEY");
    }
    command = jsonValue.get<JSONValue::JSONString>();
}

pid_t
ProcessSlotImpl::startLocal(const vector<string> &addEnv, 
                            const string &path, 
                            const string &command)
{
    TRACE(CL_LOG, "startLocal");

    LOG_DEBUG(CL_LOG, 
              "startLocal: with path=%s, command=%s", 
              path.c_str(), command.c_str());
    pid_t pid = ProcessThreadService::forkExec(addEnv, path, command);
    if (pid == -1) {
        LOG_ERROR(CL_LOG, "startLocal: Failed to run forkExec");
    }
    
    return pid;
}

void 
ProcessSlotImpl::stopLocal(pid_t pid, int32_t signal)
{
    TRACE(CL_LOG, "stopLocal");

    if (pid > 0) {
       int32_t ret = kill(pid, signal);
       if (ret != 0) {
           if (errno == ESRCH) {
               LOG_WARN(CL_LOG, "stopLocal: kill failed with ESRCH");
           }
           else {
               ostringstream oss; 
               oss << "stopLocal: kill of pid " << pid << " failed with error"
                   << strerror(errno);
               throw SystemFailureException(oss.str());
           }
       }
    }
    else {
        throw InvalidMethodException("stopLocal: pid <= 0");
    }
}

NotifyableList
ProcessSlotImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");

    throwIfRemoved();
    
    return NotifyableList();
}
 
void
ProcessSlotImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Ensure that the cache contains all the information about this
     * object, and that all watches are established.
     */
    m_cachedProcessInfo.loadDataFromRepository(false);
}

ProcessSlotImpl::~ProcessSlotImpl()
{
}

}	/* End of 'namespace clusterlib' */
