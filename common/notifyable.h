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
