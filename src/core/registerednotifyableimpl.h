/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_REGISTEREDNOTIFYABLEIMPL_H_
#define _CL_REGISTEREDNOTIFYABLEIMPL_H_

namespace clusterlib {

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

    virtual boost::shared_ptr<NotifyableImpl> loadNotifyableFromRepository(
        const std::string &notifyableName,
        const std::string &notifyableKey,
        const boost::shared_ptr<NotifyableImpl> &parentSP);

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

    virtual bool getObjectFromKey(
        const std::string &key, 
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<NotifyableImpl> *pNotifyableSP);

    /**
     * Default implementation to get this object.  Should be overriden
     * if necessary for special objects (i.e. RegisteredRootImpl or
     * RegisteredApplicationImpl).
     *
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @param accessType The access permission to get this object
     * @param msecTimeout Msecs to wait for locks (-1 for wait forever, 0 for
     *        no waiting)
     * @param pNotifyableSP NULL if cannot be found, else the object pointer
     * @return True if operation completed within the msecTimeout, 
     *         false otherwise
     */
    virtual bool getObjectFromComponents(
        const std::vector<std::string> &components, 
        int32_t elements,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<NotifyableImpl> *pNotifyableSP);

    /**
     * Get the possible registered parent name vector.  Default
     * implementation can be overriden if desired in subclasses.
     *
     * @return Vector of registered parent names for this object.
     */
    virtual const std::vector<std::string> &getRegisteredParentNameVec() const;

    /**
     * Set the parent name vec.
     */
    virtual void setRegisteredParentNameVec(
        const std::vector<std::string> &registeredParentNameVec);
    
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
     * Get the read-write lock that makes this object thread-safe.
     *
     * @return Reference to the only read-write lock in this object.
     */
    const RdWrLock &getRdWrLock() const;

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
    RdWrLock m_rdWrLock;

    /**
     * Possible parent names allowed for this object.
     */
    std::vector<std::string> m_registeredParentNameVec;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_REGISTEREDNOTIFYABLE_H_ */
