/*
 * registeredprocessslotimpl.cc
 *
 * Implementation of the RegisteredProcessSlotImpl class.
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
RegisteredProcessSlotImpl::registeredName() const
{
    return ClusterlibStrings::REGISTERED_PROCESSSLOT_NAME;
}

string
RegisteredProcessSlotImpl::generateKey(const string &parentKey, 
                                 const string &name) const
{
    return NotifyableKeyManipulator::createProcessSlotKey(parentKey, name);
}

NotifyableImpl *
RegisteredProcessSlotImpl::createNotifyable(const string &notifyableName,
                                            const string &notifyableKey,
                                            NotifyableImpl *parent,
                                            FactoryOps &factoryOps) const
{
    NodeImpl *node = dynamic_cast<NodeImpl *>(parent);
    if (node == NULL) {
        throw InvalidArgumentsException("createNotifyable: Got impossible "
                                        "parent that is not a Node");
    }
    return new ProcessSlotImpl(&factoryOps,
                               notifyableKey,
                               notifyableName,
                               node);
}

vector<string>
RegisteredProcessSlotImpl::generateRepositoryList(
    const std::string &notifyableName,
    const std::string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        ProcessSlotImpl::createProcessInfoJsonArrKey(notifyableKey));

    return resVec;
}

bool
RegisteredProcessSlotImpl::isValidKey(const vector<string> &components,
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
     * Make sure that we have enough elements to have a process slot
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::PROCESSSLOT_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent node are valid.
     */
    vector<string> nameVec;
    nameVec.push_back(ClusterlibStrings::REGISTERED_NODE_NAME);
    if (!getOps()->isValidKey(nameVec, components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is PROCESSSLOTS
     * that the distribution name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::PROCESSSLOTS) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
    
}

NotifyableImpl *
RegisteredProcessSlotImpl::getObjectFromComponents(
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
    RegisteredNotifyable *regNode = getOps()->getRegisteredNotifyable(
        ClusterlibStrings::REGISTERED_NODE_NAME, true);
    NodeImpl *parent = dynamic_cast<NodeImpl *>(
        regNode->getObjectFromComponents(components,
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
              "process slot name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    ProcessSlotImpl *processSlot = dynamic_cast<ProcessSlotImpl *>(
        getOps()->getNotifyable(parent,
                                ClusterlibStrings::REGISTERED_PROCESSSLOT_NAME,
                                components.at(elements - 1),
                                accessType));
    if (dynamic_cast<Root *>(parent) == NULL) {
        parent->releaseRef();
    }

    return processSlot;
}

};	/* End of 'namespace clusterlib' */
