/*
 * notifyable.h --
 *
 * Contains the base class for notifyable objects.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_NOTIFYABLE_H_
#define _NOTIFYABLE_H_

using namespace std;

namespace clusterlib
{

/*
 * Various notification constants.
 */
const int EN_APP_CREATION		(1<<0);
const int EN_APP_DELETION		(1<<1);

const int EN_GRP_CREATION		(1<<5);
const int EN_GRP_DELETION		(1<<6);
const int EN_GRP_MEMBERSHIP		(1<<7);
const int EN_GRP_LEADERSHIP		(1<<8);

const int EN_NODE_CREATION		(1<<10);
const int EN_NODE_DELETION		(1<<11);
const int EN_NODE_HEALTHCHANGE		(1<<12);
const int EN_NODE_CONNECTCHANGE		(1<<13);
const int EN_NODE_MASTERSTATECHANGE	(1<<14);

const int EN_DIST_CREATION		(1<<20);
const int EN_DIST_DELETION		(1<<21);
const int EN_DIST_CHANGE		(1<<22);

const int EN_PROP_CHANGE		(1<<25);

const int EN_TIMER_EXPIRED		(1<<30);

const int EN_APP_INTERESTS =    (EN_PROP_CHANGE |
                                 EN_APP_CREATION |
                                 EN_APP_DELETION);
const int EN_GRP_INTERESTS =	(EN_PROP_CHANGE |
                                 EN_GRP_CREATION |
                                 EN_GRP_DELETION |
                                 EN_GRP_MEMBERSHIP |
                                 EN_GRP_LEADERSHIP);
const int EN_NODE_INTERESTS =	(EN_PROP_CHANGE |
                                 EN_NODE_CREATION |
                                 EN_NODE_DELETION |
                                 EN_NODE_HEALTHCHANGE |
                                 EN_NODE_CONNECTCHANGE |
                                 EN_NODE_MASTERSTATECHANGE);
const int EN_DIST_INTERESTS =	(EN_PROP_CHANGE |
                                 EN_DIST_CREATION |
                                 EN_DIST_DELETION |
                                 EN_DIST_CHANGE);

/*
 * An event type.
 */
typedef int Event;

/*
 * Structure for passing events+handler to clients.
 */
struct ClientEvent
{
    ClusterEventHandler *mp_rp;
    Event m_e;

    /*
     * Constructor.
     */
    ClientEvent(ClusterEventHandler *rp, Event e)
    {
        m_e = e;
        mp_rp = rp;
    }
};

/*
 * Typedef for client event delivery queue.
 */
typedef BlockingQueue<ClientEvent *> ClientEventQueue;

/*
 * Interface for event handler. Must be derived
 * to define specific behavior for handling events.
 */
class ClusterEventHandler
{
  public:
    /*
     * Constructor.
     */
    ClusterEventHandler()
    {
    }

    /*
     * Destructor.
     */
    virtual ~ClusterEventHandler() {}

    /*
     * This must be implemented by subclasses.
     */
    virtual void deliverNotification(const Event e,
                                     Notifyable *np,
                                     void *clientData)
        = 0;
};

/*
 * Class for recording interest in events.
 */
class ClusterEventInterest
{
  public:
   /*
    * Constructor.
    */
    ClusterEventInterest(ClusterEventHandler *handler,
                         Event mask,
                         void *clientData)
        : m_mask(mask),
          mp_handler(handler),
          mp_data(clientData)
    {
    }

    /*
     * Deliver the specific event.
     */
    void deliverNotification(Event e, Notifyable *np)
    {
        mp_handler->deliverNotification(e, np, mp_data);
    }

    /*
     * Get the handler object.
     */
    ClusterEventHandler *getHandler() { return mp_handler; }

    /*
     * Add/remove events to the set we're interested in.
     */
    Event addEvents(Event mask)
    {
        m_mask |= mask;
        return m_mask;
    }
    Event removeEvents(Event mask)
    {
        m_mask &= (~(mask));
        return m_mask;
    }

    /*
     * Is the event one we're interested in?
     */
    bool matchEvent(Event e)
    {
        return ((m_mask & e) == 0) ? false : true;
    }

    /*
     * Get the set of events this interest is for.
     */
    Event getEvents() { return m_mask; }

    /*
     * Set the clientData.
     */
    void *setClientData(void *newCLD)
    {
        void *oldCLD = mp_data;
        mp_data = newCLD;
        return oldCLD;
    }

  private:
    /*
     * The events of interest.
     */
    Event m_mask;

    /*
     * The handler to invoke.
     */
    ClusterEventHandler *mp_handler;

    /*
     * Arbitrary data to pass to deliverNotification().
     */
    void *mp_data;
};
    
/*
 * A list of EventHandler instances.
 */
typedef vector<ClusterEventInterest *> ClusterEventInterests;

/*
 * Interface that must be derived by specific notifyable objects.
 */
class Notifyable
{
  public:
    /*
     * Compare two Notifyable instances.
     */
    bool operator==(Notifyable &other)
    {
        return (other.getKey() == getKey()) ? true : false;
    }

    /*
     * Notify the notifyable object.
     */
    virtual void notify(const Event e)
    {
        ClusterEventInterests::iterator i;
        ClusterEventInterests interests;

        /*
         * Update the cached object.
         */
        deliverNotification(e);

        {
            /*
             * Make a copy of the current notification interests
             * list to protect against side effects. Suggested by
             * ruslanm@yahoo-inc.com, thanks!
             */
            Locker l(getInterestsLock());

            interests = m_interests;
        }

        /*
         * Deliver the event to all interested "user-land" clients.
         */
        for (i = interests.begin(); i != interests.end(); i++) {
            if ((*i)->matchEvent(e)) {
                (*i)->deliverNotification(e, this);
            }
        }
    };

    /*
     * Register an event handler for a set of events.
     */
    void registerInterest(ClusterEventHandler *cehp,
                          Event mask,
                          void *clientData)
        throw(ClusterException)
    {
        ClusterEventInterests::iterator i;
        Locker l(getInterestsLock());

        for (i = m_interests.begin();
             i != m_interests.end();
             i++) {
            if ((*i)->getHandler() == cehp) {
                (*i)->addEvents(mask);
                (*i)->setClientData(clientData);
                return;
            }
        }
        m_interests.push_back(new ClusterEventInterest(cehp,
                                                       mask,
                                                       clientData));
    };

    /*
     * Unregister an event handler for a set of events.
     */
    void unregisterInterest(ClusterEventHandler *cehp, Event mask)
        throw(ClusterException)
    {
        ClusterEventInterests::iterator i;
        ClusterEventInterest *pi;
        Locker l(getInterestsLock());

        for (i = m_interests.begin();
             i != m_interests.end();
             i++) {
            if ((*i)->getHandler() == cehp) {
                if ((*i)->removeEvents(mask) == 0) {
                    pi = (*i);
                    m_interests.erase(i);
                    delete pi;
                }
            }
        }
    }

    /*
     * Get the name of the Notifyable.
     */
    string getName() { return m_name; }

    /*
     * Return the string identifying the represented
     * cluster object.
     */
    string getKey() { return m_key; }

  protected:
    /*
     * This must be supplied by subclasses. Use it to
     * update the cached representation before the user
     * defined notification receivers are called.
     */
    virtual void deliverNotification(const Event e) = 0;

    /*
     * Factory is a friend so it can call the below constructor.
     */
    friend class Factory;

    /*
     * Constructor.
     */
    Notifyable(FactoryOps *f,
               const string &key,
               const string &name)
        : mp_f(f),
          m_key(key),
          m_name(name)
    {
        m_interests.clear();
    };

    /*
     * Get the associated factory delegate object.
     */
    FactoryOps *getDelegate() { return mp_f; }

    /*
     * Destructor.
     */
    virtual ~Notifyable()
    {
        ClusterEventInterests::iterator i;
        Locker l(getInterestsLock());

        for (i = m_interests.begin();
             i != m_interests.end();
             i++) {
            delete (*i);
        }
        m_interests.clear();
    };

  private:
    /*
     * Retrieve the handlers lock.
     */
    Mutex *getInterestsLock() { return &m_interestsLock; }

    /*
     * Default constructor.
     */
    Notifyable() {};

  private:
    /*
     * The associated factory delegate.
     */
    FactoryOps *mp_f;

    /*
     * The key to pass to the factory delegate for
     * operations on the represented cluster node.
     */
    const string m_key;

    /*
     * The name of the Notifyable.
     */
    const string m_name;

    /*
     * The vector of notification receivers associated with
     * this notifyable.
     */
    ClusterEventInterests m_interests;
    Mutex m_interestsLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
