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
 * Interface for notification receiver. Must be derived
 * to define specific behavior for handling events.
 */
class NotificationReceiver
{
  public:
    /*
     * Constructor.
     */
    NotificationReceiver(const Event mask)
        : m_mask(mask)
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
    bool matchEvent(const Event e) { return (m_mask & e) == 0 ? false : true; }

    bool operator==(NotificationReceiver *other)
    {
        return ((long) this == (long) other) ? true : false;
    }

  private:
    /*
     * The events that this receiver is interested in receiving.
     */
    Event m_mask;
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

  public:
    /*
     * Notify the notifyable object.
     */
    virtual void notify(const Event e)
    {
        NotificationReceivers::iterator i;

        deliverNotification(e);
        for (i = m_receivers.begin(); i != m_receivers.end(); i++) {
            if ((*i)->matchEvent(e)) {
                (*i)->deliverNotification(e);
            }
        }
    };

    /*
     * This must be supplied by subclasses.
     */
    virtual void deliverNotification(const Event e) = 0;

    /*
     * Register a receiver.
     */
    void registerNotificationReceiver(NotificationReceiver *unrp)
    {
        m_receivers.push_back(unrp);
    };

    /*
     * Unregister a receiver.
     */
    void unregisterNotificationReceiver(NotificationReceiver *unrp)
    {
        NotificationReceivers::iterator i;

        if ((i = find(m_receivers.begin(), m_receivers.end(), unrp))
            != m_receivers.end()) {
            m_receivers.erase(i);
        }
    }

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
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
