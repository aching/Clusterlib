/*
 * datadistribution.h --
 *
 * Interface of class DataDistribution; it represents a data
 * distribution (mapping from a key or a HashRange to a notifyable) in
 * clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_DATADISTRIBUTION_H_
#define _CL_DATADISTRIBUTION_H_

namespace clusterlib {

/**
 * Definition of class DataDistribution.
 */
class DataDistribution
    : public virtual Notifyable
{
  public:
    /**
     * Access the cached shards
     *
     * @return A reference to the CachedShards.
     */
    virtual CachedShards &cachedShards() = 0;

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~DataDistribution() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_DATADISTRIBUTION_H_ */
