/*
 * processthreadservice.cc --
 *
 * Implementation of the ProcessThreadService class.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"
#include <iomanip>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/unistd.h>
_syscall0(pid_t,gettid)

using namespace std;

namespace clusterlib
{

pid_t 
ProcessThreadService::forkExec(const vector<string> &addEnv, 
                               const string &path, 
                               const string &cmd,
                               int *stdin,
                               int *stdout,
                               int *stderr)
{
    TRACE(CL_LOG, "forkExec");
    
    /* 
     * Debugging - note: Do not put any log4cxx messages between the
     * fork and exec as this can cause problems.
     */
    LOG_DEBUG(CL_LOG, 
              "ForkExec: running from PATH=%s\nCMD=%s\n"
              "added ENV:",
              path.c_str(),
              cmd.c_str());
    for (vector<string>::const_iterator it = addEnv.begin(); 
         it != addEnv.end();
         ++it) {
        LOG_DEBUG(CL_LOG, "env: %s", it->c_str());
    }

    /* Setup the fds and pipes for interacting with the forked process */
    ostringstream oss;
    int stdinFd[2];
    int stdoutFd[2];
    int stderrFd[2];
    int ret = -1;
    if (stdin != NULL) {
        ret = pipe(stdinFd);
        if (ret != 0) {
            oss.str("");
            oss << "forkExec: pipe(stdinFd) failed with ret=" << ret 
                << " errno=" << errno << " strerror=" << strerror(errno);
            throw SystemFailureException(oss.str());
        }
    }
    if (stdout != NULL) {
        ret = pipe(stdoutFd);
        if (ret != 0) {
            oss.str("");
            oss << "forkExec: pipe(stdoutFd) failed with ret=" << ret
                << " errno=" << errno << " strerror=" << strerror(errno);
            throw SystemFailureException(oss.str());
        }
    }
    if (stderr != NULL) {
        ret = pipe(stderrFd);
        if (ret != 0) {
            oss.str("");
            oss << "forkExec: pipe(sterrFd) failed with ret=" << ret
                << " errno=" << errno << " strerror=" << strerror(errno);
            throw SystemFailureException(oss.str());
        }
    }
    
    pid_t pid = fork();
    /* I am the child */
    if (pid == 0) {
        /*
         * If desired, duplicate the stdin, stdout, and stderr file
         * descriptors and then close the child process's access to
         * them.
         */
        if (stdin != NULL) {
            ret = dup2(stdinFd[0], STDIN_FILENO);
            if (ret == -1) {
                oss.str("");
                oss << "forkExec: dup2(stdin) failed with ret=" << ret
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            ret = close(stdinFd[0]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stdinFd[0]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            ret = close(stdinFd[1]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stdinFd[1]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
        }
        if (stdout != NULL) {
            ret = dup2(stdoutFd[1], STDOUT_FILENO);
            if (ret == -1) {
                oss.str("");
                oss << "forkExec: dup2(stdout) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            ret = close(stdoutFd[0]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stdoutFd[0]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            ret = close(stdoutFd[1]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stdoutFd[1]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
        }
        if (stderr != NULL) {
            ret = dup2(stderrFd[1], STDERR_FILENO);
            if (ret == -1) {
                oss.str("");
                oss << "forkExec: dup2(stderr) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            ret = close(stderrFd[0]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stderrFd[0]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            ret = close(stderrFd[1]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stderrFd[1]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
        }
        
	/* Change path if specified */
	if (path.size() != 0) {
	    chdir(path.c_str());
	}
	
	/* Execute */
	if (addEnv.size() != 0) {
	    /* Get the size of the current environment */
	    int envCount = 0;
	    while (environ[envCount]) { 
                ++envCount;
            }
	    
	    int newEnvCount = envCount + addEnv.size();
	    /* One extra for the blank one */
	    char **newEnvArr = new char *[newEnvCount + 1];
	    for (int i = 0; i < newEnvCount; i++) {
		if (i < envCount) {
		    newEnvArr[i] = new char[strlen(environ[i]) + 1];
		    strcpy(newEnvArr[i], environ[i]);
		}
		else {
		    newEnvArr[i] = new char[addEnv[i - envCount].size() + 1];
		    strncpy(newEnvArr[i], 
                            addEnv[i - envCount].c_str(), 
			    addEnv[i - envCount].size() + 1);
		}
	    }
	    newEnvArr[newEnvCount] = NULL;
	    ret = execle("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL, 
			 newEnvArr);
	    for (int i = envCount; i < newEnvCount; ++i) {
		delete [] newEnvArr[i];
	    }
	    delete [] newEnvArr;
	}
	else {
	    ret = execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL);
	}

	if (ret == -1) {
            ostringstream oss;
            oss << "ForkExec: execl failed with error " << errno << " " 
                << strerror(errno);
            LOG_FATAL(CL_LOG, oss.str().c_str());
            throw SystemFailureException(oss.str());
	}

        exit(-1);
    }
    else { /* I am the parent */
	if (pid == -1) {
            LOG_FATAL(CL_LOG, 
                      "ForkExec: Failed with pid == -1 for cmd=%s",
                      cmd.c_str());
	    throw SystemFailureException("ForkExec: fork failed");
	}
        
        /* Don't read from stdin, write to stdout, or write to stderr */
        if (stdin != NULL) {
            ret = close(stdinFd[0]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stdinFd[0]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            *stdin = stdinFd[1];
        }
        if (stdout != NULL) {
            ret = close(stdoutFd[1]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stdoutFd[1]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            *stdout = stdoutFd[0];
        }
        if (stderr != NULL) {
            ret = close(stderrFd[1]);
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: close(stderrFd[1]) failed with ret=" << ret 
                    << " errno=" << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            *stderr = stderrFd[0];
        }

	return pid;
    }
}

pid_t 
ProcessThreadService::forkExec(const vector<string> &addEnv, 
                               const string &path, 
                               const string &cmd)
{
    return ProcessThreadService::forkExec(
        addEnv, path, cmd, NULL, NULL, NULL);
}

bool
ProcessThreadService::waitPid(pid_t processId, int32_t &returnCode)
{
    TRACE(CL_LOG, "waitPid");
    ostringstream oss;

    if (processId <= 0) {
        oss.str("");
        oss << "waitPid: Cannot have processId <= 0 (" << processId << ")";
        throw InvalidArgumentsException(oss.str());
    }

    int statVal;
    pid_t curId = -1;
    returnCode = -1;
    oss.str("");
    oss << "waitPid: Waiting for processId " << processId;
    LOG_DEBUG(CL_LOG, "%s", oss.str().c_str());

    /*
     * Since the 'options = 0', waitpid will wait until the
     * termination of the process, not just a change in state.
     */
    curId = ::waitpid(processId, &statVal, 0);
    if (curId == -1) {
        oss.str("");
        oss << "waitPid: Error waiting for pid " << processId 
            << " with errno=" << errno << " and error=" << strerror(errno);
        LOG_ERROR(CL_LOG, "%s", oss.str().c_str());
        return false;
    }

    if (WIFEXITED(statVal)) {
        returnCode = WEXITSTATUS(statVal);
        return true;
    }
    return false;
}

bool
ProcessThreadService::forkExecWait(const vector<string> &addEnv, 
                                   const string &path, 
                                   const string &cmd,
                                   pid_t &processId,
                                   int32_t &returnCode)
    
{
    TRACE(CL_LOG, "forkExecWait");

    processId = ProcessThreadService::forkExec(addEnv, path, cmd);
    return ProcessThreadService::waitPid(processId, returnCode);
}

bool
ProcessThreadService::forkExecWait(const vector<string> &addEnv, 
                                   const string &path, 
                                   const string &cmd,
                                   pid_t &processId,
                                   int32_t &returnCode,
                                   string &stdoutOutput,
                                   string &stderrOutput)
    
{
    TRACE(CL_LOG, "forkExecWait");

    int stdout = -1;
    int stderr = -1;

    processId = ProcessThreadService::forkExec(
        addEnv, path, cmd, NULL, &stdout, &stderr);

    ostringstream oss;
    int ret = -1;

    /* Collect output from stdout and stderror until the end of file */
    stdoutOutput.clear();
    stderrOutput.clear();
    ssize_t stdoutRet = 1;
    ssize_t stderrRet = 1;
    const int32_t bufSize = 4096;
    char buf[bufSize];
    while ((stdoutRet > 0) || (stderrRet > 0)) {
        if (stdoutRet > 0) {
            bzero(buf, sizeof(buf));
            stdoutRet = read(stdout, buf, bufSize);
            if (stdoutRet < 0) {
                oss.str("");
                oss << "forkExecWait: read failed ret=" << stdoutRet
                    << " errno="  << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            LOG_DEBUG(CL_LOG, 
                      "forkExecWait: stdout ret=%" PRId32 " buf=%s",
                      static_cast<int32_t>(stdoutRet),
                      buf);
            stdoutOutput.append(buf, stdoutRet);
        }
        if (stderrRet > 0) {
            bzero(buf, sizeof(buf));
            stderrRet = read(stderr, buf, bufSize);
            if (stderrRet < 0) {
                oss.str("");
                oss << "forkExecWait: read failed ret=" << stderrRet
                    << " errno="  << errno << " strerror=" << strerror(errno);
                throw SystemFailureException(oss.str());
            }
            LOG_DEBUG(CL_LOG, 
                      "forkExecWait: stderr ret=%" PRId32 " buf=%s",
                      static_cast<int32_t>(stderrRet),
                      buf);
            stderrOutput.append(buf, stderrRet);
        }
    }
        
    ret = close(stdout);
    if (ret != 0) {
        oss.str("");
        oss << "forkExecWait: close(stdout) failed with ret=" << ret 
            << " errno=" << errno << " strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }
    ret = close(stderr);
    if (ret != 0) {
        oss.str("");
        oss << "forkExecWait: close(stderr) failed with ret=" << ret 
            << " errno=" << errno << " strerror=" << strerror(errno);
        throw SystemFailureException(oss.str());
    }    

    return ProcessThreadService::waitPid(processId, returnCode);
}

string 
ProcessThreadService::getHostname()
{
    TRACE(CL_LOG, "getHostname");
    
    const int32_t bufLen = 256;
    char tmp[bufLen + 1];
    tmp[bufLen] = '\0';
    if (gethostname(tmp, bufLen) != 0) {
        throw SystemFailureException("getHostnamePidTid: gethostname failed");
    }

    return tmp;
}

pid_t
ProcessThreadService::getPid()
{
    TRACE(CL_LOG, "getPid");
    
    return getpid();
}

int32_t
ProcessThreadService::getTid()
{
    TRACE(CL_LOG, "getTid");

    return static_cast<int32_t>(gettid());
}

string 
ProcessThreadService::getHostnamePidTid()
{
    TRACE(CL_LOG, "getHostnamePidTid");
    
    /*
     * Get the hostname, pid, and tid of the calling
     * thread.
     */
    stringstream ss;
    ss << getHostname() << ".pid." << getPid() 
       << ".tid." << gettid();
    return ss.str();
}

}	/* End of 'namespace clusterlib' */
