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

namespace clusterlib {

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
    virtual json::JSONValue::JSONArray getHostnameArr();

    virtual void setHostnameArr(const json::JSONValue::JSONArray &hostnameArr);

    virtual json::JSONValue::JSONArray getPortArr();

    virtual void setPortArr(const json::JSONValue::JSONArray &portArr);

    /**
     * Constructor.
     */
    explicit CachedProcessInfoImpl(NotifyableImpl *notifyable);

    /**
     * Destructor.
     */
    virtual ~CachedProcessInfoImpl() {}
    
  private:
    /**
     * Marshal the m_hostnameArr and m_portArr into a JSONValue for publishing.
     *
     * @return An JSON array of the two arrays.
     */
    json::JSONValue::JSONArray marshal();

    /**
     * Unmarshal the two arrays into this object
     *
     * @param encodedJsonArr The encoded JSON array of two arrays
     */
    void unmarshal(const std::string &encodedJsonArr);

  private:
    /**
     * The hostname array stored as a JSONArray
     */
    ::json::JSONValue::JSONArray m_hostnameArr;

    /**
     * The port array stored as a JSONArray
     */
    ::json::JSONValue::JSONArray m_portArr;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSINFOIMPL_H_ */
