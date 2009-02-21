/*
 * node.cc --
 *
 * Implementation of the Node class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlib.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{

/*
 * Update the cached representation of this node.
 */
void
Node::updateCachedRepresentation()
{
    TRACE(CL_LOG, "updateCachedRepresentation");
};

};	/* End of 'namespace clusterlib' */
