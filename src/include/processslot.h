/*
 * processslot.h --
 *
 * Definition of class ProcessSlot; it represents a process slot on a
 * node of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_PROCESSSLOT_H_
#define _CL_PROCESSSLOT_H_

namespace clusterlib
{

/**
 * Definition of class ProcessSlot
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
     * Used to denote the process state: stopped (current and desired state)
     */
    static const std::string PROCESS_STATE_STOPPED_VALUE;

    /**
     * Used to denote the process state: failure (current state only)
     */
    static const std::string PROCESS_STATE_FAILURE_VALUE;

    /**
     * Used to accesss the failure of the current process state.  This
     * is used to give more detailed information about the failure if
     * there was one.
     */
    static const std::string PROCESS_STATE_FAILURE_MSG_KEY; 

    /**
     * Used to access the set time of the process state.
     */
    static const std::string PROCESS_STATE_SET_MSECS_KEY;

    /**
     * Used to access the set time as a date of the process state.
     */
    static const std::string PROCESS_STATE_SET_MSECS_AS_DATE_KEY;

    /**
     * Access the cached process info
     * 
     * @return A reference to the cached process info.
     */
    virtual CachedProcessInfo &cachedProcessInfo() = 0;

    /**
     * Destructor.
     */
    virtual ~ProcessSlot() {}
};


};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_PROCESSSLOT_H_ */
