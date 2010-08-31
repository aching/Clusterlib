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

using namespace std;
using namespace boost;

namespace clusterlib {

Factory::Factory(const string &registry, int64_t msecConnectTimeout)
    : m_ops(NULL)
{
    TRACE(CL_LOG, "Factory");

    m_ops = new FactoryOps(registry, msecConnectTimeout);
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
Factory::createJSONRPCResponseClient(
    const shared_ptr<Queue> &responseQueueSP,
    const shared_ptr<Queue> &completedQueueSP)
{
    return getOps()->createJSONRPCResponseClient(responseQueueSP,
                                                 completedQueueSP);
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

}	/* End of 'namespace clusterlib' */
