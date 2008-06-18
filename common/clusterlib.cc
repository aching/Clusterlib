/*
 * clusterlib.cc --
 *
 * Implementation of the Factory class.
 *
 * =============================================================================
 * $Header$
 * $Revision:$
 * $Date:$
 * =============================================================================
 */

#include "clusterlib.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{

/*
 * Constructor of Factory.
 *
 * Connect to ZooKeeper via the registry specification.
 * Initialize all the cache data structures.
 * Create the associated FactoryOps delegate object.
 */
Factory::Factory(const string &registry)
{
    m_dataDistributions.clear();
    m_applications.clear();
    m_groups.clear();
    m_nodes.clear();

    ZooKeeperConfig cfg(registry, 3000);
    mp_zk = new ZooKeeperAdapter(cfg, this, true);
};

/*
 * Destructor of Factory
 */
Factory::~Factory()
{
};

};	/* End of 'namespace clusterlib' */
