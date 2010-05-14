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

namespace clusterlib
{

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

NotifyableImpl *
RegisteredRootImpl::createNotifyable(const string &notifyableName,
                                     const string &notifyableKey,
                                     NotifyableImpl *parent,
                                     FactoryOps &factoryOps) const
{
    return new RootImpl(&factoryOps,
                        notifyableKey,
                        notifyableName);
}

vector<string>
RegisteredRootImpl::generateRepositoryList(
    const std::string &notifyableName,
    const std::string &notifyableKey) const
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

NotifyableImpl *
RegisteredRootImpl::getObjectFromComponents(
    const vector<string> &components,
    int32_t elements, 
    AccessType accessType)
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
        return NULL;
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
    RootImpl *root =
        dynamic_cast<RootImpl *>(
            getOps()->getNotifyable(NULL,
                                    ClusterlibStrings::REGISTERED_ROOT_NAME,
                                    components.at(elements - 1),
                                    CACHED_ONLY));

    return root;
}

};	/* End of 'namespace clusterlib' */
