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

/*
 * Constructor of Factory.
 *
 * Connect to cluster via the registry specification.
 * Initialize all the cache data structures.
 * Create the associated FactoryOps delegate object.
 */
Factory::Factory(const string &registry)
    : m_ops(NULL)
{
    TRACE(CL_LOG, "Factory");

    m_ops = new FactoryOps(registry);
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

};	/* End of 'namespace clusterlib' */
