/*
 * clusterlib.cc --
 *
 * Implementation of the Factory class.
 *
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
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
    m_notificationInterests.clear();

    ZooKeeperConfig cfg(registry, 3000);
    mp_zk = new ZooKeeperAdapter(cfg, this, true);
};

/*
 * Destructor of Factory
 */
Factory::~Factory()
{
};

/*
 * Create a client.
 */
ClusterClient *
Factory::createClient()
{
    return NULL;
};

/*
 * Create a server.
 */
ClusterServer *
Factory::createServer(const string &app,
                      const string &group,
                      const string &node,
                      HealthChecker *checker,
                      bool createReg)
{
    return NULL;
};

/*
 * Handle events.
 */
void
Factory::eventReceived(const ZKEventSource zksrc, const ZKWatcherEvent &e)
{
};

/**********************************************************************/
/* Below this line are the private methods of class Factory.          */
/**********************************************************************/

/*
 * Manage interests in events.
 */
void
Factory::addInterests(Notifyable *nrp, const Event events)
{
    InterestRecord *r = m_notificationInterests[nrp->getKey()];

    if (r == NULL) {
        r = new InterestRecord(nrp, events);
        m_notificationInterests[nrp->getKey()] = r;
    }
    r->addInterests(events);
};

void
Factory::removeInterests(Notifyable *nrp, const Event events)
{
    InterestRecord *r = m_notificationInterests[nrp->getKey()];

    if (r != NULL) {
        r->removeInterests(events);
        if (r->getInterests() == 0) {
            m_notificationInterests.erase(nrp->getKey());
            delete r;
        }
    }
};

};	/* End of 'namespace clusterlib' */
