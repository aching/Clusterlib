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

#ifndef	_CL_APPLICATIONIMPL_H_
#define _CL_APPLICATIONIMPL_H_

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
    ApplicationImpl(FactoryOps *fp,
                    const std::string &key, 
                    const std::string &name, 
                    NotifyableImpl *root)
        : NotifyableImpl(fp, key, name, root),
          GroupImpl(fp, key, name, root) {}

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
          GroupImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException("Someone called the ApplicationImpl "
                                     "default constructor!");
    }
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_APPLICATIONIMPL_H_ */
