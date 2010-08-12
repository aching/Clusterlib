/*
 * datadistributionimpl.h --
 *
 * Definition of class DataDistribution; it represents a data
 * distribution (mapping from a key to a notifyable) in clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_DATADISTRIBUTIONIMPL_H_
#define _CL_DATADISTRIBUTIONIMPL_H_

namespace clusterlib {

/**
 * Definition of class DataDistribution.
 */
class DataDistributionImpl
    : public virtual DataDistribution, 
      public virtual NotifyableImpl
{
  public:
    virtual CachedShards &cachedShards();

  public:
    /**
     * Constructor used by Factory.
     */
    DataDistributionImpl(FactoryOps *fp,
                         const std::string &key,
                         const std::string &name,
                         const boost::shared_ptr<NotifyableImpl> &parentGroup);

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistributionImpl();

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

    /**
     * Create the shard JSONObject key
     *
     * @param dataDistributionKey the data distribution key
     * @return the generated shard JSONObject key
     */
    static std::string createShardJsonObjectKey(
        const std::string &dataDistributionKey);

  private:
    /**
     * Do not call the default constructor
     */
    DataDistributionImpl();
    
  private:
    /**
     * The cached shards.
     */
    CachedShardsImpl m_cachedShards;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_DATADISTRIBUTIONIMPL_H_ */
