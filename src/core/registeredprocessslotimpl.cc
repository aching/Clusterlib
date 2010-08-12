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
using namespace boost;

namespace clusterlib {

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

shared_ptr<NotifyableImpl>
RegisteredProcessSlotImpl::createNotifyable(
    const string &notifyableName,
    const string &notifyableKey,
    const shared_ptr<NotifyableImpl> &parent,
    FactoryOps &factoryOps) const
{
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<ProcessSlotImpl>(new ProcessSlotImpl(&factoryOps,
                                                        notifyableKey,
                                                        notifyableName,
                                                        parent)));
}

vector<string>
RegisteredProcessSlotImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
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

RegisteredProcessSlotImpl::RegisteredProcessSlotImpl(FactoryOps *factoryOps)
    : RegisteredNotifyableImpl(factoryOps)
{
    setRegisteredParentNameVec(
        vector<string>(1, ClusterlibStrings::REGISTERED_NODE_NAME));
}

}	/* End of 'namespace clusterlib' */
