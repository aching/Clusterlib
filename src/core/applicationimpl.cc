/*
 * applicationmpl.cc --
 *
 * Implementation of the Application class; it represents a set of groups
 * of nodes that together form a clusterlib application.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace boost;

namespace clusterlib {

void
ApplicationImpl::initializeCachedRepresentation()
{
    GroupImpl::initializeCachedRepresentation();
}

}	/* End of 'namespace clusterlib' */
