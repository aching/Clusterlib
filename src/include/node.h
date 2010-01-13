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

#ifndef	_NODE_H_
#define _NODE_H_

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
     * Get the client-state of this node.
     *
     * @return a string representing the client state for this
     * node.
     */
    virtual std::string getClientState() = 0;
    
    /**
     * Get the master-set state of this node.
     *
     * @return an int32 value representing the state set by the
     * master for this node.
     */
    virtual int32_t getMasterSetState() = 0;

    /**
     * Is this node connected?
     *
     * @param id if a valid pointer, the id is set if connected
     * @param msecs if a valid pointer, the id is set if connected
     * @return true if this node is connected.
     */
    virtual bool isConnected(std::string *id = NULL, 
                             int64_t *msecs = NULL) = 0;

    /**
     * Try to make this node connected
     *
     * @param force if true, replace the old one if it exists.
     */
    virtual bool initializeConnection(bool force = false) = 0;
    
    /**
     * Return the time at the client state was set.
     */
    virtual int64_t getClientStateTime() = 0;
 
    /**
     * Return the time at the master state was set.
     */
    virtual int64_t getMasterSetStateTime() = 0;

    /**
     * Return the time at the connection happened.
     */
    virtual int64_t getConnectionTime() = 0;

    /**
     * Is this node healthy?
     * 
     * @return true if healthy, false if not
     */
    virtual bool isHealthy() = 0;

    /**
     * \brief Registers a function that checks internal health of
     * the caller application. 
     *
     * The given function will be called asynchronously by the cluster
     * API and will provide feedback back to the cluster.  Can not be
     * called if there is always a healthChecker that is running (will
     * throw).  A thread is started with this health checker
     * immediately.
     * 
     * @param healthChecker the callback to be used when checking for
     *                      health; if <code>NULL</code> the health
     *                      monitoring is disabled
     * TODO: Add --> checkFrequency how often to execute the given 
     *               callback in milliseconds
     */
    virtual void registerHealthChecker(HealthChecker *healthChecker) = 0;

    /**
     * Unregister the health checker.  This stops the health checker
     * thread and allows any thread to register a health checker on
     * this node again.
     */
    virtual void unregisterHealthChecker() = 0;

    /**
     * Set whether process slots are to be used (an active
     * clusterlib node process is running).
     */
    virtual void setUseProcessSlots(bool use) = 0;
    
    /**
     * get whether process slots to be used (an active
     * clusterlib node process is running).
     */
    virtual bool getUseProcessSlots() = 0;

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
     * @param create create the process slot if doesn't exist
     * @return NULL if the named process slot does not exist and create
     * == false
     * @throw Exception only if tried to create and couldn't create
     */
    virtual ProcessSlot *getProcessSlot(const std::string &name, 
                                        bool create = false) = 0;

    /**
     * Get the maximum number of process slots on this node.
     *
     * @return the maximum number of process slots
     */
    virtual int32_t getMaxProcessSlots() = 0;

    /**
     * Set the maximum number of process slots on this node.
     *
     * @param maxProcessSlots the maximum number of process slots
     */
    virtual void setMaxProcessSlots(int32_t maxProcessSlots) = 0;

    /**
     * Destructor.
     */
    virtual ~Node() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NODE_H_ */
