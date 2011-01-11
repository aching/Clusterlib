/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_PROCESSSLOT_H_
#define _CL_PROCESSSLOT_H_

namespace clusterlib {

/**
 * Can manage a single process of a Clusterlib Node.
 */
class ProcessSlot
    : public virtual Notifyable
{
  public:
    /**
     * Used to access the set time of the desired state that this
     * current state is responding to.
     */
    static const std::string DESIRED_PROCESS_STATE_SET_MSECS_KEY;

    /**
     * Used to access the path that the executable will use in a
     * desired or current state.  Value should be a JSONArray of JSONStrings.
     */
    static const std::string EXEC_ENV_KEY;

    /**
     * Used to access the path that the executable will use in a
     * desired or current state.  Value should be a JSONString.
     */
    static const std::string EXEC_PATH_KEY;

    /**
     * Used to access the command that the executable will use in a
     * desired or current state.  Value should be a JSONString.
     */
    static const std::string EXEC_COMMAND_KEY;

    /**
     * Used to access the process state.
     */
    static const std::string PROCESS_STATE_KEY;

    /**
     * Used to denote the process state: running (current state only)
     */
    static const std::string PROCESS_STATE_RUNNING_VALUE;

    /**
     * Used to denote the process state: run once (desired state only)
     */
    static const std::string PROCESS_STATE_RUN_ONCE_VALUE;

    /**
     * Used to denote the process state: run continuously (desired state only)
     */
    static const std::string PROCESS_STATE_RUN_CONTINUOUSLY_VALUE;

    /**
     * Used to denote the process state: exit (current and desired
     * state).  This will cause an ActiveNode to kill the underlying
     * process in the desired state).
     */
    static const std::string PROCESS_STATE_EXIT_VALUE;

    /**
     * Used to denote the process state: clean exist (desired state).
     * This is a user-defined exit.  An ActiveNode will simply wait
     * for the process to exit and then change the PROCESS_STATE_KEY
     * to PROCESS_STATE_EXIT_VALUE in the current state.  The user
     * must enact the appropriate method to have this process exit.
     */
    static const std::string PROCESS_STATE_CLEANEXIT_VALUE;

    /**
     * Used to denote the process state: failure (current state only)
     */
    static const std::string PROCESS_STATE_FAILURE_VALUE;

    /**
     * Used to accesss more information about the current process
     * state.  It describes PROCESS_STATE_KEY in more detail.
     */
    static const std::string PROCESS_STATE_MSG_KEY; 

    /**
     * Used to access the set time of the process state.
     */
    static const std::string PROCESS_STATE_SET_MSECS_KEY;

    /**
     * Used to access the set time as a date of the process state.
     */
    static const std::string PROCESS_STATE_SET_MSECS_AS_DATE_KEY;

    /**
     * The number of times the process has been started with the
     * most recent desired process state.
     */
    static const std::string PROCESS_STATE_TOTAL_STARTS_KEY;

    /**
     * The maximum number of times that a process may be started if in
     * PROCESS_STATE_RUN_CONTINUOUSLY_VALUE mode (-1 indicates
     * forever).
     */
    static const std::string PROCESS_STATE_MAX_STARTS_KEY;

    /**
     * Used to denote the running process state (current state only)
     */
    static const std::string BINARY_STATE_KEY;

    /**
     * Used to denote the running process state: none (current state only)
     */
    static const std::string BINARY_STATE_NONE_VALUE;

    /**
     * Used to denote the running process state: preparing (current state only)
     */
    static const std::string BINARY_STATE_PREPARING_VALUE;

    /**
     * Used to denote the running process state: ready (current state only)
     */
    static const std::string BINARY_STATE_READY_VALUE;

    /**
     * Used to denote the running process state: busy (current state only)
     */
    static const std::string BINARY_STATE_BUSY_VALUE;

    /**
     * Used to denote the running process state: halting (current state only)
     */
    static const std::string BINARY_STATE_HALTING_VALUE;

    /**
     * Access the cached process info
     * 
     * @return A reference to the cached process info.
     */
    virtual CachedProcessInfo &cachedProcessInfo() = 0;

    /**
     * Checks whether the current state of a ProcessSlot resulted from
     * the desired state without holding the distributed lock.  The
     * user may do this if desired.
     *
     * @param processSlotSP ProcessSlot that will be checked
     * @return True if the desired state results from the current state, 
     *         otherwise false
     */
    static bool isCurrentStateFromDesiredState(
        const boost::shared_ptr<ProcessSlot> &processSlotSP);

    /**
     * Destructor.
     */
    virtual ~ProcessSlot() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_PROCESSSLOT_H_ */
