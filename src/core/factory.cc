/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
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
