/*
 * factory.cc --
 *
 * Implementation of the Factory class.
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
#include <linux/unistd.h>
_syscall0(pid_t,gettid)

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;

namespace clusterlib
{

Factory::Factory(const string &registry, int64_t connectTimeout)
    : m_ops(NULL)
{
    TRACE(CL_LOG, "Factory");

    m_ops = new FactoryOps(registry, connectTimeout);
}

/*
 * Destructor of Factory
 */
Factory::~Factory()
{
    TRACE(CL_LOG, "~Factory");

    delete m_ops;
}

Client *
Factory::createClient()
{
    TRACE(CL_LOG, "createClient");

    return getOps()->createClient();
}

Client *
Factory::createJSONRPCResponseClient(Queue *responseQueue,
                                     Queue *completedQueue)
{
    return getOps()->createJSONRPCResponseClient(responseQueue,
                                                 completedQueue);
}

Client *
Factory::createJSONRPCMethodClient(Queue *recvQueue,
                                   Queue *completedQueue,
                                   ::json::rpc::JSONRPCManager *rpcManager)
{
    return getOps()->createJSONRPCMethodClient(recvQueue,
                                               completedQueue,
                                               rpcManager);
}


bool
Factory::isConnected()
{
    return getOps()->isConnected();
}

void
Factory::synchronize()
{
    TRACE(CL_LOG, "synchronize");

    return getOps()->synchronize();
}

zk::ZooKeeperAdapter *
Factory::getRepository()
{
    TRACE(CL_LOG, "getRepository");

    return getOps()->getRepository();
}

string 
Factory::getHostnamePidTid()
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
