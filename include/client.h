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
const int32_t EN_NOEVENT =			0;

/** Notifyable was created */
const int32_t EN_CREATED =			(1<<0);

/** Notifyable was deleted */
const int32_t EN_DELETED =			(1<<1);

/** Notifyable state has changed */
const int32_t EN_STATECHANGE =			(1<<2);

/** Groups in this notifyable changed */
const int32_t EN_GROUPSCHANGE =			(1<<3);

/** Data distributions in this notifyable changed */
const int32_t EN_DISTSCHANGE =			(1<<4);

/** Nodes in this notifyable changed */
const int32_t EN_NODESCHANGE =		        (1<<5);

/** Leadership in this group changed */
const int32_t EN_LEADERSHIPCHANGE =		(1<<6);

/** This node's client state changed */
const int32_t EN_CLIENTSTATECHANGE =		(1<<7);

/** This node's connectivity changed */
const int32_t EN_CONNECTEDCHANGE =		(1<<10);

/** This node's master set state changed */
const int32_t EN_MASTERSTATECHANGE =		(1<<11);

/** Shards in this data distribution changed */
const int32_t EN_SHARDSCHANGE =			(1<<12);

/** Properties objects in this notifyable changed */
const int32_t EN_PROPSCHANGE =			(1<<13);

/** This properties has changed. */
const int32_t EN_PROPSVALCHANGE =		(1<<14);

/** Applications in this root changed */
const int32_t EN_APPSCHANGE =			(1<<15);

/** The lock on this notifyable changed */
const int32_t EN_LOCKNODECHANGE =               (1<<16);

/** Clusterlib has been shutdown */
const int32_t EN_ENDEVENT =                     (1<<17);

/*
 * Interface for user event handler. Must be derived
 * to define specific behavior for handling events.
 */
class UserEventHandler
{
  public:
    /*
     * Constructor.
     */
    UserEventHandler(Notifyable *np,
                     Event mask,
                     ClientData cd)
        : mp_np(np),
          m_mask(mask),
          m_cd(cd) {}

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

    void setNotifyable(Notifyable *np) { mp_np = np; }
    void setMask(Event e) { m_mask = e; }
    void setClienData(ClientData cd) { m_cd = cd; }

    Event addEvent(Event a) { m_mask |= a; return m_mask; }
    Event removeEvent(Event a) { m_mask &= (~a); return m_mask; }

    /**
     * Call the user defined handler, and then deal with conditions
     * & waiting. Intended for use by clusterlib internals.
     */
    void handleUserEventDelivery(Event e);

    /**
     * \brief Handle the event -- this must be implemented by subclasses.
     *
     * @param Event the event to be processed.
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
    bool waitUntilCondition(uint64_t maxMs = 0, bool interuptible = false);

    /**
     * \brief Determines if a condition is met (and waits should end).
     *
     * Determine whether a condition is met. The default implementation
     * always returns true. Intended to be overridden by user programs to
     * implement more complex waits and conditions.
     *
     * @param Event the event to check for meeting the condition.
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
     * Advisory lock. Must be held before waitUntilCondition is called.
     * Internal clusterlib event system always respects this lock, and relies
     * on clients calling acquireLock in order to guarantee that conditions
     * are checked with a consistent ordering with respect to waitUntilCondition.
     */
    void acquireLock() { m_waitMutex.lock(); }

    /**
     * \brief Releases the lock acquired by acquireLock.
     *
     * Advisory lock. Must be held before waitUntilCondition is called,
     * and must be released via releaseLock after waitUntilCondition returns.
     */
    void releaseLock() { m_waitMutex.unlock(); }

  private:
    /**
     * Notify any threads that are waiting for the condition, that it
     * has been met.
     */
    void notifyWaiters() { m_waitCond.signal_all(); }

  private:
    /*
     * The Notifyable this handler is for.
     */
    Notifyable *mp_np;

    /*
     * The events (a mask) that this handler is for.
     */
    Event m_mask;

    /*
     * Arbitrary data to pass to the handler for each
     * event as it is triggered.
     */
    ClientData m_cd;

    /*
     * Mutex and Cond used for waiting & condition mgmt.
     */
    Cond m_waitCond;
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
