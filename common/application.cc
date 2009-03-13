/*
 * application.cc --
 *
 * Implementation of the Application class.
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
 * Update the cached representation of this group.
 */
void
Application::updateCachedRepresentation()
{
    Group::updateCachedRepresentation();
}

};	/* End of 'namespace clusterlib' */
