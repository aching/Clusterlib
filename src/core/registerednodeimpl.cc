/*
 * registerednodeimpl.cc
 *
 * Implementation of the RegisteredQueueImpl class.
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
RegisteredNodeImpl::registeredName() const
{
    return ClusterlibStrings::REGISTERED_NODE_NAME;
}

string
RegisteredNodeImpl::generateKey(const string &parentKey, 
                                const string &name) const
{
    return NotifyableKeyManipulator::createNodeKey(parentKey, name);
}

NotifyableImpl *
RegisteredNodeImpl::createNotifyable(const string &notifyableName,
                                     const string &notifyableKey,
                                     NotifyableImpl *parent,
                                     FactoryOps &factoryOps) const
{
    GroupImpl *group = dynamic_cast<GroupImpl *>(parent);
    if (group == NULL) {
        throw InvalidArgumentsException("createNotifyable: Got impossible "
                                        "parent that is not a group");
    }
    return new NodeImpl(&factoryOps,
                        notifyableKey,
                        notifyableName,
                        group);
}

vector<string>
RegisteredNodeImpl::generateRepositoryList(
    const std::string &notifyableName,
    const std::string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        NotifyableKeyManipulator::createJSONObjectKey(
            notifyableKey));
    resVec.push_back(
        NodeImpl::createProcessSlotInfoJSONObjectKey(
            notifyableKey));

    return resVec;
}

bool
RegisteredNodeImpl::isValidKey(const vector<string> &components,
                               int32_t elements)
{
    TRACE(CL_LOG, "isValidKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isValidKey: elements % " PRId32
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
     * Make sure that we have enough elements to have a node
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::NODE_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent group are valid.
     */
    RegisteredNotifyable *regGroup = NULL;
    regGroup = getOps()->getRegisteredNotifyable(
        ClusterlibStrings::REGISTERED_GROUP_NAME, true);
    if (!regGroup->isValidKey(components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is NODES and
     * that the distribution name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::NODES) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    

}

NotifyableImpl *
RegisteredNodeImpl::getObjectFromComponents(
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
    RegisteredNotifyable *regGroup = getOps()->getRegisteredNotifyable(
        ClusterlibStrings::REGISTERED_GROUP_NAME, true);
    GroupImpl *parent = dynamic_cast<GroupImpl *>(
        regGroup->getObjectFromComponents(components,
                                        parentGroupCount,
                                        accessType));
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getObjectFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getObjectFromComponents: parent key = %s, node name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    NodeImpl *node = dynamic_cast<NodeImpl *>(
        getOps()->getNotifyable(parent,
                                ClusterlibStrings::REGISTERED_NODE_NAME,
                                components.at(elements - 1),
                                accessType));
    if (dynamic_cast<Root *>(parent) == NULL) {
        parent->releaseRef();
    }

    return node;
}


};	/* End of 'namespace clusterlib' */
