/*
 * client.h --
 *
 * Include file for client side types. Include this file if you're only writing
 * a pure clusterlib client. If you are creating a server (a node that is in a
 * group inside some app using clusterlib) then you need to include the server-
 * side functionality: server.h.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef _CLIENT_H_
#define _CLIENT_H_

namespace clusterlib
{

class Client
{
  public:
    /**
     * \brief Get the root node that contains all applications and can be
     * used for registering event handlers on.
     * 
     * @return the root node
     */
    virtual Root *getRoot() = 0;

    /**
     * \brief Register a timer handler to be called after a specified delay.
     * 
     * @param tehp pointer to the handler class that is managed by the user
     * @param afterTime milliseconds to wait for the event to be triggered
     */
    virtual TimerId registerTimer(TimerEventHandler *tehp,
                                  uint64_t afterTime,
                                  ClientData data) = 0;

    /**
     * \brief Cancel a previously registered timer.
     *
     * @return true if successful, false otherwise
     */
    virtual bool cancelTimer(TimerId id) = 0;

    /**
     * Register and cancel a cluster event handler. 
     *
     * @param uehp an instance of a handler class, managed by caller
     */
    virtual void registerHandler(UserEventHandler *uehp) = 0;

    /**
     * \brief Cancel a handler for events.
     *
     * @return true if successful, false otherwise
     */
    virtual bool cancelHandler(UserEventHandler *uehp) = 0;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENT_H_ */
