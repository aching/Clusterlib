/*
 * processslotimpl.h --
 *
 * Definition of class ProcessSlotImpl; it represents a process slot
 * on a node in an application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_PROCESSSLOTIMPL_H_
#define _CL_PROCESSSLOTIMPL_H_

namespace clusterlib
{

/**
 * Definition of class ProcessSlotImpl.
 */
class ProcessSlotImpl
    : public virtual ProcessSlot, 
      public virtual NotifyableImpl
{
  public:
    virtual json::JSONValue getJsonPortVec();
    virtual std::vector<int32_t> getPortVec();
    
    virtual void setPortVec(std::vector<int32_t> portVec);
    virtual void setJsonPortVec(json::JSONValue jsonValue);

    virtual void start();

    virtual bool getExecArgs(std::vector<std::string> &addEnv,
                             std::string &path,
                             std::string &cmd);
    virtual json::JSONValue getJsonExecArgs();

    virtual void setExecArgs(const std::vector<std::string> &addEnv,
                             const std::string &path,
                             const std::string &cmd);
    virtual void setJsonExecArgs(json::JSONValue jsonValue);

    virtual bool getRunningExecArgs(std::vector<std::string> &addEnv,
                                    std::string &path,
                                    std::string &cmd);
    virtual json::JSONValue getJsonRunningExecArgs();

    virtual int32_t getPID();
    virtual json::JSONValue getJsonPID();

    virtual void stop(int32_t sig = 15);

    virtual ProcessState getDesiredProcessState();

    virtual ProcessState getCurrentProcessState();

    virtual std::string getReservationName();

    virtual bool setReservationIfEmpty(std::string reservationName);

    virtual bool releaseReservationIfMatches(std::string reservationName);

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /*
     * Constructor used by the factory.
     */
    ProcessSlotImpl(FactoryOps *fp,
                    const std::string &key,
                    const std::string &name,
                    NodeImpl *node)
        : NotifyableImpl(fp, key, name, node) {}

    /**
     * Sets the PID after a start.
     *
     * @param pid pid to be set of running process
     */
    void setPID(int32_t pid);

    /**
     * Set the current process state
     *
     * @param processState the current process state
     */
    void setCurrentProcessState(ProcessState processState);
    
    /**
     * Start up the process with the user-defined args on this server
     *
     * @return the pid of the new process
     */
    pid_t startLocal();
    
    /**
     * Stop the process with the user-defined args on this server
     */
    void stopLocal();

    /**
     * Set the running executable arguments.
     *
     * @param addEnv the additional environment variables
     * @param path the path to execute the command from
     * @param cmd the command to execute
     */
    virtual void setRunningExecArgs(const std::vector<std::string> &addEnv,
                                    const std::string &path,
                                    const std::string &cmd);

    /**
     * Set the running executable arguments as a JSONValue.
     *
     * @param jsonValue the JSONValue with all executable arguments.
     */
    virtual void setRunningJsonExecArgs(json::JSONValue jsonValue);

    /**
     * Create a string with the default executable arguments
     */
    static std::string createDefaultExecArgs();
    
    /*
     * Destructor.
     */
    virtual ~ProcessSlotImpl();

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries();

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    ProcessSlotImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException("Someone called the ProcessSlotImpl "
                                     "default constructor!");
    }
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_PROCESSSLOTIMPL_H_ */
