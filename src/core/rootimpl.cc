/*
 * rootmpl.cc --
 *
 * Implementation of the Root class; it represents a set of
 * applications in a clusterlib instance.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;

namespace clusterlib
{

string RootImpl::m_registeredName = "root";

NameList
RootImpl::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createApplicationChildrenKey(getKey()),
        CachedObjectChangeHandlers::APPLICATIONS_CHANGE);
}

Application *
RootImpl::getApplication(const string &appName, AccessType accessType)
{
    TRACE(CL_LOG, "getApplication");

    return dynamic_cast<Application *>(
        getOps()->getNotifyable(this,
                                ClusterlibStrings::REGISTERED_APPLICATION_NAME,
                                appName,
                                accessType));
}

Notifyable *
RootImpl::getNotifyableFromKey(const string &key)
{
    TRACE(CL_LOG, "getNotifyableFromKey");

    return getOps()->getNotifyableFromKey(vector<string>(), key);
}

NotifyableList
RootImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");
    
    throwIfRemoved();
    
    return getOps()->getNotifyableList(
        this,
        ClusterlibStrings::REGISTERED_APPLICATION_NAME,
        getApplicationNames(),
        LOAD_FROM_REPOSITORY);
}

void
RootImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");
}

string
RootImpl::generateKey(const string &parentKey, const string &name) const
{
    return NotifyableKeyManipulator::createRootKey();
}

bool
RootImpl::isValidName(const string &name) const
{
    TRACE(CL_LOG, "isValidName");

    /* Name is not used for the root */
    return true;
}

NotifyableImpl *
RootImpl::createNotifyable(const string &notifyableName,
                           const string &notifyableKey,
                           NotifyableImpl *parent,
                           FactoryOps &factoryOps)
{
    return new RootImpl(&factoryOps,
                        notifyableKey,
                        notifyableName);
}

vector<string>
RootImpl::generateRepositoryList(const std::string &notifyableName,
                                 const std::string &notifyableKey)
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        NotifyableKeyManipulator::createApplicationChildrenKey(notifyableKey));

    return resVec;
}

};	/* End of 'namespace clusterlib' */
