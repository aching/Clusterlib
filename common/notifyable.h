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
    bool operator==(const Notifyable &other)
    {
        return (other.getKey() == getKey()) ? true : false;
    }

    /*
     * Get the name of the Notifyable.
     */
    string getName() const { return m_name; }

    /*
     * Return the string identifying the represented
     * cluster object.
     */
    string getKey() const { return m_key; }

    /*
     * Is this notifyable "ready"? (according to the
     * ready protocol)
     */
    bool isReady() const { return m_ready; }

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
    Notifyable(FactoryOps *fp,
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
