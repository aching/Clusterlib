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

namespace clusterlib
{

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
    virtual Notifyable *getMyParent() const = 0;

    /**
     * Get a list of all the children of this notifyable.
     *
     * @return list of child Notifyable pointers
     */
    virtual NotifyableList getMyChildren() = 0;
    
    /**
     * Retrieve the application object that this Notifyable is a part of.  
     *
     * @return pointer to the Application
     * @throw Exception if Notifyable is the root
     */
    virtual Application *getMyApplication() = 0; 

    /**
     * Retrieve the group object that this Notifyable is a part of.
     *
     * @return pointer to the Group
     * @throw Exception if Notifyable is the root or application
     */
    virtual Group *getMyGroup() = 0; 

    /**
     * Get a notifyable from a key. 
     *
     * @param key the key that represents a notifyable.
     * @return a pointer to that notifyable if it exists, otherwise NULL.
     */
    virtual Notifyable *getNotifyableFromKey(const std::string &key) = 0;

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
     * will use the name ClusterlibStrings::DEFAULTPROPERTYLIST if no
     * name is selected.
     * 
     * @param name the name of the PropertyList to create
     * @param accessType The mode of access
     * @return PropertyList pointer or NULL if no PropertyList exists for this 
     * notifyable and create == false
     * @throw Exception if Notifyable is the root or application
     */
    virtual PropertyList *getPropertyList(
        const std::string &name = 
        ClusterlibStrings::DEFAULTPROPERTYLIST, 
        AccessType accessType = LOAD_FROM_REPOSITORY) = 0;

    /**
     * Get a list of names of all queues.
     * 
     * @return a copy of the list of all queues.
     */
    virtual NameList getQueueNames() = 0;

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
    virtual Queue *getQueue(const std::string &name,
                            AccessType accessType = LOAD_FROM_REPOSITORY) = 0;

    /**
     * Get the reference count of this cachec representation of a
     * Notifyable.  Useful for debugging, since it cannot be
     * manipulated directly.
     *
     * @return the reference count
     */
    virtual int32_t getRefCount() = 0;

    /**
     * Any get*() (i.e. getProperties() or getNode()) increments a
     * reference count on a cached Notifyable.  If the user wants to
     * let clusterlib know that it will no longer access the
     * Notifyable * that refers to a cached Notifyable, it should call
     * this function to decrement the reference count.  Once the
     * reference count goes to 0, the cached Notifyable will be
     * removed from the clusterlib cache.  This function does not
     * effect whether the cached object is removed from the
     * repository.  It is also not required if a user has sufficient
     * memory to hold all cached objects (current and deleted
     * combined).
     */
    virtual void releaseRef() = 0;

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
     * when trying to create/remove an object.
     *
     * @param acquireChildren lock the children as well?
     * @throw Exception if this Notifyable or its parent no
     * longer exist.
     */
    virtual void acquireLock(bool acquireChildren = false) = 0;

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
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param acquireChildren lock the children as well?
     * @return true if the lock was acquired or false if timed out
     */
    virtual bool acquireLockWaitMsecs(int64_t msecTimeout,
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
     * @param releaseChildren release the children as well?
     * @throw Exception if internal state is in consistent 
     */
    virtual void releaseLock(bool releaseChildren = false) = 0;    

    /**
     * Do I have the lock?
     *
     * @return true if I have the lock, false otherwise
     */
    virtual bool hasLock() = 0;

    /**
     * Get the current lock information.  This is mainly for
     * debugging, since this information can change at any point.
     *
     * @param id if a valid pointer and a lock holder exists, the id of 
     *        the lock holder
     * @param msecs if a valid pointer and a lock holder exists, the msecs
     *        since the epoch when the lock holder tried to get the lock
     * @return true if there is a holder of the lock
     */
    virtual bool getLockInfo(std::string *id = NULL, 
                             int64_t *msecs = NULL) = 0;

    /**
     * Helps with lock debugging only and should not be used in any
     * appliation protocol.  It specifies the clients and their
     * respective bids that are waiting or own this Notifyable's lock.
     * If children is set, the bids will be searched in any Notifyable
     * that is a child of this Notifyable.  Note that this operation
     * bypasses getting locks to load notifyable information (or else
     * would also run into problems getting lock data).  Therefore,
     * this operation should only be used in a situatution where a
     * deadlock has been reached.  Using it might cause the thread who
     * called it to Crash if the notifyable being loaded is being
     * created/deleted at the same time.
     * 
     * @return a list of the strings naming the clients and their bids
     */
    virtual NameList getLockBids(bool children) = 0;

    /** 
     * \brief Acquire the ownership for this Notifyable.
     *
     * This is a user-defined ownership.  It could represent a the
     * physical node taking control of a clusterlib node or perhaps a
     * process that is the "leader" of a group.
     *
     * @throw Exception if this Notifyable or its parent no
     * longer exist.
     */
    virtual void acquireOwnership() = 0;

    /** 
     * \brief Acquire the ownership of this Notifyable within a number
     * of msecs.
     *
     * This is a user-defined ownership.  It could represent a the
     * physical node taking control of a clusterlib node or perhaps a
     * process that is the "leader" of a group.
     *
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @return true if the lock was acquired or false if timed out
     */
    virtual bool acquireOwnershipWaitMsecs(int64_t msecTimeout) = 0;

    /** 
     * \brief Give up ownership of this Notifyable.
     *
     * This is a user-defined ownership.  It could represent a the
     * physical node taking control of a clusterlib node or perhaps a
     * process that is the "leader" of a group.
     */
    virtual void releaseOwnership() = 0;

    /**
     * Do I have ownership of this notifyable?
     *
     * @return true if I have ownership, false otherwise
     */
    virtual bool hasOwnership() = 0;

    /**
     * Get the current ownership information.  This is mainly for
     * debugging, since this information can change at any point.
     *
     * @param id if a valid pointer and an owner exists, the id of 
     *        the owner
     * @param msecs if a valid pointer and an owner exists, the msecs
     *        since the epoch when the owner tried to become the owner
     * @return true if there is an owner
     */
    virtual bool getOwnershipInfo(std::string *id = NULL, 
                                  int64_t *msecs = NULL) = 0;

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

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NOTIFYABLE_H_ */
