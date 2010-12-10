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
#ifdef HAVE_MACH_THREAD_SELF
#include <mach/mach.h>
#include <sys/resource.h>
#include <algorithm>
#else
#include <sys/syscall.h>
#endif
#include <poll.h>

extern char **environ;

using namespace std;

namespace clusterlib {

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
	    ret = chdir(path.c_str());
            if (ret != 0) {
                oss.str("");
                oss << "forkExec: chdir to " << path << " failed with ret=" 
                    << ret << " errno=" << errno << " strerror=" 
                    << strerror(errno);
                throw SystemFailureException(oss.str());
            }
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
            LOG_FATAL(CL_LOG, "%s", oss.str().c_str());
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

const int32_t PollMsecTimeout = 500;
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

    struct pollfd fds[2];
    fds[0].fd = stdout;
    fds[1].fd = stderr;
    fds[0].events = POLLIN | POLLHUP;
    fds[1].events = POLLIN | POLLHUP;

    ssize_t sizeArr[2] = {1, 1};

    /* Collect output from stdout and stderror until the end of file */
    stdoutOutput.clear();
    stderrOutput.clear();
    const int32_t bufSize = 4096;
    char buf[bufSize];
    while ((sizeArr[0] > 0) || (sizeArr[1] > 0)) {
        ret = poll(fds, 2, PollMsecTimeout);
        if (ret > 0) {
            for (int32_t i = 0; i < 2; ++i) {
                if ((sizeArr[i] > 0) && (fds[i].revents & POLLIN)) {
                    bzero(buf, sizeof(buf));
                    sizeArr[i] = read(fds[i].fd, buf, bufSize);
                    if (sizeArr[i] < 0) {
                        oss.str("");
                        oss << "forkExecWait: fd=" << i << " read failed ret=" 
                            << sizeArr[i] << " errno="  << errno
                            << " strerror=" << strerror(errno);
                        throw SystemFailureException(oss.str());
                    }
                    LOG_DEBUG(CL_LOG, 
                              "forkExecWait: (%s) fd=%" PRId32 
                              " size=%" PRId32 ", total_buffer_size=% " PRId64 
                              " buf=%s",
                              ((i == 0) ? "stdout" : "stderr"),
                              static_cast<int32_t>(fds[i].fd),
                              static_cast<int32_t>(sizeArr[i]),
                              static_cast<int64_t>(
                                  ((i == 0) ? stdoutOutput.size() : 
                                   stderrOutput.size()) + sizeArr[i]),
                              buf);
                    switch(i) {
                        case 0:
                            stdoutOutput.append(buf, sizeArr[i]);
                            break;
                        case 1:
                            stderrOutput.append(buf, sizeArr[i]);
                            break;
                        default:
                            oss.str("");
                            oss << "forkExecWait: fd=" << i << " ret=" 
                                << sizeArr[i] << " errno="  << errno
                                << " strerror=" << strerror(errno);
                            throw SystemFailureException(oss.str());
                    }
                }
                else if (fds[i].revents & POLLHUP) {
                    oss.str("");
                    oss << "forkExecWait: ("
                        << ((i == 0) ? "stdout" : "stderr") << ") fd=" 
                        << fds[i].fd << " got POLLHUP "
                        << "errno=" << errno << " strerror=" 
                        << strerror(errno);
                    LOG_DEBUG(CL_LOG, "%s", oss.str().c_str());
                    sizeArr[i] = 0;
                }
            }
        }
        else if (ret == 0) {
            LOG_DEBUG(CL_LOG, "forkExecWait: No poll event happened");
        }
        else {
            oss.str("");
            oss << "forkExecWait: poll failed with ret=" << ret 
                << " errno=" << errno << " strerror=" << strerror(errno);
            throw SystemFailureException(oss.str());
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

#ifdef HAVE_MACH_THREAD_SELF
    return static_cast<int32_t>(mach_thread_self());
#else
    return static_cast<int32_t>(syscall(__NR_gettid));
#endif
}

string 
ProcessThreadService::getHostnamePidTid()
{
    TRACE(CL_LOG, "getHostnamePidTid");
    
    /*
     * Get the hostname, pid, and tid of the calling
     * thread.
     */
    ostringstream oss;
    oss << getHostname() << ".pid." << getPid() 
	<< ".tid." << getTid();
    return oss.str();
}

}	/* End of 'namespace clusterlib' */
