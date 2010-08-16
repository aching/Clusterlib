/*
 * notifyable.h --
 *
 * Contains the base class for notifyable objects.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_NOTIFYABLE_H_
#define _CL_NOTIFYABLE_H_

namespace clusterlib {

/**
 * Interface that must be derived by specific notifyable objects.
 */
class Notifyable
{
  public:
    /**
     * Used to access the PID of the current state.
     */
    static const std::string PID_KEY;

    /**
     * Used to access the notifyable state.
     */
    static const std::string NOTIFYABLE_STATE_KEY;

    /**
     * Used to denote the notifyable state: preparing (current state only)
     */
    static const std::string NOTIFYABLE_STATE_PREPARING_VALUE;

    /**
     * Used to denote the notifyable state: ready (current state only)
     */
    static const std::string NOTIFYABLE_STATE_READY_VALUE;

    /**
     * Used to denote the notifyable state: unavailable (current state only)
     */
    static const std::string NOTIFYABLE_STATE_UNAVAILABLE_VALUE;

    /**
     * Used to denote the notifyable state: unused (current state only)
     */
    static const std::string NOTIFYABLE_STATE_UNUSED_VALUE;

    /**
     * Used to denote the notifyable state: maintaining (current state only)
     */
    static const std::string NOTIFYABLE_STATE_MAINTAINING_VALUE;

    /**
     * Used to denote the notifyable state: maintain (desired state only)
     */
    static const std::string NOTIFYABLE_STATE_MAINTAIN_VALUE;

    /**
     * Used to denote the notifyable state: none (desired state only)
     */
    static const std::string NOTIFYABLE_STATE_NONE_VALUE;

    /**
     * State of the Notifyable.
     */
    enum State {
        READY = 0,
        REMOVED
    };

    /**
     * Compare two Notifyable instances.
     *
     * @param other the Notifyable instance to compare against
     * @return true if the Notifyables are the same
     */
    virtual bool operator==(const Notifyable &other) = 0;

    /**
     * Get the name of the Notifyable.
     * 
     * @return name of the Notifyable
     */
    virtual const std::string &getName() const = 0;

    /**
     * Return the string identifying the represented
     * cluster object.
     *
     * @return key unique key that represents this Notifyable
     */
    virtual const std::string &getKey() const = 0;

    /**
     * Get the parent of this Notifyable (if it exists)
     *
     * @return pointer to parent 
     * @throw Exception if Notifyable is the root
     */
    virtual boost::shared_ptr<Notifyable> getMyParent() const = 0;

    /**
     * Get a list of all the children of this notifyable.
     *
     * @return list of child Notifyable pointers
     */
    virtual NotifyableList getMyChildren() = 0;

    /**
     * Retrieve the application object that this Notifyable is a part of.  
     *
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediatel
     * @param pApplicationSP Pointer to that Application if it exists, 
     *        otherwise NULL.
     * @return True is the method completed prior to the timeout, false 
     *         otherwise
     * @throw Exception if Notifyable is the root
     */
    virtual bool getMyApplicationWaitMsecs(
        int64_t msecTimeout,
        boost::shared_ptr<Application> *pApplicationSP) = 0; 
    
    /**
     * Retrieve the application object that this Notifyable is a part
     * of (wait forever).
     *
     * @return pointer to the Application
     * @throw Exception if Notifyable is the root
     */
    virtual boost::shared_ptr<Application> getMyApplication() = 0; 

    /**
     * Retrieve the group object that this Notifyable is a part of.  
     *
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediatel
     * @param pGroupSP Pointer to that Group if it exists, 
     *                 otherwise NULL.
     * @return True is the method completed prior to the timeout, false 
     *         otherwise
     * @throw Exception if Notifyable is the root
     */
    virtual bool getMyGroupWaitMsecs(
        int64_t msecTimeout,
        boost::shared_ptr<Group> *pGroupSP) = 0; 

    /**
     * Retrieve the group object that this Notifyable is a part of
     * (wait forever).
     *
     * @return pointer to the Group
     * @throw Exception if Notifyable is the root or application
     */
    virtual boost::shared_ptr<Group> getMyGroup() = 0; 

    /**
     * Get a notifyable from a key. 
     *
     * @param key the key that represents a notifyable.
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediatel
     * @param pNotifyableSP Pointer to that notifyable if it exists, 
     *        otherwise NULL.
     * @return True if the operation finished before the timeout
     */
    virtual bool getNotifyableFromKeyWaitMsecs(
        const std::string &key,
        int64_t msecTimeout,
        boost::shared_ptr<Notifyable> *pNotifyableSP) = 0;

    /**
     * Get a notifyable from a key (wait forever). 
     *
     * @param key the key that represents a notifyable.
     * @return Pointer to that notifyable if it exists, 
     *         otherwise NULL.
     */
    virtual boost::shared_ptr<Notifyable> getNotifyableFromKey(
        const std::string &key) = 0;

    /**
     * What state is this Notifyable in?  It is safe to call this even
     * if the Notifyable was removed but still has a valid reference
     * to it.  The state will be READY or REMOVED.  It cannot
     * transition from the REMOVED state to another state.
     *
     * @return the state of the Notifyable
     */
    virtual Notifyable::State getState() const = 0;

    /**
     * Get a list of names of all property lists.
     * 
     * @return a copy of the list of all property lists.
     */
    virtual NameList getPropertyListNames() = 0;

    /**
     * Get the property lists for this object (if it is allowed). If
     * subclasses do not want to allow getPropertyList(), override it
     * and throw a clusterlib exception.  Propertylists can be named and
     * should use the name ClusterlibStrings::DEFAULTPROPERTYLIST if no
     * name is selected.
     * 
     * @param name the name of the PropertyList to create
     * @param accessType The mode of access
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pPropertyListSP Pointer to that pPropertyListSP if it exists, 
     *        otherwise NULL.
     * @return True if the operation finished before the timeout
     * @throw Exception if Notifyable is the root or application
     */
    virtual bool getPropertyListWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<PropertyList> *pPropertyListSP) = 0;

    /**
     * Get the property lists for this object (if it is allowed). If
     * subclasses do not want to allow getPropertyList(), override it
     * and throw a clusterlib exception.  Propertylists can be named and
     * should use the name ClusterlibStrings::DEFAULTPROPERTYLIST if no
     * name is selected.  
     * 
     * @param name the name of the PropertyList to create
     * @param accessType The mode of access
     * @return Pointer to that pPropertyListSP if it exists, otherwise NULL.
     * @throw Exception if Notifyable is the root or application
     */
    virtual boost::shared_ptr<PropertyList> getPropertyList(
        const std::string &name,
        AccessType accessType) = 0;

    /**
     * Get a list of names of all queues.
     * 
     * @return a copy of the list of all queues.
     */
    virtual NameList getQueueNames() = 0;

    /**
     * Get the queue for this object (if it is allowed). If
     * subclasses do not want to allow getQueue(), override it
     * and throw a clusterlib exception.  Queues can be named and
     * should use the name ClusterlibStrings::DEFAULTQUEUE if no
     * name is selected.
     * 
     * @param name the name of the Queue to create
     * @param accessType The mode of access
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pQueueSP Pointer to that pQueueSP if it exists, 
     *        otherwise NULL.
     * @return True if the operation finished before the timeout
     * @throw Exception if Notifyable is the root or application
     */
    virtual bool getQueueWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Queue> *pQueueSP) = 0;

    /**
     * Get the queues for this object (if it is allowed). If
     * subclasses do not want to allow getQueue(), override it
     * and throw a clusterlib exception.  
     * 
     * @param name the name of the queue to create
     * @param accessType The mode of access
     * @return queue pointer or NULL if no queue exists for this 
     * notifyable and create == false
     */
    virtual boost::shared_ptr<Queue> getQueue(const std::string &name,
                                              AccessType accessType) = 0;

    /** 
     * \brief Acquire the clusterlib lock for this Notifyable.
     *
     * Advisory lock.  In order to guarantee that changes to
     * Notifyable objects are ordered, clients must also
     * acquire/release the appropriate locks and work with
     * coordinating clients (that also respect locks).  This call can
     * also lock all children.  Clients must be careful (i.e. have
     * some resource ordering) to ensure that deadlock does not occur.
     * The only time locks are implicitly grabbed by clusterlib is
     * when trying to create/remove an object.  Locks are all
     * specified in ClusterlibStrings (NOTIFYABLE_LOCK,
     * OWNERSHIP_LOCK, and CHILD_LOCK).
     *
     * @param lockName Name of the lock
     * @param distributedLockType Type of lock to access
     * @param acquireChildren lock the children as well?
     * @throw Exception if this Notifyable or its parent no
     * longer exist.
     */
    virtual void acquireLock(const std::string &lockName,
                             DistributedLockType distributedLockType,
                             bool acquireChildren = false) = 0;

    /** 
     * \brief Acquire the clusterlib lock for this Notifyable within a
     * number of msecs.
     *
     * Advisory lock.  In order to guarantee that changes to
     * Notifyable objects are ordered, clients must also
     * acquire/release the appropriate locks and work with
     * coordinating clients (that also respect locks).  This call can
     * also lock all children.  Clients must be careful (i.e. have
     * some resource ordering) to ensure that deadlock does not occur.
     * The only time locks are implicitly grabbed by clusterlib is
     * when trying to create/remove an object.  If the function
     * returns true, then all locks have been acquired.  Otherwise, no
     * locks have been acquired.
     *
     * @param lockName Name of the lock
     * @param distributedLockType Type of lock to access
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param acquireChildren lock the children as well?
     * @return true if the lock was acquired or false if timed out
     */
    virtual bool acquireLockWaitMsecs(const std::string &lockName,
                                      DistributedLockType distributedLockType,
                                      int64_t msecTimeout,
                                      bool acquireChildren = false) = 0;

    /** 
     * \brief Release the clusterlib lock for this Notifyable.
     *
     * Advisory lock.  In order to guarantee that changes to
     * Notifyable objects are ordered, clients must also
     * acquire/release the appropriate locks.  This call can also
     * release all children.  If a client does not release a lock it
     * is held until until the client destroys the factory or a
     * network connection is lost.
     *
     * @param lockName Name of the lock
     * @param releaseChildren release the children as well?
     * @throw Exception if internal state is in consistent 
     */
    virtual void releaseLock(const std::string &lockName,
                             bool releaseChildren = false) = 0;    

    /**
     * Do I have the lock?
     *
     * @param lockName Name of the lock
     * @param pDistributedLockType If set, will be the type of lock held
     * @return true if I have the lock, false otherwise
     */
    virtual bool hasLock(const std::string &lockName,
                         DistributedLockType *pDistributedLockType = NULL) = 0;

    /**
     * Get the current lock information.  This is mainly for
     * debugging, since this information can change at any point.
     *
     * @param lockName Name of the lock
     * @param pId if a valid pointer and a lock holder exists, the id of 
     *        the lock holder
     * @param pDistributedLockType If set, will be the type of lock held
     * @param pMsecs if a valid pointer and a lock holder exists, the msecs
     *        since the epoch when the lock holder tried to get the lock
     * @return true if there is a holder of the lock
     */
    virtual bool getLockInfo(
        const std::string &lockName,
        std::string *pId = NULL,
        DistributedLockType *pDistributedLockType = NULL,
        int64_t *pMsecs = NULL) = 0;

    /**
     * Helps with lock debugging only and should not be used in any
     * appliation protocol.  It specifies the clients and their
     * respective bids that are waiting or own this Notifyable's lock.
     * If children is set, the bids will be searched in any Notifyable
     * that is a child of this Notifyable.  Note that this operation
     * bypasses getting locks to load notifyable information (or else
     * would also run into problems getting lock data).  Therefore,
     * this operation should only be used in a situatution where a
     * deadlock has been reached.
     * 
     * @param lockName Name of the lock to look for (if empty, check all locks)
     * @param children Check for lock bids of children?
     * @return a list of the strings naming the clients and their bids
     */
    virtual NameList getLockBids(const std::string &lockName,
                                 bool children) = 0;

    /**
     * Remove the this notifyable.  This causes the object to be
     * removed in clusterlib.  It will no longer be accessable and all
     * state associated with this object will be removed.
     *
     * @param removeChildren if true, try to remove all children, else
     * try to only remove self.
     * @throw Exception if the Notifyable was already removed, 
     * the Notifyable has children and removeChildren was not set, or the 
     * Notifyable is not allowed to be removed (i.e. root).
     */
    virtual void remove(bool removeChildren = false) = 0;

    /**
     * Access the cached current state of this notifyable
     *
     * @return A reference to the cached current state.
     */
    virtual CachedState &cachedCurrentState() = 0;    

    /**
     * Access the cached desired state of this notifyable
     *
     * @return A reference to the cached desired state.
     */
    virtual CachedState &cachedDesiredState() = 0;    

    /*
     * Destructor.
     */
    virtual ~Notifyable() {};
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NOTIFYABLE_H_ */
