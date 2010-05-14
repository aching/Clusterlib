/*
 * registeredqueueimpl.h --
 *
 * Definition of RegisteredQueueImpl.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_REGISTEREDQUEUEIMPL_H_
#define _CL_REGISTEREDQUEUEIMPL_H_

namespace clusterlib
{

/**
 * Implementation of interfaces for a RegisteredQueueImpl.
 */
class RegisteredQueueImpl
    : public virtual RegisteredNotifyableImpl
{
  public:
    virtual const std::string &registeredName() const;

    virtual std::string generateKey(const std::string &parentKey,
                                    const std::string &name) const;

    virtual bool isValidName(const std::string &name) const;

    virtual NotifyableImpl *createNotifyable(
        const std::string &notifyableName,
        const std::string &notifyableKey,
        NotifyableImpl *parent,
        FactoryOps &factoryOps) const;

    virtual std::vector<std::string> generateRepositoryList(
        const std::string &notifyableName,
        const std::string &notifyableKey) const;
    
    virtual bool isValidKey(const std::vector<std::string> &components, 
                            int32_t elements = -1);

    virtual NotifyableImpl *getObjectFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        AccessType accessType = LOAD_FROM_REPOSITORY);

    /**
     * Constructor
     */
    RegisteredQueueImpl(FactoryOps *factoryOps)
        : RegisteredNotifyableImpl(factoryOps) {}

    /**
     * Virtual destructor.
     */
    virtual ~RegisteredQueueImpl() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
