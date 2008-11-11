/*
 * group.cc --
 *
 * Implementation of the Group class.
 *
 * =============================================================================
 * $Header:$
 * $Revision$
 * $Date$
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
Group::getNode(const string &nodeName, 
	       bool create)
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
    np = getDelegate()->getNode(nodeName, this, true, create);
    if (np != NULL) {
        Locker l2(getNodeMapLock());

        m_nodes[nodeName] = np;
        return np;
    }

    throw ClusterException(string("") +
                           "Cannot find node object " +
                           nodeName);
}

/*
 * Update the cached representation of this group.
 */
void
Group::updateCachedRepresentation()
{
}

};	/* End of 'namespace clusterlib' */
