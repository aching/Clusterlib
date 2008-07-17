/*
 * group.cc --
 *
 * Implementation of the Group class.
 *
 * =============================================================================
 * $Header:$
 * $Revision:$
 * $Date:$
 * =============================================================================
 */

#include "clusterlib.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{

/*
 * Retrieve a node object. Load it from
 * the cluster if it is not yet in the
 * cache.
 */
Node *
Group::getNode(const string &nodeName)
    throw(ClusterException)
{
    Node *np;

    /*
     * If it is already cached, return the
     * cached node object.
     */
    {
        Locker l1(getNodeMapLock());

        np = m_nodes[nodeName];
        if (np != NULL) {
            return np;
        }
    }

    /*
     * If it is not yet cached, load the
     * node from the cluster, cache it,
     * and return the object.
     */
    np = getDelegate()->getNode(nodeName, this, true);
    if (np != NULL) {
        Locker l2(getNodeMapLock());

        m_nodes[nodeName] = np;
        return np;
    }

    throw ClusterException(string("") +
                           "Cannot find node object " +
                           nodeName);
};

/*
 * Deliver notification to the Notifyable object first,
 * before calling user supplied notification receivers.
 * This gives the object a chance to update its cached
 * representation.
 */
void
Group::deliverNotification(const Event event)
{
};

};	/* End of 'namespace clusterlib' */
