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
        INIT,
        READY,
        REMOVED
    };

    /*
     * Compare two Notifyable instances.
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
     * @throw ClusterException if Notifyable is the root
     */
    virtual Notifyable *getMyParent() const = 0;
    
    /**
     * Retrieve the application object that this Notifyable is a part of.  
     *
     * @return pointer to the Application
     * @throw ClusterException if Notifyable is the root
     */
    virtual Application *getMyApplication() = 0; 

    /**
     * Retrieve the group object that this Notifyable is a part of.
     *
     * @return pointer to the Group
     * @throw ClusterException if Notifyable is the root or application
     */
    virtual Group *getMyGroup() = 0; 

    /**
     * What state is this Notifyable in?
     *
     * @return the state of the Notifyable
     */
    virtual Notifyable::State getState() = 0;

    /**
     * Get the properties for this object (if it is allowed). If
     * subclasses do not want to allow getProperties(), override it
     * and throw a clusterlib exception.
     * 
     * @param create create the properties if doesn't exist?
     * @return properties pointer or NULL if no properties exists for this 
     * notifyable and create == false
     * @throw ClusterException if Notifyable is the root or application
     */
    virtual Properties *getProperties(bool create = false) = 0;

    /** 
     * \brief Get the clusterlib lock for this Notifyable.
     *
     * Advisory lock.  Internal clusterlib event system will always
     * respect this lock.  In order to guarantee that changes to
     * Notifyable objects are ordered and atomic, clients must also
     * acquire the appropirate locks.
     *
     * @throw ClusterException if this Notifyable or its parent no
     * longer exist.
     *
     */
    virtual void acquireLock() = 0;

    /** 
     * \brief Release the clusterlib lock for this Notifyable.
     *
     * Advisory lock.  Internal clusterlib event system will always
     * respect this lock.  In order to guarantee that changes to
     * Notifyable objects are ordered and atomic, clients must also
     * acquire the appropirate locks.
     *
     * @throw ClusterException if internal state is in consistent 
     */
    virtual void releaseLock() = 0;    

    /**
     * NOT OPERATIONAL - Do not use!!!
     *
     * Remove the this notifyable.  This causes the object to be
     * removed in clusterlib.  It will no longer be accessable and all
     * state associated with this object will be removed.
     *
     * @param removeChildren if true, try to remove all children, else
     * try to only remove self.
     * @throw ClusterException if the Notifyable was already removed, 
     * the Notifyable has children and removeChildren was not set, or the 
     * Notifyable is not allowed to be removed (i.e. root).
     */
    virtual void remove(bool removeChildren) = 0;

    /*
     * Destructor.
     */
    virtual ~Notifyable() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
