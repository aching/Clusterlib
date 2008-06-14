/*
 * application.h --
 *
 * Definition of class Application; it represents a set of groups of nodes that
 * together form a clusterlib application.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_APPLICATION_H_
#define _APPLICATION_H_

namespace clusterlib
{

/*
 * Definition of class Application.
 */
class Application
    : public virtual NotificationTarget
{
  public:
    /*
     * Retrieve the name of the application.
     */
    const string getName() { return m_name; }

    /*
     * Retrieve a named group within an application.
     */
    const Group *getGroup(const string &groupName)
        throw(ClusterException);

    /*
     * Retrieve a map of all groups within the application.
     */
    GroupMap *getGroups() { return &m_groups; }

    /*
     * Retrieve a named data distribution within an
     * application.
     */
    DataDistribution *getDistribution(const string &distName)
        throw(ClusterException);

    /*
     * Retrieve a map of all data distributions within the
     * application (at the application level).
     */
    DataDistributionMap *getDistributions() { return &m_distributions; }

  protected:
    /*
     * Friend declaration for factory so that it can call
     * the protected constructor.
     */
    friend class Factory;

    /*
     * Constructor used by Factory.
     */
    Application(const string &name,
                const string &key,
                Factory *f,
                Notifyable *nrp)
        : NotificationTarget(nrp),
	  mp_f(f),
          m_name(name),
          m_key(key)
    {
        m_groups.clear();
        m_distributions.clear();
    }

    /*
     * Allow the factory access to my key.
     */
    const string getKey() { return m_key; }

  private:
    /*
     * The default constructor is private so noone can call it.
     */
    Application()
        : NotificationTarget(NULL)
    {
        throw ClusterException("Someone called the Application "
                               "default constructor!");
    }

  private:
    /*
     * The instance of the factory in use.
     */
    Factory *mp_f;

    /*
     * The name of this application.
     */
    const string m_name;

    /*
     * The key associated with this application.
     */
    const string m_key;

    /*
     * Map of all groups within this application.
     */
    GroupMap m_groups;

    /*
     * Map of all data distributions within this application.
     */
    DataDistributionMap m_distributions;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_APPLICATION_H_ */

