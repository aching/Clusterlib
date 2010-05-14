/*
 * registeredapplicationimpl.cc
 *
 * Implementation of the RegisteredApplicationImpl class.
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
RegisteredApplicationImpl::registeredName() const
{
    return ClusterlibStrings::REGISTERED_APPLICATION_NAME;
}

string
RegisteredApplicationImpl::generateKey(const string &parentKey, 
                                       const string &name) const
{
    return NotifyableKeyManipulator::createApplicationKey(name);
}

bool
RegisteredApplicationImpl::isValidName(const string &name) const
{
    TRACE(CL_LOG, "isValidName");

    if ((name.compare(ClusterlibStrings::DEFAULT_CLI_APPLICATION)) &&
        (!NotifyableKeyManipulator::isValidNotifyableName(name))) {
        LOG_WARN(CL_LOG,
                 "isValidName: Illegal Application name %s",
                 name.c_str());
        return false;
    }
    else {
        return true;
    }
}

NotifyableImpl *
RegisteredApplicationImpl::createNotifyable(const string &notifyableName,
                                            const string &notifyableKey,
                                            NotifyableImpl *parent,
                                            FactoryOps &factoryOps) const
{
    return new ApplicationImpl(&factoryOps,
                               notifyableKey,
                               notifyableName,
                               parent);
}

vector<string>
RegisteredApplicationImpl::generateRepositoryList(
    const std::string &notifyableName,
    const std::string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        NotifyableKeyManipulator::createGroupChildrenKey(notifyableKey));
    resVec.push_back(
        NotifyableKeyManipulator::createDataDistributionChildrenKey(
            notifyableKey));
    resVec.push_back(
        NotifyableKeyManipulator::createNodeChildrenKey(notifyableKey));

    return resVec;
}

bool
RegisteredApplicationImpl::isValidKey(const vector<string> &components,
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

    if (elements != ClusterlibInts::APP_COMPONENTS_COUNT) {
        return false;
    }

    if ((components.at(ClusterlibInts::CLUSTERLIB_INDEX) != 
         ClusterlibStrings::CLUSTERLIB) ||
        (components.at(ClusterlibInts::ROOT_INDEX) != 
         ClusterlibStrings::ROOT) ||
        (components.at(ClusterlibInts::APP_INDEX) != 
         ClusterlibStrings::APPLICATIONS)) {
        return false;
    } 

    return true;    
}

NotifyableImpl *
RegisteredApplicationImpl::getObjectFromComponents(
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
                  "getObjectFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return NULL;
    }

    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return NULL;
    }

    RegisteredNotifyable *regRoot = getOps()->getRegisteredNotifyable(
        ClusterlibStrings::REGISTERED_ROOT_NAME, true);
    RootImpl *parent = dynamic_cast<RootImpl *>(
        regRoot->getObjectFromComponents(components,
                                        parentGroupCount,
                                        accessType));
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getObjectFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getObjectFromComponents: parent key = %s, "
              "application name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    ApplicationImpl *application = dynamic_cast<ApplicationImpl *>(
        getOps()->getNotifyable(
            parent,
            ClusterlibStrings::REGISTERED_APPLICATION_NAME, 
            components.at(elements - 1),
            accessType));
    if (dynamic_cast<Root *>(parent) == NULL) {
        parent->releaseRef();
    }

    return application;
}

};	/* End of 'namespace clusterlib' */
