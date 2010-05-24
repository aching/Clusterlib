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

bool
Factory::removeClient(Client *client)
{
    TRACE(CL_LOG, "removeClient");

    return getOps()->removeClient(dynamic_cast<ClientImpl *>(client));
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
    ClusterlibRPCManager *rpcManager)
{
    return getOps()->createJSONRPCMethodClient(rpcManager);
}

bool
Factory::isConnected()
{
    return getOps()->isConnected();
}

void
Factory::registerPeriodicThread(Periodic &periodic)
{
    getOps()->registerPeriodicThread(periodic);
}

bool
Factory::cancelPeriodicThread(Periodic &periodic)
{
    return getOps()->cancelPeriodicThread(periodic);
}

void
Factory::synchronize()
{
    TRACE(CL_LOG, "synchronize");

    return getOps()->synchronize();
}

void
Factory::registerHashRange(const HashRange &hashRange)
{
    TRACE(CL_LOG, "registerHashRange");

    getOps()->registerHashRange(hashRange);
}

zk::ZooKeeperAdapter *
Factory::getRepository()
{
    TRACE(CL_LOG, "getRepository");

    return getOps()->getRepository();
}

};	/* End of 'namespace clusterlib' */
