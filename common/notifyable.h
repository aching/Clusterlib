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
     * Add a notification receiver to the chain.
     */
    void add(NotificationReceiver *ap)
    {
        if (mp_next == NULL) {
            mp_next = ap;
        } else {
            mp_next->add(ap);
        }
    };

    /*
     * Remove a notification receiver from the chain.
     */
    void remove(NotificationReceiver *ap)
    {
        if (mp_next == NULL) {
            return;
        }
        if (mp_next != ap) {
            mp_next->remove(ap);
            return;
        }
        mp_next = ap->mp_next;
        ap->mp_next = NULL;
    }

    /*
     * Constructor.
     */
    NotificationReceiver(const Event mask)
        : m_mask(mask),
          mp_next(NULL)
    {
    }

    /*
     * Destructor.
     */
    virtual ~NotificationReceiver()
    {
        if (mp_next != NULL) {
            throw ClusterException("Destroying a registered "
                                   "notification receiver!");
        }
    }

    /*
     * This must be implemented by subclasses.
     */
    virtual void deliverNotification(const Event e) = 0;
        
    /*
     * Is this event one we're interested in?
     */
    bool matchEvent(const Event e) { return (m_mask & e) == 0 ? false : true; }

    /*
     * Return the next receiver in the chain.
     */
    NotificationReceiver *next() { return mp_next; }

  private:
    /*
     * The events that this receiver is interested in receiving.
     */
    Event m_mask;

    /*
     * The next receiver in the chain.
     */
    NotificationReceiver *mp_next;
};

/*
 * Interface that must be derived by specific notifyable objects.
 */
class Notifyable
{
  public:
    /*
     * Constructor.
     */
    Notifyable(FactoryOps *f)
        : mp_f(f),
          mp_unrp(NULL)
    {
    };

    /*
     * Destructor.
     */
    virtual ~Notifyable() {};

    /*
     * Get the associated factory delegate object.
     */
    FactoryOps *getDelegate() { return mp_f; }

  public:
    /*
     * Notify the notifyable object.
     */
    virtual void notify(const Event e)
    {
        NotificationReceiver *unrp;

        deliverNotification(e);
        for (unrp = mp_unrp; unrp != NULL; unrp = unrp->next()) {
            if (unrp->matchEvent(e)) {
                unrp->deliverNotification(e);
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
        unrp->add(NULL);
        if (mp_unrp == NULL) {
            mp_unrp = unrp;
        } else {
            mp_unrp->add(unrp);
        }
    };

    /*
     * Unregister a receiver.
     */
    void unregisterNotificationReceiver(NotificationReceiver *unrp)
    {
        if (unrp == NULL) {
            return;
        }
        if (mp_unrp == unrp) {
            mp_unrp = unrp->next();
        } else {
            mp_unrp->remove(unrp);
        }
    }

  private:
    /*
     * The associated factory delegate.
     */
    FactoryOps *mp_f;

    /*
     * The list of receivers attached to this notifyable object.
     */
    NotificationReceiver *mp_unrp;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
