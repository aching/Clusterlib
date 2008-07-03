/*
 * clusterlib.h --
 *
 * The main include file for users of clusterlib.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERLIB_H_
#define	_CLUSTERLIB_H_

#include <string>
#include <vector>
#include <map>

#include "forwarddecls.h"
#include "zkadapter.h"
#include "blockingqueue.h"
#include "mutex.h"
#include "thread.h"
#include "event.h"
#include "clusterexception.h"
#include "notifyable.h"

using namespace std;
using namespace zk;

#include "healthchecker.h"
#include "clusterclient.h"
#include "clusterserver.h"
#include "application.h"
#include "group.h"
#include "node.h"
#include "datadistribution.h"

namespace clusterlib
{

/*
 * The actual factory class.
 */
class Factory
    : public virtual ZKEventListener
{
  public:
    /*
     * Create a factory instance, connect it to
     * the specified cluster registry.
     */
    Factory(const string &registry);

    /*
     * Destructor.
     */
    virtual ~Factory();

    /*
     * Create a cluster client object.
     */
    ClusterClient *createClient();

    /*
     * Create a cluster server object. Also
     * create the needed registration if createReg
     * is set to true.
     */
    ClusterServer *createServer(const string &app,
                                const string &group,
                                const string &node,
                                HealthChecker *checker,
                                bool createReg = false);

    /*
     * This API must be provided because of inheritance from
     * EventListener.
     */
    void eventReceived(const ZKEventSource zksrc, const ZKWatcherEvent &e);

  private:
    /*
     * Friend declaration for FactoryOps so it'll have
     * access to all these private operations.
     */
    friend class FactoryOps;

    /*
     * Manage interests in events.
     */
    void addInterests(Notifyable *nrp, const Event events);
    void removeInterests(Notifyable *nrp, const Event events);

    /*
     * Retrieve (and potentially create) instances of
     * objects representing applications, groups, nodes,
     * and distributions.
     */
    Application *getApplication(const string &name);
    Group *getGroup(const string &name, Application *app);
    Node *getNode(const string &name, Group *grp);
    DataDistribution *getDistribution(const string &name,
                                      Application *app);

  private:
    /*
     * The factory ops delegator.
     */
    FactoryOps *m_ops;

    /*
     * The registry of cached data distributions.
     */
    DataDistributionMap m_dataDistributions;

    /*
     * The registry of cached applications.
     */
    ApplicationMap m_applications;

    /*
     * The registry of cached groups.
     */
    GroupMap m_groups;

    /*
     * The registry of cached nodes.
     */
    NodeMap m_nodes;

    /*
     * The ZooKeeper adapter object being used.
     */
    ZooKeeperAdapter *mp_zk;

    class InterestRecord
    {
      public:
        /*
         * Constructor.
         */
        InterestRecord(Notifyable *nrp, Event e)
            : mp_nrp(nrp),
              m_e(e)
        {
        };

        /*
         * Retrieve the elements of the record.
         */
        Event getInterests() { return m_e; }
        Notifyable *getNotifyable() { return mp_nrp; }

        /*
         * Manage the selection of which notifications
         * are delivered.
         */
        void addInterests(Event e)
        {
            m_e |= e;
        }
        void removeInterests(Event e)
        {
            m_e &= (~(e));
        }

      private:
        /*
         * The notifyable object to deliver events to.
         */
        Notifyable *mp_nrp;

        /*
         * The events of interest.
         */
        Event m_e;
    };
    typedef map<string, InterestRecord *> NotificationInterestsMap;

    /*
     * Map from keys to notification interests records.
     */
    NotificationInterestsMap m_notificationInterests;
};

/*
 * Definition and implementation of class FactoryOps. An
 * instance of this class is given to internal objects so
 * that these objects can call operations on the factory
 * that shouldn't be generally available.
 */
class FactoryOps
{
  public:
    void addInterests(Notifyable *nrp, Event events)
    {
        mp_f->addInterests(nrp, events);
    }
    void removeInterests(Notifyable *nrp, Event events)
    {
        mp_f->removeInterests(nrp, events);
    }

    Application *getApplication(const string &name)
    {
        return mp_f->getApplication(name);
    }
    Group *getGroup(const string &name, Application *app)
    {
        return mp_f->getGroup(name, app);
    }
    Node *getNode(const string &name, Group *grp)
    {
        return mp_f->getNode(name, grp);
    }
    DataDistribution *getDistribution(const string &name,
                                      Application *app)
    {
        return mp_f->getDistribution(name, app);
    }
    
  private:
    /*
     * Friend declaration for Factory so it can call
     * the constructor.
     */
    friend class Factory;
    
    /*
     * Constructor used by Factory.
     */
    FactoryOps(Factory *f) : mp_f(f) {};

    /*
     * Default constructor throws an exception.
     */
    FactoryOps()
    {
        throw ClusterException("Someone called the default "
                               "FactoryOps constructor!");
    }

  private:
    /*
     * The factory associated with this instance of FactoryOps.
     */
    Factory *mp_f;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIB_H_ */
