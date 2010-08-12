/*
 * registeredqueueimpl.cc
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
using namespace boost;

namespace clusterlib {

const string &
RegisteredQueueImpl::registeredName() const
{
    return ClusterlibStrings::REGISTERED_QUEUE_NAME;
}

string
RegisteredQueueImpl::generateKey(const string &parentKey, 
                                 const string &name) const
{
    return NotifyableKeyManipulator::createQueueKey(parentKey, name);
}

bool
RegisteredQueueImpl::isValidName(const string &name) const
{
    TRACE(CL_LOG, "isValidName");

    if ((name.compare(ClusterlibStrings::DEFAULT_RECV_QUEUE) != 0) &&
        (name.compare(ClusterlibStrings::DEFAULT_RESP_QUEUE) != 0) &&
        (name.compare(ClusterlibStrings::DEFAULT_COMPLETED_QUEUE) != 0) &&
        !NotifyableKeyManipulator::isValidNotifyableName(name)) {
        LOG_WARN(CL_LOG,
                 "isValidName: Illegal Queue name %s",
                 name.c_str());
        return false;
    }
    else {
        return true;
    }
}

shared_ptr<NotifyableImpl>
RegisteredQueueImpl::createNotifyable(const string &notifyableName,
                                      const string &notifyableKey,
                                      const shared_ptr<NotifyableImpl> &parent,
                                      FactoryOps &factoryOps) const
{
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<QueueImpl>(new QueueImpl(&factoryOps,
                                            notifyableKey,
                                            notifyableName,
                                            parent)));
}

vector<string>
RegisteredQueueImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        NotifyableKeyManipulator::createQueueParentKey(notifyableKey));

    return resVec;
}

bool
RegisteredQueueImpl::isValidKey(const vector<string> &components,
                                int32_t elements)
{
    TRACE(CL_LOG, "isValidKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isValidKey: elements %" PRId32
                  " > size of components %" PRIuPTR,
                  elements,
                  components.size());
        throw InvalidArgumentsException(
            "isValidKey: elements > size of components");
    }
    
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a Queue and that
     * after the Application key there are an even number of elements
     * left.
     */
    if ((elements < ClusterlibInts::QUEUE_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent notifyable are valid.
     */
    vector<string> nameVec;
    nameVec.push_back(ClusterlibStrings::REGISTERED_ROOT_NAME);
    nameVec.push_back(ClusterlibStrings::REGISTERED_GROUP_NAME);
    nameVec.push_back(ClusterlibStrings::REGISTERED_DATADISTRIBUTION_NAME);
    nameVec.push_back(ClusterlibStrings::REGISTERED_NODE_NAME);
    nameVec.push_back(ClusterlibStrings::REGISTERED_PROCESSSLOT_NAME);
    if (!getOps()->isValidKey(nameVec, components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is QUEUES and
     * that the queue name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::QUEUES) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

}	/* End of 'namespace clusterlib' */
