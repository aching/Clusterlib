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
    virtual CachedProcessInfo &cachedProcessInfo();

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
        : NotifyableImpl(fp, key, name, node),
          m_cachedProcessInfo(this) {}


    /**
     * Create the key-value JSONArray key
     *
     * @param processSlotKey the ProcessSlot key
     * @return the generated process info JSONArray key
     */
    static std::string createProcessInfoJsonArrKey(
        const std::string &processSlotKey);

    /**
     * Helper function to get the exeutable arguments from a cancelled
     * state.
     */
    void getExecArgs(CachedState &cachedState,
                     std::vector<std::string> &addEnv, 
                     std::string &path, 
                     std::string &command);

    /**
     * Start up the process with given arguments this node.
     *
     * @param addEnv The added environment for the new process.
     * @param path The path to execute the command.
     * @param command The command to start.
     * @return the pid of the new process
     */
    pid_t startLocal(const std::vector<std::string> &addEnv, 
                     const std::string &path, 
                     const std::string &command);
    
    /**
     * Stop the process with the user-defined args on this server
     *
     * @param pid The pid to send the signal to.
     * @param signal The kill signal.
     */
    void stopLocal(pid_t pid, int32_t signal);

    /**
     * Create a string with the default executable arguments
     */
    static std::string createDefaultExecArgs();
    
    /*
     * Destructor.
     */
    virtual ~ProcessSlotImpl();

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    ProcessSlotImpl()
        : NotifyableImpl(NULL, "", "", NULL),
          m_cachedProcessInfo(this)
    {
        throw InvalidMethodException("Someone called the ProcessSlotImpl "
                                     "default constructor!");
    }

  private:
    /**
     * The cached process info
     */
    CachedProcessInfoImpl m_cachedProcessInfo;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_PROCESSSLOTIMPL_H_ */
