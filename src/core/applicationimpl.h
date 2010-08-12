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

namespace clusterlib {

/**
 * Definition of class ApplicationImpl.
 */
class ApplicationImpl
    : public virtual Application, 
      public virtual GroupImpl
{
  public:
    virtual bool getMyGroupWaitMsecs(
        int64_t msecTimeout,
        boost::shared_ptr<Group> *pGroupSP)
    {
        throw InvalidMethodException(
            "Application cannot be a part of a Group!");
    }

    /*
     * Internal functions not used by outside clients
     */
  public:
    /**
     * Constructor used by Factory.
     */
    ApplicationImpl(FactoryOps *fp,
                    const std::string &key, 
                    const std::string &name, 
                    boost::shared_ptr<NotifyableImpl> root)
        : NotifyableImpl(fp, key, name, root),
          GroupImpl(fp, key, name, root) {}

    /**
     * Destructor.
     */
    virtual ~ApplicationImpl() {};

    virtual void initializeCachedRepresentation();

  private:
    /**
     * Do not call the default constructor
     */
    ApplicationImpl();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_APPLICATIONIMPL_H_ */
