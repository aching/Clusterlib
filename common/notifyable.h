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

const int EN_NODE_CREATION		(1<<10);
const int EN_NODE_DELETION		(1<<11);
const int EN_NODE_HEALTHCHANGE		(1<<12);
const int EN_NODE_CONNECTCHANGE		(1<<13);
const int EN_NODE_MASTERSTATECHANGE	(1<<14);

const int EN_DIST_CREATION		(1<<20);
const int EN_DIST_DELETION		(1<<21);
const int EN_DIST_CHANGE		(1<<22);

const int EN_APP_INTERESTS =    (EN_APP_CREATION | EN_APP_DELETION);
const int EN_GRP_INTERESTS =	(EN_GRP_CREATION |
                                 EN_GRP_DELETION |
                                 EN_GRP_MEMBERSHIP);
const int EN_NODE_INTERESTS =	(EN_NODE_CREATION |
                                 EN_NODE_DELETION |
                                 EN_NODE_HEALTHCHANGE |
                                 EN_NODE_CONNECTCHANGE |
                                 EN_NODE_MASTERSTATECHANGE);
const int EN_DIST_INTERESTS =	(EN_DIST_CREATION |
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
    NotificationReceiver *mp_rp;
    Event m_e;

    /*
     * Constructor.
     */
    ClientEvent(NotificationReceiver *rp, Event e)
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
 * Interface for notification receiver. Must be derived
 * to define specific behavior for handling events.
 */
class NotificationReceiver
{
  public:
    /*
     * Constructor.
     */
    NotificationReceiver(const Event mask,
                         ClusterClient *cl,
                         Notifyable *np)
        : m_mask(mask),
          mp_notifyable(np),
          mp_cl(cl)
    {
    }

    /*
     * Destructor.
     */
    virtual ~NotificationReceiver() {}

    /*
     * This must be implemented by subclasses.
     */
    virtual void deliverNotification(const Event e) = 0;
        
    /*
     * Is this event one we're interested in?
     */
    bool matchEvent(const Event e)
    {
        return (m_mask & e) == 0 ? false : true;
    }

    /*
     * Equality operator.
     */
    bool operator==(NotificationReceiver *other)
    {
        return ((long) this == (long) other) ? true : false;
    }

    /*
     * Get the object for which the event is being delivered.
     */
    Notifyable *getNotifyable() { return mp_notifyable; }

    /*
     * Get the client to which this notification receiver
     * object belongs.
     */
    ClusterClient *getClient() { return mp_cl; }

  private:
    /*
     * The events that this receiver is interested in receiving.
     */
    Event m_mask;

    /*
     * The notifyable object that the events are about.
     */
    Notifyable *mp_notifyable;

    /*
     * The client to which we belong.
     */
    ClusterClient *mp_cl;
};

/*
 * A list of notification receivers.
 */
typedef vector<NotificationReceiver *> NotificationReceivers;

/*
 * Interface that must be derived by specific notifyable objects.
 */
class Notifyable
{
  public:
    /*
     * Constructor.
     */
    Notifyable(FactoryOps *f, const string &key)
        : mp_f(f),
          m_key(key)
    {
        m_receivers.clear();
    };

    /*
     * Destructor.
     */
    virtual ~Notifyable() {};

    /*
     * Get the associated factory delegate object.
     */
    FactoryOps *getDelegate() { return mp_f; }

    /*
     * Return the string identifying the represented
     * cluster object.
     */
    const string getKey() { return m_key; }

    /*
     * Notify the notifyable object.
     */
    virtual void notify(const Event e,
                        const string &key)
    {
        NotificationReceivers::iterator i;
        NotificationReceivers receivers;

        /*
         * Update the cached object.
         */
        deliverNotification(e);

        {
            /*
             * Make a copy of the current notification reciever
             * list to protect against side effects. Suggested by
             * ruslanm@yahoo-inc.com, thanks!
             */
            Locker l(getReceiversLock());

            receivers = m_receivers;
        }

        /*
         * Deliver the event to all interested "user-land" clients.
         */
        for (i = receivers.begin(); i != receivers.end(); i++) {
            if ((*i)->matchEvent(e)) {
                (*i)->deliverNotification(e);
            }
        }
    };

    /*
     * This must be supplied by subclasses. Use it to
     * update the cached representation before the user
     * defined notification receivers are called.
     */
    virtual void deliverNotification(const Event e) = 0;

    /*
     * Register a receiver.
     */
    void registerNotificationReceiver(NotificationReceiver *unrp)
    {
        Locker l(getReceiversLock());

        m_receivers.push_back(unrp);
    };

    /*
     * Unregister a receiver.
     */
    void unregisterNotificationReceiver(NotificationReceiver *unrp)
    {
        NotificationReceivers::iterator i;
        Locker l(getReceiversLock());

        if ((i = find(m_receivers.begin(), m_receivers.end(), unrp))
            != m_receivers.end()) {
            m_receivers.erase(i);
        }
    }

  private:
    Mutex *getReceiversLock() { return &m_receiversLock; }

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
     * The vector of notification receivers associated with
     * this notifyable.
     */
    NotificationReceivers m_receivers;
    Mutex m_receiversLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
