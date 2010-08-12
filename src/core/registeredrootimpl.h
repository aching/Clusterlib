/*
 * registeredrootimpl.h --
 *
 * Definition of RegisteredRootImpl.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_REGISTEREDROOTIMPL_H_
#define _CL_REGISTEREDROOTIMPL_H_

namespace clusterlib {

/**
 * Implementation of interfaces for a RegisteredRootImpl.
 */
class RegisteredRootImpl
    : public virtual RegisteredNotifyableImpl
{
  public:
    virtual const std::string &registeredName() const;

    virtual std::string generateKey(const std::string &parentKey,
                                    const std::string &name) const;
    
    virtual bool isValidName(const std::string &name) const;

    virtual boost::shared_ptr<NotifyableImpl> createNotifyable(
        const std::string &notifyableName,
        const std::string &notifyableKey,
        const boost::shared_ptr<NotifyableImpl> &parent,
        FactoryOps &factoryOps) const;

    virtual std::vector<std::string> generateRepositoryList(
        const std::string &notifyableName,
        const std::string &notifyableKey) const;
    
    virtual bool isValidKey(const std::vector<std::string> &components, 
                            int32_t elements = -1);

    virtual bool getObjectFromComponents(
        const std::vector<std::string> &components,
        int32_t elements,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<NotifyableImpl> *pNotifyableSP);

    /**
     * Constructor
     */
    RegisteredRootImpl(FactoryOps *factoryOps)
        : RegisteredNotifyableImpl(factoryOps) {}

    /**
     * Virtual destructor.
     */
    virtual ~RegisteredRootImpl() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
