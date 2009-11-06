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

#ifndef	_PROCESSSLOT_H_
#define _PROCESSSLOT_H_

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
     * State of the ProcessSlot.
     */
    enum ProcessState {
        UNUSED,   /**< Not used */
        STARTED,  /**< Active node reported the process was started */
        RUNNING,  /**< User wants the process to run in desired state */
        STOPPED,  /**< User wants the process to be halted */
        FINISHED, /**< Active node reported the process is done */
        FAILED,   /**< Active node failed to start up the process */
        INVALID   /**< Active node reports unknown problem */
    };

    /**
     * Get the process state as a string
     *
     * @param processState the state to be converted
     * @return processState as a string
     */
    static std::string getProcessStateAsString(ProcessState processState)
    {
        switch (processState) {
            case UNUSED:
                return ClusterlibStrings::PROCESSSTATE_UNUSED;
            case STARTED:
                return ClusterlibStrings::PROCESSSTATE_STARTED;
            case RUNNING:
                return ClusterlibStrings::PROCESSSTATE_RUNNING;
            case STOPPED:
                return ClusterlibStrings::PROCESSSTATE_STOPPED;
            case FINISHED:
                return ClusterlibStrings::PROCESSSTATE_FINISHED;
            case FAILED:
                return ClusterlibStrings::PROCESSSTATE_FAILED;
            case INVALID:
                return ClusterlibStrings::PROCESSSTATE_INVALID;
            default:
                throw InvalidArgumentsException(
                    "getProcessStateAsString: Invalid process state ");

        }
    }

    /** 
     * Get the process state from a string 
     * 
     * @param processStateString the process state as a string
     * @return the process state 
     */
    static ProcessState getProcessStateFromString(
        const std::string &processStateString) {
        if (processStateString.compare(
                ClusterlibStrings::PROCESSSTATE_UNUSED) == 0) {
            return UNUSED;
        }
        else if (processStateString.compare(
                     ClusterlibStrings::PROCESSSTATE_STARTED) == 0) {
            return STARTED;
        }
        else if (processStateString.compare(
                     ClusterlibStrings::PROCESSSTATE_RUNNING) == 0) {
            return RUNNING;
        }
        else if (processStateString.compare(
                     ClusterlibStrings::PROCESSSTATE_STOPPED) == 0) {
            return STOPPED;
        }
        else if (processStateString.compare(
                     ClusterlibStrings::PROCESSSTATE_FINISHED) == 0) {
            return FINISHED;
        }
        else if (processStateString.compare(
                     ClusterlibStrings::PROCESSSTATE_FAILED) == 0) {
            return FAILED;
        }
        else if (processStateString.compare(
                     ClusterlibStrings::PROCESSSTATE_INVALID) == 0) {
            return INVALID;
        }
        throw InvalidArgumentsException(
            "getProcessStateFromString: Invalid process state string");
    }

    /**
     * Get a vector of ports (user-defined).
     *
     * @return the vector of ports
     */
    virtual std::vector<int32_t> getPortVec() = 0;

    /**
     * Get a vector of ports (user-defined) as a JSONValue.
     *
     * @return the vector of ports as a JSONValue.
     */
    virtual json::JSONValue getJsonPortVec() = 0;

    /**
     * Set a vector of ports (user-defined) as a JSONValue.
     *
     * @param portVec the vector of ports to set as a JSONValue
     */
    virtual void setPortVec(std::vector<int32_t> portVec) = 0;

    /**
     * Set a vector of ports (user-defined).
     *
     * @param portVec the vector of ports to set
     */
    virtual void setJsonPortVec(json::JSONValue jsonValue) = 0;

    /**
     * Fork and execute with the current exec arguments.  The PID is
     * saved after the process has started.  To ensure that the same
     * exec arguments are used, hold the distributed lock.
     */
    virtual void start() = 0;
 
    /**
     * Get the current exec arguments.  The addEnv, path, and cmd are
     * only set sucessfully if the function returns true.
     *
     * @param addEnv current environment
     * @param path the path to execute
     * @param cmd the command to execute
     * @return true if sucessfull
     */
    virtual bool getExecArgs(std::vector<std::string> &addEnv,
                             std::string &path,
                             std::string &cmd) = 0;
 
    /**
     * Get the current exec arguments as a JSONValue.
     *
     * @return JSONNulll on failure, the correct JSONValue is successful
     */
   virtual json::JSONValue getJsonExecArgs() = 0;
    
    /**
     * Set the current exec arguments.
     *
     * @param addEnv current environment
     * @param path the path to execute
     * @param cmd the command to execute
     */
    virtual void setExecArgs(const std::vector<std::string> &addEnv,
                             const std::string &path,
                             const std::string &cmd) = 0;

    /**
     * Set the current exec arguments as a JSONValue.
     *
     * @param jsonValue the addEnv, path, and cmd as a JSONValue
     */
    virtual void setJsonExecArgs(json::JSONValue jsonValue) = 0;
    
    /**
     * Get the running exec arguments.  The addEnv, path, and cmd are
     * only set sucessfully if the function returns true.
     *
     * @param addEnv current environment
     * @param path the path to execute
     * @param cmd the command to execute
     * @return true if sucessfull
     */
    virtual bool getRunningExecArgs(std::vector<std::string> &addEnv,
                                    std::string &path,
                                    std::string &cmd) = 0;

    /**
     * Get the running exec arguments as a JSONValue.
     *
     * @return JSONNulll on failure, the correct JSONValue is successful
     */
    virtual json::JSONValue getJsonRunningExecArgs() = 0;

    /** 
     * Get the PID of the process if it has started.  This is
     * informational.
     *
     * @return the pid or -1 if there is no valid PID.
     */
    virtual int32_t getPID() = 0;

    /** 
     * Get the PID of the process if it has started as a JSONValue.
     * This is informational.
     *
     * @return the JSONValue or JSONNulll if there is no valid PID
     */
    virtual json::JSONValue getJsonPID() = 0;

    /** 
     * Stop the command line process (by issuing a kill command with
     * SIGTERM - 15).
     */
    virtual void stop(int32_t sig = 15) = 0;

    /**
     * Get the user desired state of the ProcessSlot.
     *
     * @return the desired process state
     */
    virtual ProcessState getDesiredProcessState() = 0;

    /**
     * Get the current state of the ProcessSlot.
     * @return the current process state
     */
    virtual ProcessState getCurrentProcessState() = 0;

    /**
     * Get the reservation slot name (informational).
     */
    virtual std::string getReservationName() = 0;

    /**
     * Reserve the process slot if empty.  Running this does a
     * test-and-set with this notifyable's distributed lock.
     * 
     * @param reservationName the name to reserve under
     * @return true if successful, false if already reserved.
     */
    virtual bool setReservationIfEmpty(std::string reservationName) = 0;

    /**
     * Releases the process slot if the reservation name matches.
     * Running this does a test-and-set with this notifyable's
     * distributed lock.
     * 
     * @param reservationName the name that was reserved under
     * @return true if successful, false if not able to release reservation.
     */
    virtual bool releaseReservationIfMatches(std::string reservationName) = 0;

    /**
     * Destructor.
     */
    virtual ~ProcessSlot() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_PROCESSSLOT_H_ */
