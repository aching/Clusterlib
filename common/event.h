/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include <string>
#include <set>
#include <deque>
#include <algorithm>
#ifdef GCC4
#   include <tr1/memory>
using namespace std::tr1;
#else
#   include <boost/shared_ptr.hpp>
using namespace boost;
#endif

#include "log.h"
#include "blockingqueue.h"
#include "mutex.h"
#include "thread.h"

DEFINE_LOGGER(EV_LOG, "event")

namespace clusterlib {

/**********************************************************************/
/*                                                                    */
/* INTERNALS OF EVENT SYSTEM -- NOT INTENDED FOR USE BY CLIENTS       */
/*                                                                    */
/**********************************************************************/

/*
 * Enum of the various types of events supported:
 */
enum EventTypes {
    ILLEGALEVENT = -1,
    TIMEREVENT = 0,
    ZKEVENT = 1
};

/*
 * Forward declaration of EventSource.
 */
template<typename E>
class EventSource;

/**
 * \brief This interface is implemented by an observer
 * \brief of a particular {@link EventSource}.
 */
template<typename E>
class EventListener
{
  public:
        
    /**
     * \brief This method is invoked whenever an event 
     * \brief has been received by the event source being observed.
     * 
     * @param source the source the triggered the event
     * @param event the actual event being triggered
     */
    virtual void eventReceived(const EventSource<E> &source, 
                               const E &e)
        = 0;
};  

/**
 * \brief This class represents a source of events.
 * 
 * <p>
 * Each source can have many observers (listeners) attached to it
 * and in case of an event, this source may propagate the event
 * using {@link #fireEvent} method.
 */
template<typename E>           
class EventSource
{
  public:
        
    /**
     * \brief The type corresponding to the list of registered
     * event listeners.
     */
    typedef set<EventListener<E> *> EventListeners;
        
    /**
     * \brief Registers a new event listener.
     * 
     * @param listener the listener to be added to the set of listeners
     */
    void addListener(EventListener<E> *listener)
    {
        m_listeners.insert(listener);
    }
        
    /**
     * \brief Removes an already registered listener.
     * 
     * @param listener the listener to be removed
     */
    void removeListener(EventListener<E> *listener)
    {
        m_listeners.erase(listener);
    }
        
    /**
     * \brief Destructor.
     */
    virtual ~EventSource() {}
        
  protected:
        
    /**
     * \brief Fires the given event to all registered listeners.
     * 
     * <p>
     * This method essentially iterates over all listeners
     * and invokes 
     * {@link fireEvent(EventListener<E> *listener, const E &event)}
     * for each element. All derived classes are free to
     * override the method to provide better error handling
     * than the default implementation.
     * 
     * @param event the event to be propagated to all listeners
     */
    void fireEventToAllListeners(const E &event);
        
    /**
     * \brief Sends an event to the given listener.
     * 
     * @param listener the listener to whom pass the event
     * @param event the event to be handled
     */
    virtual void fireEvent(EventListener<E> *listener, const E &event);
        
  private:
        
    /**
     * The set of registered event listeners.
     */
    EventListeners m_listeners;            
};

/**
 * \brief The interface of a generic event wrapper.
 */
class AbstractEventWrapper
{
  public:
        
    /**
     * \brief Destructor.
     */
    virtual ~AbstractEventWrapper() {}
        
    /**
     * \brief Returns the underlying wrapee's data.
     */
    virtual void *getWrapee() = 0;

    /**
     * \brief Clone functionality for deep copy.
     */
    virtual AbstractEventWrapper *clone() const = 0;
};

/**
 * \brief A template based implementation of {@link AbstractEventWrapper}.
 */
template<typename E>
class EventWrapper
    : public AbstractEventWrapper
{
  public:
    EventWrapper(const E &e)
        : m_e(e)
    {
    }
    ~EventWrapper() {}
    void *getWrapee()
    {
        return &m_e;
    }
    virtual AbstractEventWrapper *clone() const {
        return new EventWrapper(m_e);
    }
  private:
    E m_e;
};

/**
 * \brief This class represents a generic event.
 */
class GenericEvent
{
  public:
        
    /**
     * \brief Constructor.
     */
    GenericEvent() : m_type(0), mp_eventWrapper(NULL) {}

    /**
     * \brief Constructor.
     * 
     * @param type the type of this event
     * @param eventWrapper the wrapper around event's data 
     *                     (transfers ownership to this object)
     */
    GenericEvent(int32_t type, AbstractEventWrapper *eventWrapper)
        : m_type(type),
          mp_eventWrapper(eventWrapper)
    {
    }
        
    ~GenericEvent()
    {
        delete mp_eventWrapper;
    }

    GenericEvent(const GenericEvent &ge)
    {
        m_type = ge.getType();
        mp_eventWrapper = ge.mp_eventWrapper->clone();
    }

    /**
     * \brief Returns the type of this event.
     * 
     * @return type of this event
     */
    int32_t getType() const { return m_type; }
        
    /**
     * \brief Returns the event's data.
     * 
     * @return the event's data
     */
    void *getEvent() const { return mp_eventWrapper->getWrapee(); }
        
  private:
    /**
     * No assignment operator allowed
     */
    GenericEvent &operator = (const GenericEvent &ge);

    /**
     * The event type.
     */
    int32_t m_type;

    /**
     * The event represented as abstract wrapper.
     */
    AbstractEventWrapper *mp_eventWrapper;
    //shared_ptr<AbstractEventWrapper> m_eventWrapper;
        
};
    
/**
 * \brief This class adapts {@link EventListener} to a generic listener.
 * Essentially this class listens on incoming events and fires them 
 * as {@link GenericEvent}s.
 */
template<typename E, const int32_t type>
class EventListenerAdapter
    : public virtual EventListener<E>,
      public virtual EventSource<GenericEvent>
{
  public:
        
    /**
     * \brief Constructor.
     * 
     * @param eventSource the source on which register this listener
     */
    EventListenerAdapter(EventSource<E> &eventSource)
    {
        eventSource.addListener(this);
    }
        
    void eventReceived(const EventSource<E> &source, const E &e)
    {
        LOG_DEBUG(EV_LOG, "EventListenerAdapter::eventReceived: before fire");
        AbstractEventWrapper *wrapper = new EventWrapper<E>(e);
        GenericEvent event(type, wrapper);
        fireEventToAllListeners(event);
    }
};        

/**
 * \brief This class provides an adapter between an asynchronous
 * and synchronous event handling.
 * 
 * <p>
 * This class queues up all received events and exposes them through 
 * {@link #getNextEvent()} method.
 */
template<typename E>                  
class SynchronousEventAdapter
    : public EventListener<E>
{
  public:
        
    void eventReceived(const EventSource<E> &source, const E &e)
    {
        LOG_DEBUG(EV_LOG, 
                  "eventReceived: event 0x%x, instance 0x%x, thread %u",
                  (uint32_t) &e, 
                  (uint32_t) this, 
                  (uint32_t) pthread_self());
        
        m_queue.put(e);
    }

    /**
     * \brief Returns the next available event from the underlying queue,
     * \brief possibly blocking, if no data is available.
     * 
     * @return the next available event
     */
    E getNextEvent()
    {
        LOG_DEBUG(EV_LOG, "SynchronousEventAdapter::getNextEvent: starting");
        return m_queue.take();
    }
        
    /**
     * \brief Returns whether there are any events in the queue or not.
     * 
     * @return true if there is at least one event and 
     *         the next call to {@link #getNextEvent} won't block
     */
    bool hasEvents() const
    {
        return (m_queue.empty() ? false : true);
    }
        
    /**
     * \brief Destructor.
     */
    virtual ~SynchronousEventAdapter() {}

  private:

    /**
     * The blocking queue of all events received so far.
     */
    BlockingQueue<E> m_queue;
};

template<typename E>
void EventSource<E>::fireEventToAllListeners(const E &event)
{
    for (typename EventListeners::iterator eIt = m_listeners.begin(); 
         eIt != m_listeners.end(); 
         ++eIt) 
    {
        fireEvent(*eIt, event);
    }
}

template<typename E>
void EventSource<E>::fireEvent(EventListener<E> *lp, const E &event)
{
    LOG_DEBUG(EV_LOG,
              "fireEvent: Sending event: event 0x%x, listener 0x%x, "
              "thread %u", 
              (uint32_t) &event, 
              (uint32_t) lp, 
              (uint32_t) pthread_self());

    assert(lp);
    lp->eventReceived(*this, event);
}

/*
 * This is a helper class for handling events using a member function.
 */
template<class T>
class EventHandler
{
  public:
    /*
     * Define the type of the member function to invoke.
     */
    typedef Event (T::*EventMethod)(Notifyable *np,
                                    int32_t etype,
                                    const string &path);

    /*
     * Constructor.
     */
    EventHandler(T *objp, EventMethod handler)
        : mp_obj(objp),    
          m_handler(handler)
    {
    };

    /*
     * Deliver the event.
     */
    Event deliver(Notifyable *np, 
                  int32_t etype,
                  const string &path)
    {
        return ((*mp_obj).*m_handler)(np, etype, path);
    };

    /*
     * Retrieve the object on which the method
     * is being called.
     */
    T *getObject() { return mp_obj; }

  private:
    /*
     * The instance.
     */
    T *mp_obj;

    /*
     * The handler method to call.
     */
    EventMethod m_handler;
};

/*
 * Generic types for delivering events. Events are
 * delivered to a Factory object.
 */
typedef EventHandler<Factory> FactoryEventHandler;

/*
 * Payload for delivering events from ZooKeeper to clients of
 * Clusterlib.
 */
class ClusterEventPayload
{
  public:
    /*
     * Constructor.
     */
    ClusterEventPayload(Notifyable *np, Event e)
        : mp_np(np),
          m_e(e)
    {
    }

    /*
     * Destructor.
     */
    virtual ~ClusterEventPayload() {}

    /*
     * Retrieve fields.
     */
    Event getEvent() { return m_e; }
    Notifyable *getTarget() { return mp_np; }

  private:
    /*
     * The target object clients are being notified about.
     */
    Notifyable *mp_np;

    /*
     * The event that clients are being notified about.
     */
    Event m_e;
};

/*
 * Typedef for blocking queue of pointers to cluster event payload objects.
 */
typedef BlockingQueue<ClusterEventPayload *> ClusterEventPayloadQueue;

/***********************************************************************/
/*                                                                     */
/* Below are user level event handling definitions.                    */
/*                                                                     */
/***********************************************************************/

/*---------------------------------------------------------------------*/
/*                                                                     */
/* PART A: CLUSTER EVENTS                                              */
/*                                                                     */
/*---------------------------------------------------------------------*/

/*
 * Various event notification constants.
 */
const int32_t EN_NOEVENT =			0;

const int32_t EN_CREATED =			(1<<0);
const int32_t EN_DELETED =			(1<<1);
const int32_t EN_READY =			(1<<2);

const int32_t EN_GROUPSCHANGE =			(1<<3);
const int32_t EN_DISTSCHANGE =			(1<<4);

const int32_t EN_MEMBERSHIPCHANGE =		(1<<5);
const int32_t EN_LEADERSHIPCHANGE =		(1<<6);

const int32_t EN_HEALTHCHANGE =			(1<<7);
const int32_t EN_CONNECTCHANGE =		(1<<8);
const int32_t EN_MASTERSTATECHANGE =		(1<<9);

const int32_t EN_DISTCHANGE =			(1<<10);
const int32_t EN_PROPCHANGE =			(1<<11);

/*
 * Interface for cluster event handler. Must be derived
 * to define specific behavior for handling events.
 */
class ClusterEventHandler
{
  public:
    /*
     * Constructor.
     */
    ClusterEventHandler(Notifyable *np,
                        Event mask,
                        ClientData cd)
        : mp_np(np),
          m_mask(mask),
          m_cd(cd)
    {
    }

    /*
     * Destructor.
     */
    virtual ~ClusterEventHandler() {}

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

    /*
     * Handle the event -- this must be
     * implemented by subclasses.
     */
    virtual void handleClusterEvent(Event e) = 0;

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
};

/*---------------------------------------------------------------------*/
/*                                                                     */
/* PART B: TIMER EVENTS                                                */
/*                                                                     */
/*---------------------------------------------------------------------*/

/**
 * This class represents a timer event parametrized by the user's data type.
 */
template<typename T>
class TimerEvent
{
  public:
       
    /**
     * \brief Constructor.
     * 
     * @param id the ID of this event
     * @param alarmTime when this event is to be triggered
     * @param userData the user data associated with this event
     */
    TimerEvent(TimerId id, int64_t alarmTime, const T &userData)
        : m_id(id),
          m_alarmTime(alarmTime),
          m_userData(userData) 
    {
        LOG_DEBUG(EV_LOG, 
                  "Created timer event: instance 0x%x, "
                  "id %d alarm time %lld", 
                  (uint32_t) this, 
                  (uint32_t) id, 
                  alarmTime);
    }

    /**
     * \brief Constructor.
     */
    TimerEvent()
        : m_id(-1),
          m_alarmTime(-1)
    {
    }
                           
    /**
     * \brief Returns the ID.
     * 
     * @return the ID of this event
     */
    TimerId getID() const { return m_id; }
        
    /**
     * \brief Returns the alarm time.
     * 
     * @return the alarm time
     */
    int64_t getAlarmTime() const { return m_alarmTime; }
              
    /**
     * \brief Returns the user's data.
     * 
     * @return the user's data
     */
    T const &getUserData() const { return m_userData; }
        
    /**
     * \brief Returns whether the given alarm time is less than this event's 
     * \brief time.
     */
    bool operator<(const int64_t alarmTime) const
    {
        return m_alarmTime < alarmTime;
    }
        
  private:
  
    /**
     * The ID of ths event.
     */
    TimerId m_id;
        
    /**
     * The time at which this event triggers.
     */
    int64_t m_alarmTime;    
        
    /**
     * The user specific data associated with this event.
     */
    T m_userData;
};

template<typename T>
class Timer
    : public EventSource<TimerEvent<T> >
{
  public:
        
    /**
     * \brief Constructor.
     */
    Timer()
        : m_currentEventID(0),
          m_terminating(false)
    {
        m_workerThread.Create(*this, &Timer<T>::sendAlarms);
    }
        
    /**
     * \brief Destructor.
     */
    virtual ~Timer()
    {
        m_terminating = true;
        m_lock.notify();
        m_workerThread.Join();
    }
        
    /**
     * \brief Schedules the given event <code>timeFromNow</code> milliseconds.
     * 
     * @param timeFromNow time from now, in milliseconds, when the event 
     *                    should be triggered 
     * @param userData the user data associated with the timer event
     * 
     * @return the ID of the newly created timer event
     */
    TimerId scheduleAfter(int64_t timeFromNow, const T &userData)
    {
        return scheduleAt(getCurrentTimeMillis() + timeFromNow, userData);
    }

    /**
     * \brief Schedules an event at the given time.
     * 
     * @param absTime absolute time, in milliseconds, at which the event 
     *                should be triggered; the time is measured
     *                from Jan 1st, 1970   
     * @param userData the user data associated with the timer event
     * 
     * @return the ID of the newly created timer event
     */
    TimerId scheduleAt(int64_t absTime, const T &userData)
    {
        m_lock.lock();
        typename QueueType::iterator pos = 
            lower_bound(m_queue.begin(), m_queue.end(), absTime);
        TimerId id = m_currentEventID++;
        TimerEvent<T> event(id, absTime, userData); 
        m_queue.insert(pos, event);
        m_lock.notify();
        m_lock.unlock();
        return id;
    }
        
    /**
     * \brief Returns the current time since Jan 1, 1970, in milliseconds.
     * 
     * @return the current time in milliseconds
     */
    static int64_t getCurrentTimeMillis()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_sec * 1000LL + now.tv_usec / 1000;
    }

    /**
     * \brief Cancels the given timer event.
     * 
     * @param eventID the ID of the event to be canceled
     * 
     * @return whether the event has been canceled
     */
    bool cancelAlarm(TimerId eventID)
    {
        bool canceled = false;                      
        m_lock.lock();
        typename QueueType::iterator i;
        for (i = m_queue.begin(); i != m_queue.end(); ++i) {
            if (eventID == i->getID()) {
                m_queue.erase(i);
                canceled = true;
                break;
            }
        }
        m_lock.unlock();
        return canceled;
    }
        
    /**
     * Executes the main loop of the worker thread.
     */
    void sendAlarms()
    {
        LOG_DEBUG(EV_LOG, 
                  "sendAlarms, this: 0x%x, thread %u",
                  (uint32_t) this, 
                  (uint32_t) pthread_self());
        
        //iterate until terminating
        while (!m_terminating) {
            m_lock.lock();
            //1 step - wait until there is an event in the queue
            if (m_queue.empty()) {
                //wait up to 100ms to get next event
                m_lock.wait(100);
            }
            bool fire = false;
            if (!m_queue.empty()) {
                //retrieve the event from the queue and send it
                TimerEvent<T> event = m_queue.front();      
                //check whether we can send it right away
                int64_t timeToWait = 
                    event.getAlarmTime() - getCurrentTimeMillis();
                if (timeToWait <= 0) {
                    m_queue.pop_front();
                    //we fire only if it's still in the queue and alarm
                    //time has just elapsed (in case the top event
                    //is canceled)
                    fire = true;    
                } else {
                    m_lock.wait(timeToWait);
                }
                m_lock.unlock();
                if (fire) {
                    fireEventToAllListeners(event);
                }
            } else {
                m_lock.unlock();
            }
        }    
    }
        
  private:
        
    /**
     * The type of timer events queue.
     */
    typedef deque<TimerEvent<T> > QueueType;
        
    /**
     * The current event ID, auto-incremented each time a new event 
     * is created.
     */
    TimerId m_currentEventID;
        
    /**
     * The queue of timer events sorted by {@link TimerEvent#alarmTime}.
     */
    QueueType m_queue;
        
    /**
     * The lock used to guard {@link #m_queue}.
     */
    Lock m_lock;
        
    /**
     * The thread that triggers alarms.
     */
    CXXThread<Timer<T> > m_workerThread;
        
    /**
     * Whether {@link #m_workerThread}  is terminating.
     */
    volatile bool m_terminating;
};

/*
 * The payload for a timer event.
 */
class TimerEventPayload
{
  public:
    /*
     * Constructor.
     */
    TimerEventPayload(int64_t ending,
                      TimerEventHandler *handler,
                      ClientData data)
        : m_ending(ending),
          mp_handler(handler),
          mp_data(data),
          m_id((TimerId) 0),
          m_cancelled(false)
    {
    };

    /*
     * Destructor.
     */
    virtual ~TimerEventPayload() {}

    /*
     * Retrieve the fields.
     */
    int64_t getEnding() { return m_ending; }
    TimerEventHandler *getHandler() { return mp_handler; }
    ClientData getData() { return mp_data; }
    TimerId getId() { return m_id; }
    bool cancelled() { return m_cancelled; }

    /*
     * Cancel the event.
     */
    void cancel() { m_cancelled = true; }

    /*
     * Update the timer ID.
     */
    void updateTimerId(TimerId id) { m_id = id; }

  private:
    /*
     * When is the timer ending?
     */
    int64_t m_ending;

    /*
     * The event handler itself.
     */
    TimerEventHandler *mp_handler;

    /*
     * Client data associated with this event.
     */
    ClientData mp_data;

    /*
     * Timer ID for this timer.
     */
    TimerId m_id;

    /*
     * Is this timer cancelled?
     */
    bool m_cancelled;
};

/**
 * The types for timer events and timer event source.
 */
typedef TimerEvent<TimerEventPayload *>		ClusterlibTimerEvent;
typedef Timer<TimerEventPayload *>		ClusterlibTimerEventSource;
typedef BlockingQueue<TimerEventPayload *>	TimerEventQueue;

/**
 * This class must be subclassed to define
 * timer event handlers that can be instantiated.
 */
class TimerEventHandler
{
  public:
    /*
     * Constructor.
     */
    TimerEventHandler()
    {
    }

    /*
     * Destructor.
     */
    virtual ~TimerEventHandler() {}

    /*
     * Handle the event -- this must be implemented by
     * subclasses.
     */
    virtual void handleTimerEvent(TimerId id, ClientData data)
        = 0;
};

}   /* end of 'namespace clusterlib' */

#endif /* __EVENT_H__ */
