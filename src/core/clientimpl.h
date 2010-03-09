/*
 * clientimpl.h --
 *
 * Definition of class ClientImpl.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef _CL_CLIENTIMPL_H_
#define _CL_CLIENTIMPL_H_

namespace clusterlib
{

class ClientImpl
    : public virtual Client
{
  public:
    virtual Root *getRoot();

    virtual TimerId registerTimer(TimerEventHandler *tehp,
                                  uint64_t afterMsecs,
                                  ClientData data);

    virtual bool cancelTimer(TimerId id);

    virtual void registerHandler(UserEventHandler *uehp);

    virtual bool cancelHandler(UserEventHandler *cehp);

    virtual void registerJSONRPCResponseHandler(Queue *responseQueue,
                                                Queue *completedQueue);

    virtual bool cancelJSONRPCResponseHandler();

    virtual void registerJSONRPCMethodHandler(
        Queue *recvQueue,
        Queue *completedQueue,
        int32_t completedQueueMaxSize,
        PropertyList *rpcMethodHandlerPropertyList,
        ::json::rpc::JSONRPCManager *rpcManager);

    virtual bool cancelJSONRPCMethodHandler();

    virtual uint64_t fetchAndIncrRequestCounter();

    /*
     * Internal functions not used by outside clients
     */
  public:
    /**
     * Constructor used by the factory.
     */
    ClientImpl(FactoryOps *fp)
        : mp_f(fp),
          m_jsonRPCRequestCounter(0),
          m_jsonRPCResponseHandler(NULL),
          m_jsonRPCMethodHandler(NULL)
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
        m_eventThread.Create(*this, &ClientImpl::consumeUserEvents);
    }

    /**
     * Get the associated factory delegate object.
     *
     * @return a pointer to the factory delegate
     */
    FactoryOps *getOps() { return mp_f; }

    /**
     * Send an event to this client.
     *
     * @param cepp the payload that goes into the queue
     */
    void sendEvent(UserEventPayload *cepp);

    /**
     * Virtual destructor.
     */
    virtual ~ClientImpl();

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
     * Consume user events. This method runs in a separate thread,
     * see m_eventThread below.
     */
    void consumeUserEvents(void *param);

    /**
     * Get the event handlers registry lock.
     *
     * @return a pointer to the mutex
     */
    Mutex *getEventHandlersLock() { return &m_eventHandlersLock; }

    /**
     * Dispatch all handlers registered for this combo of event and
     * Notifyable.
     *
     * @param key the key of the notifyable
     * @param e the event on this notifyable
     */
    void dispatchHandlers(const std::string &key, Event e);

  private:
    /**
     * The factory delegate instance we're using.
     */
    FactoryOps *mp_f;

    /**
     * The blocking queue for delivering notifications
     * to this client.
     */
    UserEventPayloadQueue m_queue;

    /**
     * The thread consuming the events.
     */
    CXXThread<ClientImpl> m_eventThread;

    /**
     * Map of user event handlers.
     */
    EventHandlersMultimap m_eventHandlers;
    /**
     * Map of first-time user event handles.
     */
    std::vector<UserEventHandler *> m_firstTimeEventHandlers;
    /**
     * Protects both m_eventHandlers and m_firstTimeEventHandlers
     */
    Mutex m_eventHandlersLock;

    /**
     * Protect m_jsonRPCRequestCounter
     */
    Mutex m_jsonRPCRequestCounterMutex;
    
    /**
     * Counter to make sure our requests are unique.
     */
    uint64_t m_jsonRPCRequestCounter;

    /**
     * JSONRPCResponseHandler pointer
     */
    JSONRPCResponseHandler *m_jsonRPCResponseHandler;

    /**
     * JSONRPCMethodHandler pointer
     */
    JSONRPCMethodHandler *m_jsonRPCMethodHandler;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENTIMPL_H_ */
