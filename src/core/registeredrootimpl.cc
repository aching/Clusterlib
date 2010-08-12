/*
 * registeredrootimpl.cc
 *
 * Implementation of the RegisteredRootImpl class.
 *
 * ===========================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

#include "clusterlibinternal.h"

using namespace std;
using namespace boost;

namespace clusterlib {

const string &
RegisteredRootImpl::registeredName() const
{
    return ClusterlibStrings::REGISTERED_ROOT_NAME;
}

string
RegisteredRootImpl::generateKey(const string &parentKey, 
                                const string &name) const
{
    return NotifyableKeyManipulator::createRootKey();
}

bool
RegisteredRootImpl::isValidName(const string &name) const
{
    TRACE(CL_LOG, "isValidName");

    if (name.compare(ClusterlibStrings::ROOT)) {
        return false;
    }
    else {
        return true;
    }
}

shared_ptr<NotifyableImpl>
RegisteredRootImpl::createNotifyable(const string &notifyableName,
                                     const string &notifyableKey,
                                     const shared_ptr<NotifyableImpl> &parent,
                                     FactoryOps &factoryOps) const
{
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<Root>(new RootImpl(&factoryOps,
                                      notifyableKey,
                                      notifyableName)));
}

vector<string>
RegisteredRootImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        NotifyableKeyManipulator::createApplicationChildrenKey(notifyableKey));

    return resVec;
}

bool
RegisteredRootImpl::isValidKey(const vector<string> &components,
                               int32_t elements)
{
    TRACE(CL_LOG, "isValidKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isValidKey: elements %" PRId32 
                  " > size of components %" PRIuPTR,
                  elements,
                  components.size());
        throw InvalidArgumentsException("isValidKey: elements > size of "
                                        "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (elements != ClusterlibInts::ROOT_COMPONENTS_COUNT) {
        return false;
    }

    if ((components.at(ClusterlibInts::CLUSTERLIB_INDEX) != 
         ClusterlibStrings::CLUSTERLIB) ||
        (components.at(ClusterlibInts::ROOT_INDEX) != 
         ClusterlibStrings::ROOT)) {
        return false;
    } 

    return true;    
}

bool
RegisteredRootImpl::getObjectFromComponents(
    const vector<string> &components,
    int32_t elements, 
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<NotifyableImpl> *pNotifyableSP)
{
    TRACE(CL_LOG, "getObjectFromComponents");

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (!isValidKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getRootFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return shared_ptr<NotifyableImpl>();
    }

    LOG_DEBUG(CL_LOG, 
              "getRootFromComponents: root name = %s", 
              components.at(elements - 1).c_str());

    /*
     * Since the root was created in the FactoryOps constructor, it
     * has to be found in the cache.  The only time it is permitted to
     * not exist if when the root is being created and the
     * handleNotifyableRemovedChange is being called, but it doesn't
     * exist fully yet.
     */
    return getOps()->getNotifyableWaitMsecs(
        shared_ptr<NotifyableImpl>(),
        ClusterlibStrings::REGISTERED_ROOT_NAME,
        components.at(elements - 1),
        CACHED_ONLY,
        msecTimeout,
        pNotifyableSP);
}

}	/* End of 'namespace clusterlib' */
