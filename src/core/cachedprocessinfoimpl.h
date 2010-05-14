/*
 * cachedprocessslotimpl.h --
 *
 * Implementation of class CachedProcessInfoImpl; it represents the cached
 * process info of a notifyable.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDPROCESSINFOIMPL_H_
#define _CL_CACHEDPROCESSINFOIMPL_H_

namespace clusterlib
{

/**
 * Definition of class CachedProcessInfoImpl
 */
class CachedProcessInfoImpl
    : public virtual CachedDataImpl,
      public virtual CachedProcessInfo
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    virtual json::JSONValue::JSONArray getPortArr();

    virtual void setPortArr(const json::JSONValue::JSONArray &portArr);

    /**
     * Constructor.
     */
    explicit CachedProcessInfoImpl(NotifyableImpl *ntp);

    /**
     * Destructor.
     */
    virtual ~CachedProcessInfoImpl() {}
    
  private:
    /**
     * The port array stored as a JSONArray
     */
    ::json::JSONValue::JSONArray m_portArr;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSINFOIMPL_H_ */
