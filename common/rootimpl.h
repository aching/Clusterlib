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

#ifndef	_ROOTIMPL_H_
#define _ROOTIMPL_H_

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

    virtual Application *getApplication(const std::string &appName,
                                        bool create = false);

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

    virtual Properties *getProperties(bool create = false)
    {
        throw InvalidMethodException("RootImpl cannot have properties");
    }

    virtual void acquireLock(bool acquireChildren = 0)
    {
        throw InvalidMethodException("RootImpl cannot acquire locks");
    }

    virtual void releaseLock(bool releaseChildren = 0)
    {
        throw InvalidMethodException("RootImpl cannot release locks");
    }

    virtual void remove(bool removeChildren = 0) 
    {
        throw InvalidMethodException("RootImpl cannot be removed");
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

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries() 
    {
        throw InvalidMethodException("RootImpl cannot remove its"
                                       " repository entries");
    }

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
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_ROOTIMPL_H_ */
