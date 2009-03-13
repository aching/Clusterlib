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
Notifyable::Notifyable(FactoryOps *fp,
                       const string &key,
                       const string &name,
                       Notifyable *parent)
    : mp_f(fp),
      m_key(key),
      m_name(name),
      mp_parent(parent),
      m_ready(false)
{
    mp_f->establishNotifyableReady(this);
};

/*
 * Destructor.
 */
Notifyable::~Notifyable()
{
}

Properties *
Notifyable::getProperties(bool create)
{
    Properties *prop;

    /*
     * If it is already cached, return the
     * cached properties object.
     */
    {
        Locker l1(getPropertiesMapLock());

        PropertiesMap::const_iterator propIt = 
            m_properties.find(getKey());
        if (propIt != m_properties.end()) {
            return propIt->second;
        }
    }

    /*
     * If it is not yet cached, load the
     * properties from the cluster, cache it,
     * and return the object.
     */
    prop = getDelegate()->getProperties(this, create);
    if (prop != NULL) {
        Locker l2(getPropertiesMapLock());

        m_properties[getKey()] = prop;
        return prop;
    }

    /*
     * Object not found.
     */
    throw ClusterException(string("Cannot find properties object ") +
                           getKey());
}

Notifyable *
Notifyable::getMyParent() const
{
    return mp_parent;
}

Application *
Notifyable::getMyApplication()
{
    string appKey = getKey();
    Application *app = NULL;

    do {
        app = getDelegate()->getApplicationFromKey(appKey, false);
        appKey = getDelegate()->removeObjectFromKey(getKey());
    }  while ((app == NULL) && (!appKey.empty()));

    return app;
}

Group *
Notifyable::getMyGroup()
{
    string groupKey = getKey();

    /*
     * Try to find the group or application (in that order) from the
     * string working backwards.  Otherwise give up and return NULL.
     */
    uint32_t foundGroupKey = groupKey.rfind(ClusterlibStrings::GROUPS);
    if (foundGroupKey != string::npos) {
        /* 
         * Resize groupKey to end at the group name
         */
        uint32_t foundGroupName = 
            groupKey.find(ClusterlibStrings::PATHSEPARATOR, foundGroupKey);
        if ((foundGroupName != string::npos) &&
            (foundGroupName != (groupKey.size() - 1))) {
            foundGroupName = groupKey.find(ClusterlibStrings::PATHSEPARATOR, 
                                           foundGroupName + 1);
            groupKey.resize(foundGroupName);
            return getDelegate()->getGroupFromKey(groupKey, false);

        }
    }

    foundGroupKey = groupKey.rfind(ClusterlibStrings::APPLICATIONS);
    if (foundGroupKey != string::npos) {
        /* 
         * Resize groupKey to end at the application name
         */
        uint32_t foundGroupName = 
            groupKey.find(ClusterlibStrings::PATHSEPARATOR, foundGroupKey);
        if ((foundGroupName != string::npos) &&
            (foundGroupName != (groupKey.size() - 1))) {
            foundGroupName = groupKey.find(ClusterlibStrings::PATHSEPARATOR, 
                                           foundGroupName + 1);
            groupKey.resize(foundGroupName);
            return getDelegate()->getGroupFromKey(groupKey, false);

        }
    }
    
    LOG_WARN(CL_LOG, 
             "getMyGroup: Couldn't find a group or application "
             "with key %s",
             getKey().c_str());

    return NULL;
}

};	/* End of 'namespace clusterlib' */

