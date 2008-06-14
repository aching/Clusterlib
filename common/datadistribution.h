/*
 * datadistribution.h --
 *
 * Definition of class DataDistribution; it represents a data distribution (mapping
 * from a key to a node) in clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_DATADISTRIBUTION_H_
#define _DATADISTRIBUTION_H_

namespace clusterlib
{

/*
 * Definition of class DataDistribution.
 */
class DataDistribution
    : public virtual NotificationTarget
{
  public:
    /*
     * Retrieve the name of this distribution.
     */
    const string getName() { return m_name; }

    /*
     * Retrieve the application object in which this
     * distribution is contained.
     */
    const Application *getApplication() { return mp_app; }

  protected:
    /*
     * Friend declaration of Factory so that it can call the
     * protected constructor.
     */
    friend class Factory;

    /*
     * Constructor used by Factory.
     */
    DataDistribution(const Application *app,
                     const string &name,
                     const string &key,
                     Factory *f,
                     Notifyable *nrp)
        : NotificationTarget(nrp),
          mp_f(f),
          mp_app(app),
          m_name(name),
          m_key(key)
    {
        m_shards.clear();
        m_overrides.clear();
#ifdef	NOT_IMPLMENTED_YET
        if (nrp != NULL) {
            mp_f->addDistributionInterests(m_key, nrp);
        }
#endif
        updateDistribution();
    }

    /*
     * Allow the factory access to my key.
     */
    const string getKey() { return m_key; }

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    DataDistribution()
        : NotificationTarget(NULL)
    {
        throw ClusterException("Someone called the DataDistribution "
                               "default constructor!");
    }

    /*
     * Update the distribution.
     */
    void updateDistribution() throw(ClusterException);

  private:
    /*
     * The factory instance we're using.
     */
    Factory *mp_f;

    /*
     * The application object for the application that contains
     * this distribution.
     */
    const Application *mp_app;

    /*
     * The name of this data distribution.
     */
    const string m_name;

    /*
     * The factory key for this object.
     */
    const string m_key;

    /*
     * The shards in this data distribution.
     */
    ShardList m_shards;

    /*
     * The manual overrides for this data distribution.
     */
    ManualOverridesMap m_overrides;
};

/*
 * Definition of class Shard.
 */
class Shard
{
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_DATADISTRIBUTION_H_ */
