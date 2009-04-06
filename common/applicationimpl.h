/*
 * applicationimpl.h --
 *
 * Definition of class ApplicationImpl; it represents a set of groups
 * of nodes that together form a clusterlib application.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_APPLICATIONIMPL_H_
#define _APPLICATIONIMPL_H_

namespace clusterlib
{

/**
 * Definition of class ApplicationImpl.
 */
class ApplicationImpl
    : public virtual Application, 
      public virtual GroupImpl
{
  public:

    virtual Group *getMyGroup();

    /*
     * Internal functions not used by outside clients
     */
  public:
    /*
     * Constructor used by Factory.
     */
    ApplicationImpl(const std::string &name, 
                    const std::string &key, 
                    FactoryOps *fp,
                    NotifyableImpl *root)
        : NotifyableImpl(fp, key, name, root),
          GroupImpl(name, key, fp, root) {}

    /*
     * Destructor.
     */
    virtual ~ApplicationImpl() {};

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries();

  private:
    /*
     * The default constructor is private so no one can call it.
     */
    ApplicationImpl()
        : NotifyableImpl(NULL, "", "", NULL),
          GroupImpl("", "", NULL, NULL)
    {
        throw ClusterException("Someone called the ApplicationImpl "
                               "default constructor!");
    }
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_APPLICATIONIMPL_H_ */
