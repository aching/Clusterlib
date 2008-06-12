/*
 * datadistribution.h --
 *
 * Definition of class DataDistribution; it represents a data distribution (mapping
 * from a key to a node) in clusterlib.
 *
 * $Header:$
 * $Revision:$
 * $Date:$
 */

#ifndef	_DATADISTRIBUTION_H_
#define _DATADISTRIBUTION_H_

namespace clusterlib
{

/*
 * Definition of class DataDistribution.
 */
class DataDistribution
{
  public:
    /*
     * Retrieve the name of this distribution.
     */
    const string getName()
    {
        return m_name;
    }

    /*
     * Retrieve the application object in which this
     * distribution is contained.
     */
    const Application *getApplication()
    {
        return mp_app;
    }

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
                     Factory *f,
                     ::zk::ZooKeeperAdapter *zk)
        : mp_f(f),
          mp_zk(zk),
          mp_app(app),
          m_name(name)
    {
    }

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    DataDistribution()
    {
        throw ClusterException("Someone called the DataDistribution "
                               "default constructor!");
    }

  private:
    /*
     * The factory instance we're using.
     */
    Factory *mp_f;

    /*
     * The instance of ZooKeeper adapter we're using.
     */
    ::zk::ZooKeeperAdapter *mp_zk;

    /*
     * The application object for the application that contains
     * this distribution.
     */
    const Application *mp_app;

    /*
     * The name of this data distribution.
     */
    const string m_name;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_DATADISTRIBUTION_H_ */
