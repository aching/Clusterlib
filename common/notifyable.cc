/*
 * notifyable.cc
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
Notifyable::Notifyable(FactoryOps *f,
                       const string &key,
                       const string &name)
    : mp_f(f),
      m_key(key),
      m_name(name),
      m_ready(false)
{
    m_interests.clear();
    mp_f->establishNotifyableReady(this);
};

/*
 * Destructor.
 */
Notifyable::~Notifyable()
{
    ClusterEventInterests::iterator i;
    Locker l(getInterestsLock());

    for (i = m_interests.begin();
         i != m_interests.end();
         i++) {
        delete (*i);
    }
    m_interests.clear();
};

/*
 * Notification mechanism.
 */
void
Notifyable::notify(const Event e)
{
    ClusterEventInterests::iterator i;
    ClusterEventInterests interests;

    {
        /*
         * Make a copy of the current notification interests
         * list to protect against side effects. Suggested by
         * ruslanm@yahoo-inc.com, thanks!
         */
        Locker l(getInterestsLock());

        interests = m_interests;
    }

    /*
     * Deliver the event to all interested "user-land" clients.
     */
    for (i = interests.begin(); i != interests.end(); i++) {
        if ((*i)->matchEvent(e)) {
            (*i)->deliverNotification(e, this);
        }
    }
};

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

};

};	/* End of 'namespace clusterlib' */

