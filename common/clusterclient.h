/*
 * client.h --
 *
 * Include file for client side types. Include this file if you're only writing
 * a pure clusterlib client. If you are creating a server (a node that is in a
 * group inside some app using clusterlib) then you need to include the server-
 * side functionality: server/clusterserver.h.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef _CLUSTERCLIENT_H_
#define _CLUSTERCLIENT_H_

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
    ApplicationMap getApplications();

    /**
     * Retrieve an application.
     *
     * @param appName non-path name of the application (i.e. llf)
     * @param create create the Application if it doesn't exist
     * @return NULL if the application doesn't exist otherwise the pointer 
     *         to the Application
     */
    Application *getApplication(const string &appName,
                                bool create = false);

    /**
     * Register a timer handler to be called after
     * a specified delay.
     * 
     * @param tehp pointer to the handler class that is managed by the user
     * @param afterTime milliseconds to wait for the event to be triggered
     */
    TimerId registerTimer(TimerEventHandler *tehp,
                          uint64_t afterTime,
                          ClientData data);

    /**
     * Cancel a previously registered timer.
     *
     * @return true if successful, false otherwise
     */
    bool cancelTimer(TimerId id);

    /**
     * Register and cancel a cluster event handler. 
     *
     * @param cehp a handler class for events, managed by caller
     */
    void registerHandler(ClusterEventHandler *cehp);
    /**
     * Cancel a handler for events.
     *
     * @return true if successful, false otherwise
     */
    bool cancelHandler(ClusterEventHandler *cehp);

  protected:
    /**
     * Make the factory a friend.
     */
    friend class Factory;

    /**
     * Constructor used by the factory.
     */
    Client(FactoryOps *fp)
        : mp_f(fp)
    {
        /**
         * Empty out the handlers table.
         */
        m_eventHandlers.clear();

        /**
         * Create the thread to dispatch cluster events to
         * specific user program handlers. The clusterlib cache
         * object affected by the event already has been updated.
         */
        m_eventThread.Create(*this, &Client::consumeClusterEvents);
    }

    /**
     * Get the associated factory delegate object.
     */
    FactoryOps *getDelegate() { return mp_f; }

    /**
     * Send an event to this client.
     */

    void sendEvent(ClusterEventPayload *cehp)
    {
        m_queue.put(cehp);
    }

    /**
     * Make the destructor protected so it can only be invoked
     * from derived classes.
     */
    virtual ~Client()
    {
        /**
         * Wait till all events have been handled.
         */
        m_eventThread.Join();
    }

    /**
     * Are we caching the applications fully?
     */
    bool cachingApplications() { return m_cachingApplications; }
    void recacheApplications();

    Mutex *getApplicationMapLock() { return &m_applicationsLock; }

  private:
    /**
     * Make the default constructor private so
     * noone can call it.
     */
    Client()
    {
        throw ClusterException("Someone called the Client "
                               "default constructor!");
    }

    /**
     * Consume cluster events. This method runs in a separate thread,
     * see m_eventThread below.
     */
    void consumeClusterEvents();

    /**
     * Get the event handlers registry lock.
     */
    Mutex *getEventHandlersLock() { return &m_eventHandlersLock; }

    /**
     * Dispatch all handlers registered for this combo of event and
     * Notifyable.
     */
    void dispatchHandlers(Notifyable *np, Event e);

  private:
    /**
     * The factory delegate instance we're using.
     */
    FactoryOps *mp_f;

    /**
     * The blocking queue for delivering notifications
     * to this client.
     */
    ClusterEventPayloadQueue m_queue;

    /**
     * The thread consuming the events.
     */
    CXXThread<Client> m_eventThread;

    /**
     * Map of user event handlers.
     */
    EventHandlersMultimap m_eventHandlers;
    Mutex m_eventHandlersLock;

    /**
     * Map of all applications within this factory.
     */
    ApplicationMap m_applications;
    Mutex m_applicationsLock;

    /**
     * Caching applications fully?
     */
    bool m_cachingApplications;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENT_H_ */
