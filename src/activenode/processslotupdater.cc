/*
 * processslotupdater.cc --
 *
 * Implementation of the ProcessSlotUpdater object.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include <sys/utsname.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include "clusterlibinternal.h"
#include "processslotupdater.h"

using namespace std;
using namespace clusterlib;
using namespace json;

namespace activenode {

ProcessSlotUpdater::ProcessSlotUpdater(int64_t msecsFrequency,
                                       Notifyable *notifyable)
    : Periodic(msecsFrequency, notifyable)
{
}

ProcessSlotUpdater::~ProcessSlotUpdater()
{
}

ProcessSlotUpdater::UpdateAction
ProcessSlotUpdater::determineAction(Notifyable &notifyable)
{
    TRACE(CL_LOG, "determineAction");

    JSONValue jsonCurrentState, jsonDesiredState;
    bool currentFound = false, desiredFound = false;

    currentFound = notifyable.cachedCurrentState().get(
        ProcessSlot::PROCESS_STATE_KEY, jsonCurrentState);    
    desiredFound = notifyable.cachedDesiredState().get(
        ProcessSlot::PROCESS_STATE_KEY, jsonDesiredState);
    
    /*
     * Algorithm:
     *
     * 1) If there is no desired state, done.
     * 2) Set the action to the desired state appropriately.
     * 3) If the current state does not match the desired state or is working
     *    off of a different desired state, do that 
     *    desired state action.
     */
    if (desiredFound == false) {
        return ProcessSlotUpdater::NONE;
    }
    else {
        UpdateAction tmpAction;
        string desiredState = jsonDesiredState.get<JSONValue::JSONString>();
        if ((desiredState == ProcessSlot::PROCESS_STATE_RUN_ONCE_VALUE) ||
            (desiredState == 
             ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE)) {
            tmpAction = ProcessSlotUpdater::START;
        }
        else if (desiredState == ProcessSlot::PROCESS_STATE_EXIT_VALUE) {
            tmpAction =  ProcessSlotUpdater::KILL;
        }
        else if (desiredState == ProcessSlot::PROCESS_STATE_CLEANEXIT_VALUE) {
            tmpAction =  ProcessSlotUpdater::NONE;
        }
        else {
            throw InconsistentInternalStateException(
                string("determineAction: desiredState is unknown - ") + 
                desiredState);
        }

        if (currentFound == true) {
            bool currentDesiredProcessStateSetMsecsFound = false;
            bool desiredProcessStateSetMsecsFound = false;
            JSONValue currentDesiredProcessStateSetMsecs;
            JSONValue desiredProcessStateSetMsecs;

            currentDesiredProcessStateSetMsecsFound = 
                notifyable.cachedCurrentState().get(
                    ProcessSlot::DESIRED_PROCESS_STATE_SET_MSECS_KEY, 
                    currentDesiredProcessStateSetMsecs);    
            desiredProcessStateSetMsecsFound = 
                notifyable.cachedDesiredState().get(
                    ProcessSlot::PROCESS_STATE_SET_MSECS_KEY, 
                    desiredProcessStateSetMsecs);
            if (!desiredProcessStateSetMsecsFound) {
                throw InconsistentInternalStateException(
                    "determineAction: Couldn't find process state set time!");
            }
            else if (currentDesiredProcessStateSetMsecsFound) {
                /* Current state operating off the wrong desired state? */
                if (currentDesiredProcessStateSetMsecs.
                    get<JSONValue::JSONInteger>()
                    != desiredProcessStateSetMsecs.
                    get<JSONValue::JSONInteger>()) {
                    return tmpAction;
                }
            }

            string currentState = 
                jsonCurrentState.get<JSONValue::JSONString>();
            if ((tmpAction == ProcessSlotUpdater::START) &&
                (currentState == ProcessSlot::PROCESS_STATE_RUNNING_VALUE)) {
                return ProcessSlotUpdater::NONE;
            }
            else if ((tmpAction == ProcessSlotUpdater::START) &&
                     (desiredState == 
                      ProcessSlot::PROCESS_STATE_RUN_ONCE_VALUE) &&
                     (currentState == 
                      ProcessSlot::PROCESS_STATE_EXIT_VALUE)) {
                return ProcessSlotUpdater::NONE;
            }
            else if ((tmpAction == ProcessSlotUpdater::KILL) &&
                     (currentState == 
                      ProcessSlot::PROCESS_STATE_EXIT_VALUE)) {
                return ProcessSlotUpdater::NONE;
            }
            else {
                return tmpAction;
            }
        }
        else {
            return tmpAction;
        }
    }
}

void
ProcessSlotUpdater::run()
{
    TRACE(CL_LOG, "run");

    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(
        getNotifyable());
    if (processSlot == NULL) {
        throw InconsistentInternalStateException(
            "run: processSlot is NULL");
    }

    processSlot->acquireLock();

    LOG_DEBUG(CL_LOG,
              "run: current state = %s, desired state = %s",
              JSONCodec::encode(processSlot->cachedCurrentState().
                                getHistoryArray()).c_str(),
              JSONCodec::encode(processSlot->cachedDesiredState().
                                getHistoryArray()).c_str());

    /*
     * Make sure if there was a running process that we see if the
     * status changed.
     */
    ostringstream msgss;
    JSONValue jsonPid;
    bool foundPid = processSlot->cachedCurrentState().get(
        ProcessSlot::PID_KEY, jsonPid);
    pid_t oldPid = -1;
    pid_t waitedPid = -1;
    if (foundPid) {
        int stat_loc;
        oldPid = jsonPid.get<JSONValue::JSONInteger>();
        if (oldPid < -1) {
            ostringstream oss;
            oss << "run: Bad oldPid=" << oldPid;
            throw InconsistentInternalStateException(oss.str());
        }
        else if (oldPid == -1) {
            /* Nothing to do here. */
        }
        else {
            pid_t waitedPid = waitpid(oldPid, &stat_loc, WNOHANG);
            if (waitedPid == -1) {
                ostringstream oss;
                oss << "run: Failed with error " << errno << " and error=" 
                    << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            else if (waitedPid == 0) {
                LOG_DEBUG(CL_LOG, 
                          "run: No update on process %" PRId32, 
                          static_cast<int32_t>(oldPid));
            }
            else if (waitedPid != oldPid) {
                ostringstream oss;
                oss << "run: Returned waitedPid=" << waitedPid 
                    << " from oldPid=" << oldPid;
                throw SystemFailureException(oss.str());
            }
            else {
                processSlot->cachedCurrentState().set(
                    ProcessSlot::PID_KEY, JSONValue::JSONInteger(-1));
                processSlot->cachedCurrentState().set(
                    ProcessSlot::PROCESS_STATE_KEY, 
                    ProcessSlot::PROCESS_STATE_EXIT_VALUE);
            }
        }
    }

    UpdateAction action = determineAction(*processSlot);

    switch(action) {
        case NONE:
            break;
        case START:
            {
                /* Might have to stop first if already running */
                if ((oldPid > 0) && (oldPid != waitedPid)) {
                    processSlot->stopLocal(oldPid, SIGKILL);
                    msgss << "Start Action: Stopped old PID " 
                          << oldPid << endl;
                }
                
                vector<string> addEnv;
                string path;
                string command;
                try {
                    processSlot->getExecArgs(processSlot->cachedDesiredState(),
                                             addEnv,
                                             path, 
                                             command);
                    pid_t newPid = processSlot->startLocal(
                        addEnv, path, command);
                    processSlot->cachedCurrentState().set(
                        ProcessSlot::PID_KEY, 
                        JSONValue::JSONInteger(newPid));
                    processSlot->cachedCurrentState().set(
                        ProcessSlot::PROCESS_STATE_KEY, 
                        ProcessSlot::PROCESS_STATE_RUNNING_VALUE);
                    msgss << "StartAction: Started new PID " << newPid << endl;
                }
                catch (const RepositoryDataMissingException &e) { 
                    msgss.str("");
                    msgss << e.what();
                    processSlot->cachedCurrentState().set(
                        ProcessSlot::PROCESS_STATE_KEY, 
                        ProcessSlot::PROCESS_STATE_FAILURE_VALUE);
                }
            }
            break;
        case KILL:
            if ((oldPid > 0) && (oldPid != waitedPid)) {
                processSlot->stopLocal(oldPid, SIGKILL);
                msgss << "Kill Action: Stopped old PID " << oldPid << endl;
            }
            processSlot->cachedCurrentState().set(
                ProcessSlot::PROCESS_STATE_KEY, 
                ProcessSlot::PROCESS_STATE_EXIT_VALUE);
            break;
        default:
            throw InconsistentInternalStateException("run: Invalid action");
    };

    processSlot->cachedCurrentState().set(
        ProcessSlot::PROCESS_STATE_MSG_KEY, 
        msgss.str());
    int64_t msecs = TimerService::getCurrentTimeMsecs();
    processSlot->cachedCurrentState().set(
        ProcessSlot::PROCESS_STATE_SET_MSECS_KEY, msecs);
    processSlot->cachedCurrentState().set(
        ProcessSlot::PROCESS_STATE_SET_MSECS_AS_DATE_KEY, 
        TimerService::getMsecsTimeString(msecs));
    JSONValue jsonDesiredProcessStateSetMsecs;
    bool found = processSlot->cachedDesiredState().get(
        ProcessSlot::PROCESS_STATE_SET_MSECS_KEY, 
        jsonDesiredProcessStateSetMsecs);
    if (found) {
        processSlot->cachedCurrentState().set(
            ProcessSlot::DESIRED_PROCESS_STATE_SET_MSECS_KEY, 
            jsonDesiredProcessStateSetMsecs);
    }
    processSlot->cachedCurrentState().publish();

    processSlot->releaseLock();
} 

}
