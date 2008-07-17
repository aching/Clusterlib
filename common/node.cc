/*
 * node.cc --
 *
 * Implementation of the Node class.
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
 * Deliver notification to the Notifyable object first,
 * before calling user supplied notification receivers.
 * This gives the object a chance to update its cached
 * representation.
 */
void
Node::deliverNotification(const Event event)
{
};

};	/* End of 'namespace clusterlib' */
