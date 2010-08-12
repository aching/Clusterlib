/*
 * propertylistimpl.h --
 *
 * Definition of class PropertyListImpl; it represents the property list of a
 * clusterlib object.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_PROPERTYLISTIMPL_H_
#define _CL_PROPERTYLISTIMPL_H_

namespace clusterlib {

/**
 * Definition of class PropertyListImpl
 */
class PropertyListImpl
    : public virtual PropertyList,
      public virtual NotifyableImpl
{
  public:
    virtual CachedKeyValues &cachedKeyValues();    

    virtual bool getPropertyListWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<PropertyList> *pPropertyListSP);

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /**
     * Constructor.
     */
    PropertyListImpl(FactoryOps *fp,
                     const std::string &key,
                     const std::string &name,
                     const boost::shared_ptr<NotifyableImpl> &parent);

    virtual NotifyableList getChildrenNotifyables();
    
    virtual void initializeCachedRepresentation();

    /**
     * Create the key-value JSONObject key
     *
     * @param propertyListKey the property list key
     * @return the generated key-value JSONObject key
     */
    static std::string createKeyValJsonObjectKey(
        const std::string &propertyListKey);

  private:
    /**
     * Do not call the default constructor.
     */
    PropertyListImpl();

  private:
    /**
     * The cached key-values
     */
    CachedKeyValuesImpl m_cachedKeyValues;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_PROPERTYLISTIMPL_H_ */

