/*
 * clusterserver.cc --
 *
 * Implementation of the Server class.
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
 * Constructor.
 */
Server::Server(FactoryOps *f,
               const string &appName,
               const string &grpName,
               const string &nodeName,
               HealthChecker *healthChecker,
               ServerFlags flags)
    : Client(f),
      mp_f(f),
      m_appName(appName),
      m_grpName(grpName),
      m_nodeName(nodeName),
      mp_healthChecker(healthChecker),
      m_flags(flags)
{
    mp_node = mp_f->getNode(m_appName,
                            m_grpName,
                            m_nodeName,
                            (m_flags & SF_MANAGED) == SF_MANAGED);
    if (mp_node == NULL) {
        throw ClusterException("Could not find or create node!");
    }
};

};	/* End of 'namespace clusterlib' */
