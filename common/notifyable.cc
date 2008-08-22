/*
 * group.cc --
 *
 * Implementation of the notification classes outlined methods.
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
 * Constructor.
 */
NotificationReceiver::NotificationReceiver(const Event mask,
                                           Client *cl,
                                           Notifyable *np)
    : m_mask(mask),
      mp_notifyable(np),
      mp_queue(cl->getQueue())
{
}

};	/* End of 'namespace clusterlib' */
