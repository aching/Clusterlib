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
    virtual void notify(const Event e);

    /*
     * Register an event handler for a set of events.
     */
    void registerInterest(ClusterEventHandler *cehp,
                          Event mask,
                          void *clientData)
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

    /*
     * Is this notifyable "ready"? (according to the
     * ready protocol)
     */
    bool isReady() { return m_ready; }

    /*
     * Get the properties for this node
     */
    Properties *getProperties(bool create = false);

  protected:
    /*
     * Factory is a friend so it can call the below constructor.
     */
    friend class Factory;

    /*
     * Constructor.
     */
    Notifyable(FactoryOps *f,
               const string &key,
               const string &name);

    /*
     * Set the "ready" state of this notifyable.
     */
    void setReady(bool v) { m_ready = v; }

    /*
     * Get the associated factory delegate object.
     */
    FactoryOps *getDelegate() { return mp_f; }

    /*
     * Destructor.
     */
    virtual ~Notifyable();

    /*
     * Update the cached representation -- must be provided
     * by subclasses.
     */
    virtual void updateCachedRepresentation()
        = 0;

    /*
     * Get the address of the lock for the node map.
     */
    Mutex *getPropertiesMapLock() { return &m_propLock; }

  private:
    /*
     * Retrieve the handlers lock.
     */
    Mutex *getInterestsLock() { return &m_interestsLock; }

    /*
     * Default constructor.
     */
    Notifyable() 
    {
        throw ClusterException("Someone called the Notifyable "
                               "default constructor!");
    };

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

    /*
     * Is this notifyable "ready" according to the ready
     * protocol?
     */
    bool m_ready;

    /*
     * Map of all properties within this object.
     */
    PropertiesMap m_properties;
    Mutex m_propLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
