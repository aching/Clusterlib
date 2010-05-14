/*
 * cachedprocessinfo.h --
 *
 * Definition of class CachedProcessInfo; it represents the cached
 * process info of a ProcessSlot.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDPROCESSINFO_H_
#define _CL_CACHEDPROCESSINFO_H_

namespace clusterlib
{

/**
 * Definition of class CachedProcessInfo
 */
class CachedProcessInfo
    : public virtual CachedData
{
  public:
    /**
     * Get a vector of ports (user-defined).  They are of type
     * JSONValue::JSONInteger.  If not defined, will be empty.
     *
     * @return the vector of ports
     */
    virtual json::JSONValue::JSONArray getPortArr() = 0;

    /**
     * Set a vector of ports (user-defined) as a JSONValue.
     *
     * @param portArr The JSONArray of ports to set as a JSONValue
     */
    virtual void setPortArr(const json::JSONValue::JSONArray &portArr) = 0;
 
    /**
     * Destructor.
     */
    virtual ~CachedProcessInfo() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSINFO_H_ */
