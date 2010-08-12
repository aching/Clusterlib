/*
 * cachedkeyvaluesimpl.h --
 *
 * Implementation of class CachedKeyValuesImpl; it represents the cached
 * current state of a notifyable.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CACHEDKEYVALUESIMPL_H_
#define _CL_CACHEDKEYVALUESIMPL_H_

namespace clusterlib {

/**
 * Definition of class CachedKeyValuesImpl
 */
class CachedKeyValuesImpl
    : public virtual CachedDataImpl,
      public virtual CachedKeyValues
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    virtual std::vector<json::JSONValue::JSONString> getKeys();

    virtual bool get(
        const std::string &key, 
        json::JSONValue &jsonValue,
        bool searchParent = false,
        int64_t ancestorMsecTimeout = -1,
        boost::shared_ptr<PropertyList> *pUsedProperyListSP = NULL);

    virtual void set(const ::json::JSONValue::JSONString &key, 
                     const ::json::JSONValue &value);

    virtual bool erase(const ::json::JSONValue::JSONString &key);

    virtual void clear();

    /**
     * Constructor.
     */
    explicit CachedKeyValuesImpl(NotifyableImpl *notifyable);

    /**
     * Destructor.
     */
    virtual ~CachedKeyValuesImpl() {}
    
  private:
    /**
     * The key values state in user-defined format.
     */
    ::json::JSONValue::JSONObject m_keyValues;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDKEYVALUESIMPL_H_ */
