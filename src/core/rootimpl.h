/*
 * rootimpl.h --
 *
 * Definition of class RootImpl; it represents a set of applications
 * that are part of a clusterlib instance.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_ROOTIMPL_H_
#define _CL_ROOTIMPL_H_

namespace clusterlib
{

/**
 * Definition of class RootImpl.
 */
class RootImpl
    : public virtual Root, 
      public virtual NotifyableImpl
{
  public:
    virtual NameList getApplicationNames();

    virtual Application *getApplication(
        const std::string &appName,
        AccessType accessType = LOAD_FROM_REPOSITORY);

    virtual Notifyable *getNotifyableFromKey(const std::string &key);

    virtual Notifyable *getMyParent() const
    {
        throw InvalidMethodException("RootImpl does not have a parent");
    }
    
    virtual Application *getMyApplication()
    {
        throw InvalidMethodException(
            "RootImpl not is part of an application");
    }

    virtual Group *getMyGroup()
    {
        throw InvalidMethodException("RootImpl is not part of a group");
    }

    virtual PropertyList *getPropertyList(
        std::string name,
        AccessType accessType = LOAD_FROM_REPOSITORY)
    {
        throw InvalidMethodException("RootImpl cannot have properties");
    }

    virtual void remove(bool removeChildren = false) 
    {
        throw InvalidMethodException("RootImpl cannot be removed");
    }

    virtual void releaseRef()
    {
        throw InvalidMethodException("RootImpl cannot be released");
    }

    /*
     * Internal functions not used by outside clients
     */
  public:
    /*
     * Constructor used by Factory.
     */
    RootImpl(FactoryOps *fp,
             const std::string &key,
             const std::string &name)
        : NotifyableImpl(fp, key, name, NULL) {}

    virtual ~RootImpl() {};

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries() 
    {
        throw InvalidMethodException("RootImpl cannot remove its"
                                     " repository entries");
    }

    virtual std::string generateKey(const std::string &parentKey,
                                    const std::string &name) const;

    virtual bool isValidName(const std::string &name) const;

    virtual NotifyableImpl *createNotifyable(const std::string &notifyableName,
                                             const std::string &notifyableKey,
                                             NotifyableImpl *parent,
                                             FactoryOps &factoryOps);

    virtual std::vector<std::string> generateRepositoryList(
        const std::string &notifyableName,
        const std::string &notifyableKey);

  private:
    /*
     * The default constructor is private so no one can call it.
     */
    RootImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException("Someone called the RootImpl "
                                       "default constructor!");
    }

    /**
     * The registered name of this object.
     */
    static std::string m_registeredName;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_ROOTIMPL_H_ */
