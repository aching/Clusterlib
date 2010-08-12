/*
 * registeredpropertylistimpl.h --
 *
 * Definition of RegisteredPropertyListImpl.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_REGISTEREDPROPERTYLISTIMPL_H_
#define _CL_REGISTEREDPROPERTYLISTIMPL_H_

namespace clusterlib {

/**
 * Implementation of interfaces for a RegisteredPropertyListImpl.
 */
class RegisteredPropertyListImpl
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

    /**
     * Constructor
     */
    RegisteredPropertyListImpl(FactoryOps *factoryOps)
        : RegisteredNotifyableImpl(factoryOps) {}

    /**
     * Virtual destructor.
     */
    virtual ~RegisteredPropertyListImpl() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
