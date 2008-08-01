/*
 * timer.h --
 *
 * Mechanism for handling timer events.
 *
 * ============================================================================
 * $Header$
 * $Revision:$
 * $Date:$
 * ============================================================================
 */

#ifndef	__TIMER_H__
#define __TIMER_H__

namespace clusterlib
{

/*
 * The payload for a timer event.
 */
class TimerPayload
    : public virtual Payload
{
  public:
    /*
     * Constructor.
     */
    TimerPayload(ClusterClient *ccp,
                 int64_t ending,
                 ClusterClientEventHandler *handler)
        : Payload(ccp, handler, NULL),
          m_ending(ending),
          m_cancelled(false)
    {
    };

    /*
     * Destructor.
     */
    virtual ~TimerPayload() {}

    /*
     * Retrieve the fields.
     */
    int64_t ending() { return m_ending; }
    bool cancelled() { return m_cancelled; }

    /*
     * Cancel the event.
     */
    void cancel() { m_cancelled = true; }

  private:
    /*
     * When is the timer ending?
     */
    int64_t m_ending;

    /*
     * Is this timer cancelled?
     */
    bool m_cancelled;
};

/**
 * The types for timer events and timer event source.
 */
typedef TimerEvent<TimerPayload *> ClusterlibTimerEvent;
typedef Timer<TimerPayload *> ClusterlibTimerEventSource;

};	/* End of 'namespace clusterlib' */

#endif	/* !__TIMER_H__ */
