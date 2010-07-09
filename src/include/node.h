/*
 * node.h --
 *
 * Definition of class Node; it represents a node in a group in an
 * application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_NODE_H_
#define _CL_NODE_H_

namespace clusterlib
{

/**
 * Definition of class Node
 */
class Node
    : public virtual Notifyable
{
  public:
    /**
     * Used to access the health of the current state.
     */
    static const std::string HEALTH_KEY;

    /**
     * Used to denote the health: good (current and desired state)
     */
    static const std::string HEALTH_GOOD_VALUE;

    /**
     * Used to denote the health: bad (current and desired state)
     */
    static const std::string HEALTH_BAD_VALUE;

    /**
     * Used to access the health set time of the current state.
     */
    static const std::string HEALTH_SET_MSECS_KEY;

    /**
     * Used to access the health set time as a date of the current state.
     */
    static const std::string HEALTH_SET_MSECS_AS_DATE_KEY;

    /**
     * Used to shutdown an active node by setting in a desired state.
     */
    static const std::string ACTIVENODE_SHUTDOWN;

    /**
     * Access the process slot info cached object
     *
     * @return A reference to the cached process slot info.
     */
    virtual CachedProcessSlotInfo &cachedProcessSlotInfo() = 0;

    /** 
     * Get a list of names of all process slots
     * 
     * @return a copy of the list of all process slots.
     */
    virtual NameList getProcessSlotNames() = 0;

    /**
     * Get the named process slot (only if enabled).
     * 
     * @param name the name of the proccess slot
     * @param accessType The mode of access
     * @return NULL if the named process slot does not exist and create
     * == false
     * @throw Exception only if tried to create and couldn't create
     */
    virtual ProcessSlot *getProcessSlot(
        const std::string &name, 
        AccessType accessType = LOAD_FROM_REPOSITORY) = 0;

    /**
     * Destructor.
     */
    virtual ~Node() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NODE_H_ */
