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
        throw ClusterException("RootImpl does not have a parent");
    }
    
    virtual Application *getMyApplication()
    {
        throw ClusterException("RootImpl not is part of an application");
    }

    virtual Group *getMyGroup()
    {
        throw ClusterException("RootImpl is not part of a group");
    }

    virtual Properties *getProperties(bool create = false)
    {
        throw ClusterException("RootImpl cannot have properties");
    }

    /*
     * Internal functions not used by outside clients
     */
  public:
    /*
     * Constructor used by Factory.
     */
    RootImpl(const std::string &name, 
             const std::string &key, 
             FactoryOps *fp)
        : NotifyableImpl(fp, key, name, NULL) {}

    /*
     * Make the destructor private also.
     */
    virtual ~RootImpl() {};

    /*
     * Initialize the cached representation.
     */
    virtual void initializeCachedRepresentation();

  private:
    /*
     * The default constructor is private so no one can call it.
     */
    RootImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw ClusterException("Someone called the RootImpl "
                               "default constructor!");
    }
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_ROOTIMPL_H_ */
