/*
 * activenodeperiodiccheck.cc --
 *
 * Implementation of the ActiveNodePeriodicCheck object.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include <sys/utsname.h>
#include <string.h>
#include <sys/wait.h>
#include "clusterlibinternal.h"
#include "activenodeperiodiccheck.h"

using namespace std;
using namespace clusterlib;
using namespace json;

namespace activenode {

ActiveNodePeriodicCheck::ActiveNodePeriodicCheck(int64_t msecsFrequency,
                                                 Notifyable *notifyable,
                                                 PredMutexCond &predMutexCond)
    : Periodic(msecsFrequency, notifyable),
      m_predMutexCond(predMutexCond)
{
}

ActiveNodePeriodicCheck::~ActiveNodePeriodicCheck()
{
}

void
ActiveNodePeriodicCheck::run()
{
    TRACE(CL_LOG, "run");

    if (getNotifyable() == NULL) {
        throw InconsistentInternalStateException(
            "run: getNotifyable() is NULL");
    }

    bool shutdownFound = false;
    JSONValue jsonShutdown;
    
    getNotifyable()->acquireLock();

    getNotifyable()->cachedCurrentState().set(Node::HEALTH_KEY,
                                              Node::HEALTH_GOOD_VALUE);
    shutdownFound = getNotifyable()->cachedDesiredState().get(
        Node::ACTIVENODE_SHUTDOWN, jsonShutdown);
    int64_t msecs = TimerService::getCurrentTimeMsecs();
    getNotifyable()->cachedCurrentState().set(
        Node::HEALTH_KEY, Node::HEALTH_GOOD_VALUE);
    getNotifyable()->cachedCurrentState().set(
        Node::HEALTH_SET_MSECS_KEY, msecs);
    getNotifyable()->cachedCurrentState().set(
        Node::HEALTH_SET_MSECS_AS_DATE_KEY, 
        TimerService::getMsecsTimeString(msecs));
    getNotifyable()->cachedCurrentState().publish();

    getNotifyable()->releaseLock();

    if (shutdownFound) {
        if (jsonShutdown.get<JSONValue::JSONBoolean>() == true) {
            /* Let the ActiveNode know to shutdown. */
            m_predMutexCond.predSignal();
        }
    }
} 

}
