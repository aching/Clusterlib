/*
 * payload.h --
 *
 * Defines interface for payload carried by all events.
 *
 * ============================================================================
 * $Header$
 * $Revision:$
 * $Date:$
 * ============================================================================
 */

#ifndef	__PAYLOAD_H__
#define __PAYLOAD_H__

namespace clusterlib
{

/*
 * Common part of payload carried by all events.
 */
class Payload
{
  public:
    /*
     * Constructor.
     */
    Payload(ClusterClient *ccp,
            ClusterClientEventHandler *handler,
            void *data)
        : mp_ccp(ccp),
          mp_handler(handler),
          mp_data(data)
    {
    }

    /*
     * Destructor.
     */
    virtual ~Payload();

    /*
     * Retrieve the payload elements.
     */
    ClusterClient *getClient() { return mp_ccp; }
    ClusterClientEventHandler *getHandler() { return mp_handler; }
    void *getData() { return mp_data; }

  private:
    /*
     * The client for which this payload is destined.
     */
    ClusterClient *mp_ccp;

    /*
     * The handler which will be invoked to handle the event.
     */
    ClusterClientEventHandler *mp_handler;

    /*
     * Data to pass to the handler.
     */
    void *mp_data;
};

/*
 * Typedef for blocking queue of pointers to payload objects.
 */
typedef BlockingQueue<Payload *> PayloadQueue;

};	/* End of 'namespace clusterlib' */

#endif	/* !__PAYLOAD_H__ */
 
