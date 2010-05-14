/*
 * registerednotifyableimpl.h --
 *
 * Contains the base class for RegisteredNotifyableImpl objects.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_REGISTEREDNOTIFYABLEIMPL_H_
#define _CL_REGISTEREDNOTIFYABLEIMPL_H_

namespace clusterlib
{

/**
 * Interface and partial implementation that must be derived by
 * specific notifyableImpl objects to be registered.
 */
class RegisteredNotifyableImpl
    : public virtual RegisteredNotifyable
{
  public:
    virtual void setSafeNotifyableMap(
        SafeNotifyableMap &safeNotifyableMap);

    virtual SafeNotifyableMap *getSafeNotifyableMap();

    virtual NotifyableImpl *loadNotifyableFromRepository(
        const std::string &notifyableName,
        const std::string &notifyableKey,
        NotifyableImpl *parent);

    virtual void createRepositoryObjects(const std::string &notifyableName,
                                         const std::string &notifyableKey);

    /**
     * Default implementation.  Override if desired.
     *
     * @param name The name to check.
     * @return True if valid, false otherwise.
     */
    virtual bool isValidName(const std::string &name) const;

    virtual bool isValidKey(const std::string &key);

    virtual bool isValidKey(const std::vector<std::string> &components, 
                            int32_t elements = -1) = 0;

    virtual NotifyableImpl *getObjectFromKey(
        const std::string &key, 
        AccessType accessType = LOAD_FROM_REPOSITORY);
    
    /**
     * Constructor.
     */
    RegisteredNotifyableImpl(FactoryOps *factoryOps)
        : mp_f(factoryOps),
          m_safeNotifyableMap(NULL) {}
    
    /**
     * Virtual destructor.
     */
    virtual ~RegisteredNotifyableImpl() {}


    /**
     * Get the associated factory object.
     */
    FactoryOps *getOps() { return mp_f; }

  private:
    /**
     * Get the mutex that makes this object thread-safe.
     *
     * @return Reference to the only mutex in this object.
     */
    Mutex &getLock();

  private:
    /**
     * The associated factory delegate.
     */
    FactoryOps *mp_f;

    /**
     * This is the map that contains these types of objects and is
     * only valid for registered objects.
     */
    SafeNotifyableMap *m_safeNotifyableMap;

    /**
     * Makes this object thread-safe
     */
    Mutex m_mutex;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
