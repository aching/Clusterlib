/*
 * clientimpl.h --
 *
 * Definition of class ClientImpl.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef _CLIENTIMPL_H_
#define _CLIENTIMPL_H_

namespace clusterlib
{

class ClientImpl
    : public virtual Client
{
  public:
    /**
     * Get the root node that contains all applications and can be
     * used for registering event handlers on.
     * 
     * @return the root node
     */
    virtual Root *getRoot();

    /**
     * Register a timer handler to be called after
     * a specified delay.
     * 
     * @param tehp pointer to the handler class that is managed by the user
     * @param afterTime milliseconds to wait for the event to be triggered
     */
    virtual TimerId registerTimer(TimerEventHandler *tehp,
                                  uint64_t afterTime,
                                  ClientData data);

    /**
     * Cancel a previously registered timer.
     *
     * @return true if successful, false otherwise
     */
    virtual bool cancelTimer(TimerId id);

    /**
     * Register and cancel a cluster event handler. 
     *
     * @param cehp a handler class for events, managed by caller
     */
    virtual void registerHandler(ClusterEventHandler *cehp);

    /**
     * Cancel a handler for events.
     *
     * @return true if successful, false otherwise
     */
    virtual bool cancelHandler(ClusterEventHandler *cehp);

    /*
     * Internal functions not used by outside clients
     */
  public:
    /**
     * Constructor used by the factory.
     */
    ClientImpl(FactoryOps *fp)
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
        m_eventThread.Create(*this, &ClientImpl::consumeClusterEvents);
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
    virtual ~ClientImpl()
    {
        /**
         * Wait till all events have been handled.
         */
        m_eventThread.Join();
    }

  private:
    /**
     * Make the default constructor private so
     * noone can call it.
     */
    ClientImpl()
    {
        throw InvalidMethodException("Someone called the ClientImpl "
                                       "default constructor!");
    }

  private:
    /**
     * Consume cluster events. This method runs in a separate thread,
     * see m_eventThread below.
     */
    void consumeClusterEvents(void *param);

    /**
     * Get the event handlers registry lock.
     */
    Mutex *getEventHandlersLock() { return &m_eventHandlersLock; }

    /**
     * Dispatch all handlers registered for this combo of event and
     * Notifyable.
     */
    void dispatchHandlers(Notifyable *np, Event e);

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
    CXXThread<ClientImpl> m_eventThread;

    /**
     * Map of user event handlers.
     */
    EventHandlersMultimap m_eventHandlers;
    Mutex m_eventHandlersLock;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENTIMPL_H_ */
