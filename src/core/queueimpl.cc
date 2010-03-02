/* 
 * queueimpl.cc --
 *
 * Implementation of the QueueImpl class.

 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "Queue"

#include <limits>
#include "clusterlibinternal.h"
#include <boost/regex.hpp>

using namespace std;
using namespace boost;

namespace clusterlib
{

/**********************************************************************/
/* Implementation of class QueueImpl                                  */
/**********************************************************************/

/*
 * Constructor.
 */
QueueImpl::QueueImpl(FactoryOps *fp,
                     const string &key,
                     const string &name,
                     NotifyableImpl *parent)
    : NotifyableImpl(fp, key, name, parent)
{
    TRACE(CL_LOG, "QueueImpl");
}

int64_t
QueueImpl::put(const string &element)
{
    TRACE(CL_LOG, "put");

    string queuePrefix = NotifyableKeyManipulator::createQueuePrefixKey(
        getKey());

    int64_t myBid = -1;
    string createdPath;
    SAFE_CALL_ZK((myBid = getOps()->getRepository()->createSequence(
                      queuePrefix, 
                      element,
                      0, 
                      false, 
                      createdPath)),
                 "Creating new queue element for queue prefix %s failed: %s",
                 queuePrefix.c_str(),
                 false,
                 true);
    if (myBid == -1) {
        throw InconsistentInternalStateException(
            "put: Failed to create new element in Zookeeper");
    }

    LOG_DEBUG(CL_LOG,
              "put: Added new element with id (%Ld) value (%s) to queue (%s)",
              myBid,
              element.c_str(),
              queuePrefix.c_str());

    return myBid;
}

void
QueueImpl::take(string &element) 
{
    TRACE(CL_LOG, "take");

    if (!takeWaitMsecs(-1LL, element)) {
        throw InconsistentInternalStateException(
            "take: Impossible that takeWaitMsecs failed!");
    }
}

bool
QueueImpl::takeWaitMsecs(int64_t msecTimeout, 
                         string &element)
{
    TRACE(CL_LOG, "takeWaitMsecs");

    const string &queueParent = getKey();

    bool signaled;
    int64_t curUsecTimeout = 0;
    int64_t maxUsecs = 0;
    if (msecTimeout != 0) {
        maxUsecs = TimerService::getCurrentTimeUsecs() + msecTimeout * 1000;
    }
    
    /*
     * Algorithm:
     * 1. Get the children.
     * 2. If children.size > 0, try to get the value of and remove the 
     *    first child.
     * 3.   If successful, return child.
     * 4. Wait for the parent to change.
     * 5. Goto 1.
     */
    NameList childList;
    bool found = false;
    bool deletedNode = false;
    do {
        SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                         queueParent,
                         childList),
                     "Getting children for node %s failed: %s",
                     queueParent.c_str(),
                     false,
                     true);

        LOG_DEBUG(CL_LOG, 
                  "take: Found %d child for parent %s", 
                  childList.size(), 
                  queueParent.c_str());
        if (childList.size() > 0) {
            SAFE_CALL_ZK((found = getOps()->getRepository()->getNodeData(
                              childList.front(),
                              element)),
                          "Getting the front %s failed: %s",
                          childList.front().c_str(),
                          false,
                          true);
            if (!found) {
                continue;
            }
            
            SAFE_CALL_ZK((deletedNode = getOps()->getRepository()->deleteNode(
                              childList.front(),
                              false,
                              -1)),
                         "Deleting the front %s failed: %s",
                         childList.front().c_str(),
                         false,
                         true);
            if (deletedNode == true) {
                LOG_DEBUG(CL_LOG, 
                          "take: Returning element (%s) from front (%s)", 
                          element.c_str(),
                          childList.front().c_str());
                return true;
            }
            else {
                continue;
            }
        }

        /*
         * Set up the waiting for the handler function on the parent.
         */
        getOps()->getQueueEventSignalMap()->addRefPredMutexCond(
            queueParent);
        CachedObjectEventHandler *handler = 
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(
                CachedObjectChangeHandlers::QUEUE_CHILD_CHANGE);
        SAFE_CALL_ZK(
            getOps()->getRepository()->getNodeChildren(
                queueParent,
                childList,
                getOps()->getZooKeeperEventAdapter(),
                handler),
            "Checking for number of children on %s failed: %s",
            queueParent.c_str(),
            false,
            true);
        
        /* 
         * Wait until it a signal from the from event handler (or
         * until remaining timeout)
         */
        LOG_DEBUG(CL_LOG, 
                  "takeWaitMsecs: num chidren=%d, wait if 0, msecTimeout=%Ld", 
                  childList.size(), msecTimeout);
        if (childList.size() == 0) {
            if (msecTimeout == -1) {
                getOps()->getQueueEventSignalMap()->waitUsecsPredMutexCond(
                    queueParent, msecTimeout);
            }
            else {
                /* Don't let curUsecTimeout go negative. */
                curUsecTimeout = max(
                    maxUsecs - TimerService::getCurrentTimeUsecs(), 0LL);
                signaled = 
                    getOps()->getQueueEventSignalMap()->waitUsecsPredMutexCond(
                        queueParent, curUsecTimeout);
                if ((!signaled) || 
                    (TimerService::compareTimeUsecs(maxUsecs) >= 0)) {
                    break;
                }
            }
        }

        /*
         * Only clean up if we are the last thread to wait on this
         * conditional (otherwise, just decrease the reference
         * count).
         */
        getOps()->getQueueEventSignalMap()->removeRefPredMutexCond(
            queueParent);
    } while (1);

    /* Escaping the loop means that timeout passed. */
    getOps()->getQueueEventSignalMap()->removeRefPredMutexCond(
        queueParent);
    return false;
}

bool
QueueImpl::front(string &element)
{
    TRACE(CL_LOG, "front");

    const string &queueParent = getKey();
    NameList childList;
    bool found = false;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                     queueParent,
                     childList),
                 "Getting children for node %s failed: %s",
                 queueParent.c_str(),
                 false,
                 true);
    if (childList.size() > 0) {
        SAFE_CALL_ZK((found = getOps()->getRepository()->getNodeData(
                          childList.front(),
                          element)),
                     "Getting the front %s failed: %s",
                     childList.front().c_str(),
                     false,
                     true);
        if (found) {
            return true;
        }
    }
    return false;
}

int64_t
QueueImpl::size()
{
    TRACE(CL_LOG, "size");
    
    const string &queueParent = getKey();

    NameList childList;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                     queueParent,
                     childList),
                 "Getting children for node %s failed: %s",
                 queueParent.c_str(),
                 false,
                 true);
    return childList.size();
}

bool 
QueueImpl::empty()
{
    TRACE(CL_LOG, "empty");

    return (size() == 0);
}

void 
QueueImpl::clear()
{
    TRACE(CL_LOG, "clear");

    const string &queueParent = getKey();
    NameList childList;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                     queueParent,
                     childList),
                 "Getting children for node %s failed: %s",
                 queueParent.c_str(),
                 false,
                 true);
    NameList::iterator childListIt;
    for (childListIt = childList.begin();
         childListIt != childList.end();
         childListIt++) {
        SAFE_CALL_ZK(getOps()->getRepository()->deleteNode(
                         *childListIt,
                         false,
                         -1),
                     "Deleting the front %s failed: %s",
                     childList.front().c_str(),
                     false,
                     true);
    }
}

bool 
QueueImpl::removeElement(int64_t id)
{
    TRACE(CL_LOG, "removeElement");

    const string &queueParent = getKey();
    NameList childList;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                     queueParent,
                     childList),
                 "Getting children for node %s failed: %s",
                 queueParent.c_str(),
                 false,
                 true);
    NameList::iterator childListIt;
    string name;
    int64_t sequenceNumber = -1;
    bool deletedNode = false;
    for (childListIt = childList.begin();
         childListIt != childList.end();
         childListIt++) {
        zk::ZooKeeperAdapter::splitSequenceNode(*childListIt,
                                                &name,
                                                &sequenceNumber);
        if (id == sequenceNumber) {
            SAFE_CALL_ZK((deletedNode = getOps()->getRepository()->deleteNode(
                              *childListIt,
                              false,
                             -1)),
                         "Deleting the front %s failed: %s",
                         childList.front().c_str(),
                         false,
                         true);
            if (deletedNode == true) {
                return true;
            }
            break;
        }
    }

    return false;
}

map<int64_t, string>
QueueImpl::getAllElements()
{
    TRACE(CL_LOG, "getAllElements");

    map<int64_t, string> idElementMap;
    map<int64_t, string>::iterator idElementMapIt;
    const string &queueParent = getKey();
    NameList childList;
    SAFE_CALL_ZK(getOps()->getRepository()->getNodeChildren(
                     queueParent,
                     childList),
                 "Getting children for node %s failed: %s",
                 queueParent.c_str(),
                 false,
                 true);
    NameList::iterator childListIt;
    int64_t sequenceNumber = -1;
    bool found = false;
    string element;
    for (childListIt = childList.begin();
         childListIt != childList.end();
         childListIt++) {
        zk::ZooKeeperAdapter::splitSequenceNode(*childListIt,
                                                NULL,
                                                &sequenceNumber);

        SAFE_CALL_ZK((found = getOps()->getRepository()->getNodeData(
                          *childListIt,
                          element)),
                     "Getting the element %s failed: %s",
                     childListIt->c_str(),
                     false,
                     true);
        
        if (found) {
            idElementMapIt = idElementMap.find(sequenceNumber);
            if (idElementMapIt != idElementMap.end()) {
                throw InconsistentInternalStateException(
                    string("getAllElements: Already found entry and ") +
                    "should be unique" + *childListIt);
            }
            idElementMap[sequenceNumber] = element;
        }
    }

    return idElementMap;
}

void
QueueImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");
    
    /*
     * Ensure that the cache contains all the information about this
     * object, and that all watches are established.
     */
    establishQueueWatch();
}

void
QueueImpl::removeRepositoryEntries()
{
    getOps()->removeQueue(this);
}

QueueImpl::~QueueImpl()
{
}

void 
QueueImpl::establishQueueWatch()
{
    TRACE(CL_LOG, "establishQueueWatch");

    const string &queueParent = getKey();
    NameList childList;
    SAFE_CALLBACK_ZK(
        getOps()->getRepository()->getNodeChildren(
            queueParent,
            childList,
            getOps()->getZooKeeperEventAdapter(), 
            getOps()->getCachedObjectChangeHandlers()->
            getChangeHandler(CachedObjectChangeHandlers::QUEUE_CHILD_CHANGE)),
        ,
        CachedObjectChangeHandlers::QUEUE_CHILD_CHANGE,
        queueParent,
        "Reestablishing watch on value of %s failed: %s",
        queueParent.c_str(),
        true,
        true);
}

};       /* End of 'namespace clusterlib' */
