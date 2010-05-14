/*
 * registerednotifyable.h --
 *
 * Contains the base class for RegisteredNotifyable objects.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_REGISTEREDNOTIFYABLE_H_
#define _CL_REGISTEREDNOTIFYABLE_H_

namespace clusterlib
{

/**
 * Interface that must be derived by specific notifyable objects to be
 * registered.
 */
class RegisteredNotifyable
{
  public:
    /**
     * (Do not use unless a registered object) Get appropriate
     * SafeNotifyableMap from the registered object.
     *
     * @return a reference to the map that contains these types of objects
     */
    virtual void setSafeNotifyableMap(
        SafeNotifyableMap &safeNotifyableMap) = 0;

    /**
     * (Do not use unless a registered object) Get appropriate
     * SafeNotifyableMap from the registered object.
     *
     * @return a pointer to the map that contains these types of objects
     */
    virtual SafeNotifyableMap *getSafeNotifyableMap() = 0;

    /**
     * For objects to be registered, they need a unique name.
     *
     * @return the object's unique name
     */
    virtual const std::string &registeredName() const = 0;

    /**
     * Makes sure all the zknodes of an object exists and then
     * allocates a new notifyable from existing repository data and
     * returns it to the user.
     *
     * @param notifyableName the name of the notifyable to load objects for
     * @param notifyableKey the key of the notifyable to load objects for
     * @param parent the parent of the new notifyable
     * @return pointer to the new notifyable or NULL if couldn't be found
     */
    virtual NotifyableImpl *loadNotifyableFromRepository(
        const std::string &notifyableName,
        const std::string &notifyableKey,
        NotifyableImpl *parent) = 0;

    /**
     * Creates the necessary zknodes in the repository.  This is part
     * of creating notifyable object.
     *
     * @param notifyableName the name of the notifyable to create objects for
     * @param notifyableKey the key of the notifyable to create objects for
     */
    virtual void createRepositoryObjects(const std::string &notifyableName,
                                         const std::string &notifyableKey) = 0;

    /**
     * Creates the full key from a name.
     *
     * @param parentKey the parent key (if any)
     * @param name the name of the notifyable
     * @return the full key of the notifyable
     */
    virtual std::string generateKey(const std::string &parentKey,
                                    const std::string &name) const = 0;

    /**
     * Check the name of the notifyable for valid names.
     */
    virtual bool isValidName(const std::string &name) const = 0;

    /**
     * (Needs to be thread-safe) Allocate a new notifyable from
     * existing repository data and return it to the user.
     *
     * @param notifyableName the name of the notifyable to load objects for
     * @param notifyableKey the key of the notifyable to load objects for
     * @param parent the parent of the new notifyable
     * @param factoryOps reference to the FactoryOps object
     * @return pointer to the new notifyable or NULL if couldn't be found
     */
    virtual NotifyableImpl *createNotifyable(
        const std::string &notifyableName,
        const std::string &notifyableKey,
        NotifyableImpl *parent,
        FactoryOps &factoryOps) const = 0;

    /**
     * Name the Zookeeper nodes that are required to be created for
     * this object.
     *
     * @param notifyableName the name of the notifyable to create objects for
     * @param notifyableKey the key of the notifyable to create objects for
     * @return a vector of all the zookeeper node names in order of 
     *         creation/checking
     */
    virtual std::vector<std::string> generateRepositoryList(
        const std::string &notifyableName,
        const std::string &notifyableKey) const = 0;

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key for this object (not if an actual object exists for that key).
     * 
     * @param key A key to test if it matches this RegisteredNotifyable
     * @return true if key is valid, false if not valid
     */
    virtual bool isValidKey(const std::string &key) = 0;

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key for this object (not if an actual object exists for that key).
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    virtual bool isValidKey(const std::vector<std::string> &components, 
                            int32_t elements = -1) = 0;

    /**
     * Try to get this object type represented exactly by this key.
     *
     * @param key should represent this object
     * @param accessType The access permission to get this object
     * @return NULL if cannot be found, else the NotifyableImpl *
     */
    virtual NotifyableImpl *getObjectFromKey(
        const std::string &key, 
        AccessType accessType = LOAD_FROM_REPOSITORY) = 0;

    /**
     * Try to get this object type represented exactly by these components.
     *
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @param accessType The access permission to get this object
     * @return NULL if cannot be found, else the object pointer
     */
    virtual NotifyableImpl *getObjectFromComponents(
        const std::vector<std::string> &components,
        int32_t elements = -1, 
        AccessType accessType = LOAD_FROM_REPOSITORY) = 0;
    
    /**
     * Virtual destructor.
     */
    virtual ~RegisteredNotifyable() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
