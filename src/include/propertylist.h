/*
 * propertylist.h --
 *
 * Interface of class PropertyList; it represents a property list
 * child of a clusterlib object.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_PROPERTYLIST_H_
#define _CL_PROPERTYLIST_H_

namespace clusterlib {

/**
 * Definition of class PropertyList
 */
class PropertyList
    : public virtual Notifyable
{
  public:
    /**
     * Access the cached key-values
     *
     * @return A reference to the cached key-values.
     */
    virtual CachedKeyValues &cachedKeyValues() = 0;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIB_H_ */

