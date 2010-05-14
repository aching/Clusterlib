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

namespace clusterlib
{

/**
 * Definition of class PropertyListImpl
 */
class PropertyListImpl
    : public virtual PropertyList,
      public virtual NotifyableImpl
{
  public:
    virtual CachedKeyValues &cachedKeyValues();    

    virtual PropertyList *getPropertyList(
        const std::string &name,
        AccessType accessType = LOAD_FROM_REPOSITORY);

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
                     NotifyableImpl *parent)
        : NotifyableImpl(fp, key, name, parent),
          m_cachedKeyValues(this) {}

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
     * The default constructor is private so no one can call it.
     */
    PropertyListImpl()
        : NotifyableImpl(NULL, "", "", NULL),
          m_cachedKeyValues(this)
    {
        throw InvalidMethodException("Someone called the PropertyListImpl "
                                     "default constructor!");
    }

  private:
    /**
     * The cached key-values
     */
    CachedKeyValuesImpl m_cachedKeyValues;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_PROPERTYLISTIMPL_H_ */

