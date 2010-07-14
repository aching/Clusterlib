/*
 * processthreadservice.h --
 *
 * Definition and implementation of ProcessThreadService class.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_PROCESSTHREADSERVICE_H_
#define	_CL_PROCESSTHREADSERVICE_H_

namespace clusterlib
{

/**
 * This class provides a static functions to help manipulate processes
 * and therads.
 */
class ProcessThreadService 
{
  public:
    /**
     * Starts up a process (no stdin, stdout, stderr redirection)
     * 
     * @param addEnv the additional environment variables
     * @param path the path to execute the command from
     * @param cmd the command to execute
     * @return the process id of the newly created process
     */
    static pid_t forkExec(const std::vector<std::string> &addEnv, 
                          const std::string &path, 
                          const std::string &cmd);

    /**
     * Wait for status information of a particular process id.
     *
     * @param processId the process id to track
     * @param returnCode the return code (valid only if the return is true)
     * @return true if returned normally, false if stopped in a 
     *         non-expected way
     */
    static bool waitPid(pid_t processId, int32_t &returnCode);

    /**
     * Try to execute and wait for a single command (no fd redirection).
     *
     * @param addEnv the additional environment variables
     * @param path the path to execute the command from
     * @param cmd the command to execute
     * @param processId the process id that was created
     * @param returnCode the return code (valid only if the return is true)
     * @return true if returned normally, false if stopped in a 
     *         non-expected way
     */
    static bool forkExecWait(const std::vector<std::string> &addEnv, 
                             const std::string &path, 
                             const std::string &cmd,
                             pid_t &processId,
                             int32_t &returnCode);

    /**
     * Starts up a process and duplicate stdin, stdout, and stderr fds.
     * 
     * @param addEnv the additional environment variables
     * @param path the path to execute the command from
     * @param cmd the command to execute
     * @param stdin If not NULL, is set to the stdin fd (write only)
     * @param stdout If not NULL, is set to the stdout fd (read only)
     * @param stderr If not NULL, is set to the stderr fd (read only)
     * @return the process id of the newly created process
     */
    static pid_t forkExec(const std::vector<std::string> &addEnv, 
                          const std::string &path, 
                          const std::string &cmd,
                          int *stdin,
                          int *stdout,
                          int *stderr);

    /**
     * Try to execute and wait for a single command (and collect
     * stdout, stderr).
     *
     * @param addEnv the additional environment variables
     * @param path the path to execute the command from
     * @param cmd the command to execute
     * @param processId the process id that was created
     * @param returnCode the return code (valid only if the return is true)
     * @param stdoutOutput the string output from the stdout fd
     * @param stderrOutput the string output from the stderr fd
     * @return true if returned normally, false if stopped in a 
     *         non-expected way
     */
    static bool forkExecWait(const std::vector<std::string> &addEnv, 
                             const std::string &path, 
                             const std::string &cmd,
                             pid_t &processId,
                             int32_t &returnCode,
                             std::string &stdoutOutput,
                             std::string &stderrOutput);

    /**
     * Get hostname string.  Helper function.
     *
     * @return string of hostname
     */
    static std::string getHostname();

    /**
     * Get the process id.
     * 
     * @return The process id of this process.
     */
    static pid_t getPid();

    /**
     * Get the thread id.
     * 
     * @return thread id converted to int32_t
     */
    static int32_t getTid();

    /**
     * Get hostname, process id and thread id string.  Useful for
     * uniquely identifying a client.
     *
     * @return string of hostname, process id, and thread id
     */
    static std::string getHostnamePidTid();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_PROCESSTHREADSERVICE_H_ */
