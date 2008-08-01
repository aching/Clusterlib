/*
 * clusterserver.cc --
 *
 * Implementation of the ClusterServer class.
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
 * Constructor.
 */
ClusterServer::ClusterServer(FactoryOps *f,
                             const string &appName,
                             const string &grpName,
                             const string &nodeName,
                             HealthChecker *healthChecker,
                             ServerFlags flags)
    : ClusterClient(f),
      mp_f(f),
      m_appName(appName),
      m_grpName(grpName),
      m_nodeName(nodeName),
      mp_healthChecker(healthChecker),
      m_flags(flags)
{
    m_key = mp_f->createNodeKey(m_appName,
                                m_grpName,
                                m_nodeName,
                                (m_flags & SF_MANAGED) == SF_MANAGED);
    mp_node = mp_f->getNode(m_appName,
                            m_grpName,
                            m_nodeName,
                            (m_flags & SF_MANAGED) == SF_MANAGED);
};

};	/* End of 'namespace clusterlib' */
