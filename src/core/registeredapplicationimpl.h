/*
 * registeredapplicationimpl.h --
 *
 * Definition of RegisteredApplicationImpl.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_REGISTEREDAPPLICATIONIMPL_H_
#define _CL_REGISTEREDAPPLICATIONIMPL_H_

namespace clusterlib {

/**
 * Implementation of interfaces for a RegisteredApplicationImpl.
 */
class RegisteredApplicationImpl
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
    RegisteredApplicationImpl(FactoryOps *factoryOps);

    /**
     * Virtual destructor.
     */
    virtual ~RegisteredApplicationImpl() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
