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

using namespace std;
using namespace boost;

namespace clusterlib {

NameList
RootImpl::getApplicationNames()
{
    TRACE(CL_LOG, "getApplicationNames");

    return getOps()->getChildrenNames(
        NotifyableKeyManipulator::createApplicationChildrenKey(getKey()),
        CachedObjectChangeHandlers::APPLICATIONS_CHANGE);
}

bool
RootImpl::getApplicationWaitMsecs(
    const string &name,
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<Application> *pApplicationSP) 
{
    TRACE(CL_LOG, "getApplicationWaitMsecs");

    throwIfRemoved();

    shared_ptr<NotifyableImpl> notifyableSP;
    bool completed = getOps()->getNotifyableWaitMsecs(
        shared_from_this(),
        ClusterlibStrings::REGISTERED_APPLICATION_NAME,
        name,
        accessType,
        msecTimeout,
        &notifyableSP);

    *pApplicationSP = dynamic_pointer_cast<Application>(notifyableSP);
    return completed;
}

shared_ptr<Application>
RootImpl::getApplication(const string &name,
                         AccessType accessType)
{
    shared_ptr<Application> applicationSP;
    getApplicationWaitMsecs(name, accessType, -1, &applicationSP);
    return applicationSP;
}

NotifyableList
RootImpl::getChildrenNotifyables()
{
    TRACE(CL_LOG, "getChildrenNotifyables");
    
    throwIfRemoved();
    
    return getOps()->getNotifyableList(
        shared_from_this(),
        ClusterlibStrings::REGISTERED_APPLICATION_NAME,
        getApplicationNames(),
        LOAD_FROM_REPOSITORY);
}

void
RootImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");
}

}	/* End of 'namespace clusterlib' */
