/*
 * registeredgroupimpl.cc
 *
 * Implementation of the RegisteredGroupImpl class.
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
RegisteredGroupImpl::registeredName() const
{
    return ClusterlibStrings::REGISTERED_GROUP_NAME;
}

string
RegisteredGroupImpl::generateKey(const string &parentKey, 
                                 const string &name) const
{
    return NotifyableKeyManipulator::createGroupKey(parentKey, name);
}


NotifyableImpl *
RegisteredGroupImpl::createNotifyable(const string &notifyableName,
                                      const string &notifyableKey,
                                      NotifyableImpl *parent,
                                      FactoryOps &factoryOps) const
{
    return new GroupImpl(&factoryOps,
                         notifyableKey,
                         notifyableName,
                         parent);
}

vector<string>
RegisteredGroupImpl::generateRepositoryList(
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
RegisteredGroupImpl::isValidKey(const vector<string> &components,
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

    /*
     * Make sure that we have enough elements to have a group/app
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::GROUP_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent groups (recursively) and
     * application are valid.
     */
    RegisteredNotifyable *regApplication = NULL;
    if (elements == ClusterlibInts::APP_COMPONENTS_COUNT) {
        regApplication = getOps()->getRegisteredNotifyable(
            ClusterlibStrings::REGISTERED_APPLICATION_NAME, true);
        if (!regApplication->isValidKey(components, elements)) {
            return false;
        }
    }
    else if (elements >= ClusterlibInts::APP_COMPONENTS_COUNT + 2) {
        if (!isValidKey(components, elements - 2)) {
            return false;
        }
    }
    else {
        /*
         * Shouldn't happen.
         */
        return false;
    }

    /*
     * Check that the second to the last element is APPLICATIONS or GROUPS and
     * that the group name is not empty.
     */
    if (((components.at(elements - 2) != ClusterlibStrings::APPLICATIONS) &&
         (components.at(elements - 2) != ClusterlibStrings::GROUPS)) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

NotifyableImpl *
RegisteredGroupImpl::getObjectFromComponents(
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

    /* 
     * Could be either an Application or a Group, need to check both.
     * If is a Group and not an Application, then it must have a
     * parent that is either an Application or Group.  If the parent
     * is a Group, this function will call itself recursively.
     */
    RegisteredNotifyable *regApplication = NULL;
    regApplication = getOps()->getRegisteredNotifyable(
        ClusterlibStrings::REGISTERED_APPLICATION_NAME, true);
    if (regApplication->isValidKey(components, elements)) {
        return regApplication->getObjectFromComponents(
            components, elements, accessType);
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
    GroupImpl *parent = dynamic_cast<GroupImpl *>(
        getObjectFromComponents(components,
                                parentGroupCount,
                                accessType));
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getObjectFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }
    
    LOG_DEBUG(CL_LOG, 
              "getObjectFromComponents: parent key = %s, group name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    GroupImpl *group = dynamic_cast<GroupImpl *>(
        getOps()->getNotifyable(parent,
                                ClusterlibStrings::REGISTERED_GROUP_NAME,
                                components.at(elements - 1),
                                accessType));
    if (dynamic_cast<Root *>(parent) == NULL) {
        parent->releaseRef();
    }

    return group;
}

};	/* End of 'namespace clusterlib' */
