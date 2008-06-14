/*
 * notifyable.h --
 *
 * Contains the base class for notifyable objects.
 *
 * $Header:$
 * $Revision:$
 * $Date:$
 */

#ifndef	_NOTIFYABLE_H_
#define _NOTIFYABLE_H_

using namespace std;

namespace clusterlib
{

class NotificationObject
{
  public:
    /*
     * Enumerate the possible events being notified.
     */
    enum Event {
        NE_ILLEGAL		= -1,

        NE_APPLICATION_DELETED	= 0,
        NE_APPLICATION_CREATED  = 1,
        NE_APPLICATION_PROPS    = 2,

        NE_GROUP_DELETED        = 10,
        NE_GROUP_CREATED	= 11,
        NE_GROUP_PROPS          = 12,

        NE_NODE_DELETED		= 20,
        NE_NODE_CREATED		= 21,
        NE_NODE_PROPS		= 22,

        NE_DIST_DELETED		= 30,
        NE_DIST_CREATED		= 31,
        NE_DIST_MODIFIED	= 32,
        NE_DIST_PROPS		= 33
    };

    /*
     * The key of the object being notified.
     */
    const string getKey() { return m_key; }

    /*
     * The event being notified.
     */
    Event getEvent() { return m_e; }

  protected:
    /*
     * Declare a friend relation with Factory so that
     * the factory can call a protected constructor.
     */
    friend class Factory;

    /*
     * Protected constructor used by the factory.
     */
    NotificationObject(const string &key,
                       Event e)
        : m_key(key),
          m_e(e)
    {
    };

  private:
    /*
     * The key for which the event is.
     */
    const string m_key;

    /*
     * The event (type of notification).
     */
    const Event m_e;
};

/*
 * Interface that must be derived by specific notifyable objects.
 */
class Notifyable
{
  public:
    /*
     * Notify the notifyable object.
     */
    virtual void notify(const NotificationObject *notification) = 0;
};

/*
 * Base class for all objects that can have associated Notifyable objects.
 */
class NotificationTarget
{
  public:
    /*
     * Retrieve the notification receiver associated with
     * this data distribution.
     */
    Notifyable *getNotifyable() { return mp_nrp; }

    /*
     * Set the associated notification receiver object
     * to a new notification receiver.
     */
    void setNotifyable(Notifyable *nrp)
    {
#ifdef	NOT_YET_IMPLEMENTED
        if (mp_nrp != NULL) {
            mp_f->removeInterests(m_key, mp_nrp);
        }
        mp_f->addDistributionInterests(m_key, mp_nrp);
#endif
        mp_nrp = nrp;
    }

    /*
     * Constructor.
     */
    NotificationTarget(Notifyable *nrp)
        : mp_nrp(nrp)
    {
    };

  private:
    /*
     * Default constructor is hidden so it cannot be called.
     */
    NotificationTarget() {}

  private:
    /*
     * Associated notifyable object.
     */
    Notifyable *mp_nrp;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
