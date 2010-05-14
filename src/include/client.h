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

#ifndef _CL_CLIENT_H_
#define _CL_CLIENT_H_

namespace clusterlib
{

/**
 * This class must be subclassed to define
 * timer event handlers that can be instantiated.
 */
class TimerEventHandler
{
  public:
    /**
     * Constructor.
     */
    TimerEventHandler() {}

    /**
     * Destructor.
     */
    virtual ~TimerEventHandler() {}

    /**
     * Handle the event -- this must be implemented by
     * subclasses.
     */
    virtual void handleTimerEvent(TimerId id, ClientData data) = 0;
};

/*
 * Various event notification constants.
 */

/** No event occured. */
const int32_t EN_NOEVENT =			    0;

/** Notifyable was created */
const int32_t EN_CREATED =			    (1<<0);

/** Notifyable was deleted */
const int32_t EN_DELETED =			    (1<<1);

/** Notifyable current state has changed */
const int32_t EN_CURRENT_STATE_CHANGE =		    (1<<2);

/** Groups in this notifyable changed */
const int32_t EN_GROUPSCHANGE =			    (1<<3);

/** Data distributions in this notifyable changed */
const int32_t EN_DISTSCHANGE =			    (1<<4);

/** Nodes in this notifyable changed */
const int32_t EN_NODESCHANGE =		            (1<<5);

/** Leadership in this group changed */
const int32_t EN_LEADERSHIPCHANGE =		    (1<<6);

/** This node's client state changed */
const int32_t EN_CLIENTSTATECHANGE =		    (1<<7);

/** This node's process slot info changed */
const int32_t EN_PROCESS_SLOT_INFO_CHANGE =         (1<<8);

/** This node's master set state changed */
const int32_t EN_MASTERSTATECHANGE =		    (1<<9);

/** This node's use of process slots has changed */
const int32_t EN_PROCESSSLOTSUSAGECHANGE =	    (1<<10);

/** Process slots in this notifyable changed */
const int32_t EN_PROCESSSLOTSCHANGE =		    (1<<11);

/** This process slot's process info changed */
const int32_t EN_PROCESSSLOTPROCESSINFOCHANGE =	    (1<<12);

/** This process slot's executable arguments changed */
const int32_t EN_PROCESSSLOTEXECARGSCHANGE =	    (1<<13);

/** This process slot's running executable arguments changed */
const int32_t EN_PROCESSSLOTRUNNINGEXECARGSCHANGE = (1<<14);

/** This process slot's PID changed */
const int32_t EN_PROCESSSLOTPIDCHANGE =             (1<<15);

/** This process slot's desired state changed */
const int32_t EN_PROCESSSLOTDESIREDSTATECHANGE =    (1<<16);

/** This process slot's current state changed */
const int32_t EN_PROCESSSLOTCURRENTSTATECHANGE =    (1<<17);

/** This process slot's reservation  changed */
const int32_t EN_PROCESSSLOTRESERVATIONCHANGE =     (1<<18);

/** Shards in this data distribution changed */
const int32_t EN_SHARDSCHANGE =			    (1<<19);

/** Property lists in this notifyable changed */
const int32_t EN_PROPLISTSCHANGE =	       	    (1<<20);

/** This property list values have changed. */
const int32_t EN_PROPLISTVALUESCHANGE =		    (1<<21);

/** Applications in this root changed */
const int32_t EN_APPSCHANGE =			    (1<<22);

/** The lock on this notifyable changed */
const int32_t EN_LOCKNODECHANGE =                   (1<<23);

/** Queues in this notifyable changes */
const int32_t EN_QUEUESCHANGE =                     (1<<24);

/** The children of this queue changed */
const int32_t EN_QUEUECHILDCHANGE =                 (1<<25);

/** Clusterlib has been shutdown */
const int32_t EN_ENDEVENT =                         (1<<26);

/** Notifyable desired state has changed */
const int32_t EN_DESIRED_STATE_CHANGE =		    (1<<27);


/*
 * Interface for user event handler. Must be derived
 * to define specific behavior for handling events.
 */
class UserEventHandler
{
  public:
    /*
     * Constructor.  The initialRun parameter provides a way for a
     * user to run the handler at least once prior to receiving an
     * event that matches the mask.  For example, if a handler should
     * take action when a Node's health changed to "unhealthy", but
     * the handler was registered after the event has passed, the
     * handler wouldn't get run until the next event (possibly never).
     * Instead, the handler could be constructed with initialRun =
     * true to do a first check.  Then, after it was registered,
     * events would cause the handler to run at the appropriate time.
     *
     * @param np the notifyable pointer that is stored
     * @param mask the event mask that will trigger this handler
     * @param cd any pointer of user-information to store
     * @param initialRun if true, generate an event with the mask and the 
     *        notifyable to run the event handler once as soon as it is 
     *        registered by a clusterlib client
     */
    UserEventHandler(Notifyable *np,
                     Event mask,
                     ClientData cd,
                     bool initialRun = false)
        : mp_np(np),
          m_mask(mask),
          m_cd(cd),
          m_initialRun(initialRun) {}

    /**
     * Returns a comma-separated string of the events encoded in the int32_t 
     *
     * @param event the event mask
     * @return the comma-separated string of the encoded events
     */
    static std::string getEventsString(int32_t event)
    {
        std::string encodedEvents;
        if (event & EN_CREATED) { /* 0 */
            encodedEvents.append("EN_CREATED,");
        }
        if (event & EN_DELETED) { /* 1 */
            encodedEvents.append("EN_DELETED,");
        }
        if (event & EN_CURRENT_STATE_CHANGE) { /* 2 */
            encodedEvents.append("EN_CURRENT_STATE_CHANGE,");
        }
        if (event & EN_GROUPSCHANGE) { /* 3 */
            encodedEvents.append("EN_GROUPSCHANGE,");
        }
        if (event & EN_DISTSCHANGE) { /* 4 */
            encodedEvents.append("EN_DISTSCHANGE,");
        }
        if (event & EN_NODESCHANGE) { /* 5 */
            encodedEvents.append("EN_NODESCHANGE,");
        }
        if (event & EN_LEADERSHIPCHANGE) { /* 6 */
            encodedEvents.append("EN_LEADERSHIPCHANGE,");
        }
        if (event & EN_CLIENTSTATECHANGE)  { /* 7 */
            encodedEvents.append("EN_CLIENTSTATECHANGE,");
        }
        if (event & EN_PROCESS_SLOT_INFO_CHANGE) { /* 8 */
            encodedEvents.append("EN_PROCESS_SLOT_CHANGE,");
        }
        if (event & EN_MASTERSTATECHANGE) { /* 9 */
            encodedEvents.append("EN_MASTERSTATECHANGE,");
        }
        if (event & EN_PROCESSSLOTSUSAGECHANGE) { /* 10 */
            encodedEvents.append("EN_PROCESSSLOTSUSAGECHANGE,");
        }
        if (event & EN_PROCESSSLOTSCHANGE) { /* 11 */
            encodedEvents.append("EN_PROCESSSLOTSCHANGE,");
        }
        if (event & EN_PROCESSSLOTPROCESSINFOCHANGE) { /* 12 */
            encodedEvents.append("EN_PROCESSSLOTPROCESSINFOCHANGE,");
        }
        if (event & EN_PROCESSSLOTEXECARGSCHANGE) { /* 13 */
            encodedEvents.append("EN_PROCESSSLOTEXECARGSCHANGE,");
        }
        if (event & EN_PROCESSSLOTRUNNINGEXECARGSCHANGE) { /* 14 */
            encodedEvents.append("EN_PROCESSSLOTRUNNINGEXECARGSCHANGE,");
        }
        if (event & EN_PROCESSSLOTPIDCHANGE) { /* 15 */
            encodedEvents.append("EN_PROCESSSLOTPIDCHANGE,");
        }
        if (event & EN_PROCESSSLOTDESIREDSTATECHANGE) { /* 16 */
            encodedEvents.append("EN_PROCESSSLOTDESIREDSTATECHANGE,");
        }
        if (event & EN_PROCESSSLOTCURRENTSTATECHANGE) { /* 17 */
            encodedEvents.append("EN_PROCESSSLOTCURRENTSTATECHANGE,");
        }
        if (event & EN_PROCESSSLOTRESERVATIONCHANGE) { /* 18 */
            encodedEvents.append("EN_PROCESSSLOTRESERVATIONCHANGE,");
        }
        if (event & EN_SHARDSCHANGE) { /* 19 */
            encodedEvents.append("EN_SHARDSCHANGE,");
        }
        if (event & EN_PROPLISTSCHANGE) { /* 20 */
            encodedEvents.append("EN_PROPLISTSCHANGE,");
        }
        if (event & EN_PROPLISTVALUESCHANGE) { /* 21 */
            encodedEvents.append("EN_PROPLISTVALUESCHANGE,");
        }
        if (event & EN_APPSCHANGE) { /* 22 */
            encodedEvents.append("EN_APPSCHANGE,");
        }
        if (event & EN_LOCKNODECHANGE) { /* 23 */
            encodedEvents.append("EN_LOCKNODECHANGE,");
        }
        if (event & EN_QUEUESCHANGE) { /* 24 */
            encodedEvents.append("EN_QUEUECHILDCHANGE,");
        }
        if (event & EN_QUEUECHILDCHANGE) { /* 25 */
            encodedEvents.append("EN_QUEUECHILDCHANGE,");
        }
        if (event & EN_ENDEVENT) { /* 26 */
            encodedEvents.append("EN_ENDEVENT,");
        }
        if (event & EN_DESIRED_STATE_CHANGE) { /* 27 */
            encodedEvents.append("EN_ENDEVENT,");
        }

        /* Get rid of the last ',' */
        if (!encodedEvents.empty()) {
            encodedEvents.erase(encodedEvents.size() - 1, 1);
        }
        else {
            encodedEvents = "EN_NOEVENT";
        }
        return encodedEvents;
    }

    /*
     * Destructor.
     */
    virtual ~UserEventHandler() {}

    /*
     * Accessors.
     */
    Notifyable *getNotifyable() { return mp_np; }
    Event getMask() { return m_mask; }
    ClientData getClientData() { return m_cd; }
    bool getInitialRun() { return m_initialRun; }

    void setNotifyable(Notifyable *np) { mp_np = np; }
    void setMask(Event e) { m_mask = e; }
    void setClienData(ClientData cd) { m_cd = cd; }

    Event addEvent(Event a) { m_mask |= a; return m_mask; }
    Event removeEvent(Event a) { m_mask &= (~a); return m_mask; }

    /**
     * \brief Handle the event -- this must be implemented by subclasses.
     *
     * @param e the event to be processed.
     */
    virtual void handleUserEvent(Event e) = 0;

    /**
     * \brief Waits until a condition is met.
     *
     * Before calling this, the user program must have called acquireLock,
     * and it must call releaseLock after the call to waitUntilCondition
     * returns. The call to waitUntilCondition blocks until an event is
     * processed by clusterlib that causes the meetsCondition API (see below)
     * to return true. If maxMS > 0 and the wait lasts longer than maxMS
     * milliseconds, or if interruptible == true and the wait was interrupted,
     * then returns false. Otherwise returns true.
     *
     * @param maxMs how many milliseconds to wait. default 0 means forever.
     * @param interruptible is this wait interruptible? default is false.
     */
    bool waitUntilCondition(uint64_t maxMs = 0, bool interruptible = false);

    /**
     * \brief Determines if a condition is met (and waits should end).
     *
     * Determine whether a condition is met. The default implementation
     * always returns true. Intended to be overridden by user programs to
     * implement more complex waits and conditions.
     *
     * @param e the event to check for meeting the condition.
     */
    virtual bool meetsCondition(Event e) { return true; }

    /**
     * \brief User-specified method called to reset the condition.
     *
     * Called when meetsCondition returns true. Intended to be redefined
     * by user programs to clean up application-specific data associated
     * with this condition. The default implementation does nothing.
     */
    virtual void resetCondition() {}

    /**
     * \brief Acquires the lock for waitUntilCondition.
     *
     * Advisory lock. Must be held before waitUntilCondition is
     * called.  Internal clusterlib event system always respects this
     * lock, and relies on clients calling acquireLock in order to
     * guarantee that conditions are checked with a consistent
     * ordering with respect to waitUntilCondition.
     */
    void acquireLock() { m_waitMutex.acquire(); }

    /**
     * \brief Releases the lock acquired by acquireLock.
     *
     * Advisory lock. Must be held before waitUntilCondition is called,
     * and must be released via releaseLock after waitUntilCondition returns.
     */
    void releaseLock() { m_waitMutex.release(); }

  public:
    /**
     * Call the user defined handler, and then deal with conditions
     * & waiting. Intended for use by clusterlib internals.
     *
     * @param e the event to be processed.
     */
    void handleUserEventDelivery(Event e);

  private:
    /**
     * Notify any threads that are waiting for the condition, that it
     * has been met.
     */
    void notifyWaiters() { m_waitCond.signal_all(); }

  private:
    /**
     * The Notifyable this handler is for.
     */
    Notifyable *mp_np;

    /**
     * The events (a mask) that this handler is for.
     */
    Event m_mask;

    /**
     * Arbitrary data to pass to the handler for each
     * event as it is triggered.
     */
    ClientData m_cd;

    /**
     * If set, the first time the client thread sees it, run it.
     * After it is run, only run this handler when a clusterlib event
     * matches.
     */
    bool m_initialRun;

    /*
     * Conditional for use with m_waitCond to synchronize handler.
     */
    Cond m_waitCond;

    /**
     * Mutex for use with m_waitCond to synchronize handler.
     */
    Mutex m_waitMutex;
};

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
     * @param afterMsecs milliseconds to wait for the event to be triggered
     * @param data the pointer to user-defined data that is given back when 
     *        handling the event.
     */
    virtual TimerId registerTimer(TimerEventHandler *tehp,
                                  uint64_t afterMsecs,
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

    /**
     * Virtual destructor.
     */
    virtual ~Client() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERCLIENT_H_ */
