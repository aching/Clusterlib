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
    : public virtual Notifyable
{
  public:
    /*
     * Retrieve the name of the application.
     */
    const string getName() { return m_name; }

    /*
     * Retrieve a named group within an application.
     */
    Group *getGroup(const string &groupName)
        throw(ClusterException);

    /*
     * Retrieve a map of all groups within the application.
     */
    GroupMap *getGroups();

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

    /*
     * Deliver received event notifications.
     */
    void deliverNotification(const Event e);

    /*
     * Have the distribution and group maps been filled already?
     */
    bool filledGroupMap() { return m_filledGroupMap; }
    bool filledDataDistributionMap() { return m_filledDataDistributionMap; }

  protected:
    /*
     * Friend declaration for factory so that it can call
     * the protected constructor.
     */
    friend class Factory;

    /*
     * Constructor used by Factory.
     */
    Application(const string &name, const string &key, FactoryOps *f)
        : Notifyable(f, key),
          m_name(name)
    {
        m_groups.clear();
        m_distributions.clear();
    }

    /*
     * Did we already fill the group and/or distribution maps?
     */
    void setFilledGroupMap(bool v) { m_filledGroupMap = v; }

    void setFilledDataDistributionMap(bool v)
    {
        m_filledDataDistributionMap = v;
    }

    Mutex *getGroupMapLock() { return &m_grpLock; }
    Mutex *getDistributionMapLock() { return &m_distLock; }

  private:
    /*
     * The default constructor is private so noone can call it.
     */
    Application()
        : Notifyable(NULL, "")
    {
        throw ClusterException("Someone called the Application "
                               "default constructor!");
    }

  private:
    /*
     * The name of this application.
     */
    const string m_name;

    /*
     * Map of all groups within this application.
     */
    GroupMap m_groups;
    bool m_filledGroupMap;
    Mutex m_grpLock;

    /*
     * Map of all data distributions within this application.
     */
    DataDistributionMap m_distributions;
    bool m_filledDataDistributionMap;
    Mutex m_distLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_APPLICATION_H_ */

