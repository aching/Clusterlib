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
     * Get a map of the available applications in this factory.
     *
     * @return a map of the available applications
     */
    virtual ApplicationMap getApplications() = 0;

    /**
     * Retrieve an application.
     *
     * @param appName non-path name of the application (i.e. llf)
     * @param create create the Application if it doesn't exist
     * @return NULL if the application doesn't exist otherwise the pointer 
     *         to the Application
     */
    virtual Application *getApplication(const std::string &appName,
                                        bool create = false) = 0;

    /**
     * Register a timer handler to be called after
     * a specified delay.
     * 
     * @param tehp pointer to the handler class that is managed by the user
     * @param afterTime milliseconds to wait for the event to be triggered
     */
    virtual TimerId registerTimer(TimerEventHandler *tehp,
                                  uint64_t afterTime,
                                  ClientData data) = 0;

    /**
     * Cancel a previously registered timer.
     *
     * @return true if successful, false otherwise
     */
    virtual bool cancelTimer(TimerId id) = 0;

    /**
     * Register and cancel a cluster event handler. 
     *
     * @param cehp a handler class for events, managed by caller
     */
    virtual void registerHandler(ClusterEventHandler *cehp) = 0;
    /**
     * Cancel a handler for events.
     *
     * @return true if successful, false otherwise
     */
    virtual bool cancelHandler(ClusterEventHandler *cehp) = 0;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENT_H_ */
