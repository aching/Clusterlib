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
     * @return true if this node is connected.
     */
    virtual bool isConnected() = 0;

    /*
     * Return the time at which various events happened.
     */
    virtual int64_t getClientStateTime() = 0;
    virtual int64_t getMasterSetStateTime() = 0;
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
     * @param checkFrequency how often to execute the given callback,
     *                       in seconds
     */
    virtual void registerHealthChecker(HealthChecker *healthChecker) = 0;

    /**
     * Unregister the health checker.  This stops the health checker
     * thread and allows any thread to register a health checker on
     * this node again.
     */
    virtual void unregisterHealthChecker() = 0;

    /**
     * Destructor.
     */
    virtual ~Node() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NODE_H_ */
