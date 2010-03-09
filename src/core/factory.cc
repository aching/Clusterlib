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
Factory::createJSONRPCMethodClient(
    Queue *recvQueue,
    Queue *completedQueue,
    int32_t completedQueueMaxSize,
    PropertyList *rpcMethodHandlerPropertyList,
    ::json::rpc::JSONRPCManager *rpcManager)
{
    return getOps()->createJSONRPCMethodClient(recvQueue,
                                               completedQueue,
                                               completedQueueMaxSize,
                                               rpcMethodHandlerPropertyList,
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

};	/* End of 'namespace clusterlib' */
