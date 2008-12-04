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
#if 0
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
#endif

Properties *
Notifyable::getProperties(bool create)
{
    Properties *prop;

    /* TODO:  Check to see if this is a properties key, should reject this. */
    string propertiesName = mp_f->createPropertiesKey(getKey());

    /*
     * If it is already cached, return the
     * cached properties object.
     */
    {
        Locker l1(getPropertiesMapLock());

        prop = m_properties[propertiesName];
        if (prop != NULL) {
            return prop;
        }
    }

    /*
     * If it is not yet cached, load the
     * properties from the cluster, cache it,
     * and return the object.
     */
    prop = getDelegate()->getProperties(propertiesName, create);
    if (prop != NULL) {
        Locker l2(getPropertiesMapLock());

        m_properties[propertiesName] = prop;
        return prop;
    }

    /*
     * Object not found.
     */
    throw ClusterException(string("") +
                           "Cannot find properties object " +
                           propertiesName);

}

};	/* End of 'namespace clusterlib' */

