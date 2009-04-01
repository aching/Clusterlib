/*
 * nodeimpl.cc --
 *
 * Implementation of the NodeImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{

/*
 * Initialize the cached representation of this node.
 */
void
NodeImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    /*
     * Ensure that the cache contains all the information
     * about this node, and that all watches are established.
     */
    m_connected = getDelegate()->isNodeConnected(
        getKey());
    m_clientState = getDelegate()->getNodeClientState(
        getKey());
    m_masterSetState = getDelegate()->getNodeMasterSetState(
        getKey());
}

};	/* End of 'namespace clusterlib' */
