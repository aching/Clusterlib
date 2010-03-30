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
                               const string &cmd)
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
    
    pid_t pid = fork();

    /* I am the child */
    if (pid == 0) {
	int ret = -1;

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

        LOG_DEBUG(CL_LOG, 
                  "ForkExec: Finished execl(e) with ret=%d for command %s", 
                  ret,
                  cmd.c_str());
	if (ret == -1) {
            ostringstream oss;
            oss << "ForkExec: execl failed with error " << errno << " " 
                << strerror(errno);
            LOG_FATAL(CL_LOG, oss.str().c_str());
            throw SystemFailureException(oss.str());
	}
    }
    else { /* I am the parent */
	if (pid == -1) {
            LOG_FATAL(CL_LOG, 
                      "ForkExec: Failed with pid == -1 for cmd=%s",
                      cmd.c_str());
	    throw SystemFailureException("ForkExec: fork failed");
	}

	return pid;
    }

    return pid;
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
    while (curId != processId) {
        oss.str("");
        oss << "waitPid: Waiting for processId " << processId;
        LOG_DEBUG(CL_LOG, oss.str().c_str());
        curId = ::wait(&statVal);
        if (curId == -1) {
            if (errno == ECHILD) {
                LOG_ERROR(CL_LOG,
                          "waitPid: No child processes while waiting for %d", 
                          processId);
                return false;
            }
        }

        oss.str();
        oss << "waitPid: Got curId " << curId
            << " (looking for " << processId << ")"; 
        LOG_DEBUG(CL_LOG, oss.str().c_str());
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

string 
ProcessThreadService::getHostnamePidTid()
{
    TRACE(CL_LOG, "getHostnamePidTid");
    
    const int32_t bufLen = 256;
    char tmp[bufLen + 1];
    tmp[bufLen] = '\0';
    if (gethostname(tmp, bufLen) != 0) {
        throw SystemFailureException("getHostnamePidTid: gethostname failed");
    }

    /*
     * Get the hostname, pid, and tid of the calling
     * thread.
     */
    stringstream ss;
    ss << tmp << ".pid." << getpid() 
       << ".tid." << gettid();
    return ss.str();
}

};	/* End of 'namespace clusterlib' */
