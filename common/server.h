/*
 * server.h --
 *
 * Include file for server side types. Include this file if you are writing
 * an implementation of an application that is managed by clusterlib.
 *
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERSERVER_H_
#define	_CLUSTERSERVER_H_

namespace clusterlib
{

class Server
    : public virtual Client
{
  public:
    /**
     * Retrieve the node object for "my" node.
     *
     * @return the Node * for "my" node.
     */
    virtual Node *getMyNode() = 0;

    /**
     * Is this server managed?
     *
     * @return true if the server is managed.
     */
    virtual bool isManaged() = 0;

    /**
     * \brief Registers a function that checks internal health of
     * the caller application. 
     * The given function will be called asynchronously by the cluster API
     * and will provide feedback back to the cluster.
     * 
     * @param healthChecker the callback to be used when checking for
     *                      health; if <code>NULL</code> the health
     *                      monitoring is disabled
     * @param checkFrequency how often to execute the given callback,
     *                       in seconds
     */
    virtual void registerHealthChecker(HealthChecker *healthChecker) = 0;
    
    /**
     * \brief Retrieve the current number of seconds to wait till running
     * the health check again.
     */
    virtual int32_t getHeartBeatPeriod() = 0;
    virtual int32_t getUnhealthyHeartBeatPeriod() = 0;

    /**
     * \brief Enables or disables the health checking and 
     * notifies the worker thread.
     * 
     * @param enabled whether to enable the health checking
     */
    virtual void enableHealthChecking(bool enabled) = 0;

    /**
     * \brief Participate in the leadership election protocol
     * for the containing group.
     *
     * @return true if this server became the leader of its group.
     */
    virtual bool tryToBecomeLeader() = 0;

    /**
     * \brief Am I the leader of my group?
     *
     * @return true if this server is the leader of its group.
     */
    virtual bool amITheLeader() = 0;

    /**
     * \brief Give up leadership of my group.
     */
    virtual void giveUpLeadership() = 0;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERSERVER_H_ */
