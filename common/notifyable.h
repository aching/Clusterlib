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
     * Is this notifyable "ready"? (according to the
     * ready protocol)
     *
     * @return true if this Notifyable is ready for use
     */
    virtual bool isReady() const = 0;

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

    /*
     * Destructor.
     */
    virtual ~Notifyable() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLE_H_ */
