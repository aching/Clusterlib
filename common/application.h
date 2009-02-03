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
     * Retrieve a named group within an application.
     */
    Group *getGroup(const string &groupName,
		    bool create = false);

    /*
     * Retrieve a map of all currently known groups within
     * the application.
     */
    GroupMap getGroups()
    {
        Locker l(getGroupMapLock());

        m_cachingGroups = true;
        recacheGroups();
        return m_groups;
    }


    /*
     * Retrieve a named data distribution within an
     * application.
     */
    DataDistribution *getDistribution(const string &distName,
				      bool create = false);

    /*
     * Retrieve a map of all currently known data distributions
     * within the application.
     */
    DataDistributionMap getDistributions()
    {
        Locker l(getDistributionMapLock());

        m_cachingDists = true;
        recacheDists();
        return m_distributions;
    }

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
        : Notifyable(f, key, name),
          m_cachingGroups(false),
          m_cachingDists(false)
    {
        m_groups.clear();
        m_distributions.clear();

        updateCachedRepresentation();
    }

    /*
     * Are we caching the distributions and groups fully?
     */
    bool cachingGroups() { return m_cachingGroups; }
    bool cachingDists() { return m_cachingDists; }
    void recacheGroups();
    void recacheDists();

    /*
     * Get locks associated with the various maps.
     */
    Mutex *getGroupMapLock() { return &m_grpLock; }
    Mutex *getDistributionMapLock() { return &m_distLock; }

    /*
     * Update the cached representation.
     */
    virtual void updateCachedRepresentation();

  private:
    /*
     * The default constructor is private so noone can call it.
     */
    Application()
        : Notifyable(NULL, "", "")
    {
        throw ClusterException("Someone called the Application "
                               "default constructor!");
    }

    /*
     * Make the destructor private also.
     */
    ~Application() {};

  private:
    /*
     * Map of all groups within this application.
     */
    GroupMap m_groups;
    Mutex m_grpLock;

    /*
     * Map of all data distributions within this application.
     */
    DataDistributionMap m_distributions;
    Mutex m_distLock;

    /*
     * Variables to remember whether we're caching groups
     * and distributions fully.
     */
    bool m_cachingGroups;
    bool m_cachingDists;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_APPLICATION_H_ */

