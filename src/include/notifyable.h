/*
 * notifyable.h --
 *
 * Contains the base class for notifyable objects.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_NOTIFYABLE_H_
#define _NOTIFYABLE_H_

namespace clusterlib
{

/**
 * Interface that must be derived by specific notifyable objects.
 */
class Notifyable
{
  public:
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

#if TO_BE_IMPLEMENTED_IF_NECESSARY
    /**
     * Get the application name of this notifyable
     *
     * @return application this notifyable belongs to
     */
    virtual std::string getMyApplicationName() const = 0;

    /**
     * Get the group name of this notifyable
     *
     * @return group this notifyable belongs to
     */
    virtual std::string getMyGroupName() const = 0;
#endif

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
     * What state is this Notifyable in?  It is safe to call this even
     * if the Notifyable was removed.
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
     * @param name the name of the properties to create
     * @param create create the propertyList if doesn't exist?
     * @return properties pointer or NULL if no properties exists for this 
     * notifyable and create == false
     * @throw Exception if Notifyable is the root or application
     */
    virtual PropertyList *getPropertyList(
        const std::string &name = 
        ClusterlibStrings::DEFAULTPROPERTYLIST, 
        bool create = false) = 0;

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
     *
     */
    virtual void acquireLock(bool acquireChildren = 0) = 0;

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
     *
     * @param releaseChildren release the children as well?
     * @throw Exception if internal state is in consistent 
     */
    virtual void releaseLock(bool releaseChildren = 0) = 0;    

    /**
     * Do I have the lock?
     *
     * @return true is I have the lock, false otherwise
     */
    virtual bool hasLock() = 0;

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
    virtual void remove(bool removeChildren = 0) = 0;

    /*
     * Destructor.
     */
    virtual ~Notifyable() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
