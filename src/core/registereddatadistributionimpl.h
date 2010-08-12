/*
 * registereddatadistributionimpl.h --
 *
 * Definition of RegisteredDataDistributionImpl.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_REGISTEREDDATADISTRIBUTIONIMPL_H_
#define _CL_REGISTEREDDATADISTRIBUTIONIMPL_H_

namespace clusterlib {

/**
 * Implementation of interfaces for a RegisteredDataDistributionImpl.
 */
class RegisteredDataDistributionImpl
    : public virtual RegisteredNotifyableImpl
{
  public:
    virtual const std::string &registeredName() const;

    virtual std::string generateKey(const std::string &parentKey,
                                    const std::string &name) const;

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
    RegisteredDataDistributionImpl(FactoryOps *factoryOps)
        : RegisteredNotifyableImpl(factoryOps) {}

    /**
     * Virtual destructor.
     */
    virtual ~RegisteredDataDistributionImpl() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
