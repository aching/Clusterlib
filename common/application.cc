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
 * Initialize the cached representation of this group.
 */
void
Application::initializeCachedRepresentation()
{
    Group::initializeCachedRepresentation();
}

};	/* End of 'namespace clusterlib' */
