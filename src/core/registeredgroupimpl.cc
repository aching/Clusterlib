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
using namespace boost;

namespace clusterlib {

const string &
RegisteredGroupImpl::registeredName() const
{
    return CLString::REGISTERED_GROUP_NAME;
}

string
RegisteredGroupImpl::generateKey(const string &parentKey, 
                                 const string &name) const
{
    return NotifyableKeyManipulator::createGroupKey(parentKey, name);
}

shared_ptr<NotifyableImpl>
RegisteredGroupImpl::createNotifyable(const string &notifyableName,
                                      const string &notifyableKey,
                                      const shared_ptr<NotifyableImpl> &parent,
                                      FactoryOps &factoryOps) const
{
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<GroupImpl> (new GroupImpl(&factoryOps,
                                             notifyableKey,
                                             notifyableName,
                                             parent)));
}

vector<string>
RegisteredGroupImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
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
    if ((elements < CLNumericInternal::GROUP_COMPONENTS_MIN_COUNT) ||
        (((elements - CLNumericInternal::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent groups (recursively) and
     * application are valid.
     */
    RegisteredNotifyable *regApplication = NULL;
    if (elements == CLNumericInternal::APP_COMPONENTS_COUNT) {
        regApplication = getOps()->getRegisteredNotifyable(
            CLString::REGISTERED_APPLICATION_NAME, true);
        if (!regApplication->isValidKey(components, elements)) {
            return false;
        }
    }
    else if (elements >= CLNumericInternal::APP_COMPONENTS_COUNT + 2) {
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
    if (((components.at(elements - 2) != CLString::APPLICATION_DIR) &&
         (components.at(elements - 2) != CLString::GROUP_DIR)) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

bool
RegisteredGroupImpl::getObjectFromComponents(
    const vector<string> &components,
    int32_t elements, 
    AccessType accessType,
    int64_t msecTimeout,
    shared_ptr<NotifyableImpl> *pNotifyableSP)
{
    TRACE(CL_LOG, "getObjectFromComponents");

    if (pNotifyableSP == NULL) {
        throw InvalidArgumentsException(
            "getObjectFromComponents: NULL pNotifyableSP");
    }
    pNotifyableSP->reset();

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
    RegisteredNotifyable *regApplication = getOps()->getRegisteredNotifyable(
        CLString::REGISTERED_APPLICATION_NAME, true);
    if (regApplication->isValidKey(components, elements)) {
        return regApplication->getObjectFromComponents(
            components, elements, accessType, msecTimeout, pNotifyableSP);
    }
    
    if (!isValidKey(components, elements)) {
        LOG_DEBUG(CL_LOG, 
                  "getObjectFromComponents: Couldn't find key"
                  " with %d elements",
                  elements);
        return false;
    }

    int32_t parentGroupCount = 
        NotifyableKeyManipulator::removeObjectFromComponents(components, 
                                                             elements);
    if (parentGroupCount == -1) {
        return false;
    }

    shared_ptr<NotifyableImpl> parentSP;
    bool finished = getObjectFromComponents(components,
                                            parentGroupCount,
                                            accessType,
                                            msecTimeout,
                                            &parentSP);
    if (finished == false) {
        LOG_DEBUG(CL_LOG,
                  "getObjectFromComponents: Didn't finish with msecTimeout=%" 
                  PRId64,
                  msecTimeout);
        return false;
    }

    if (parentSP == NULL) {
        LOG_WARN(CL_LOG, "getObjectFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        pNotifyableSP->reset();
        return true;
    }
    
    LOG_DEBUG(CL_LOG, 
              "getObjectFromComponents: parent key = %s, group name = %s", 
              parentSP->getKey().c_str(),
              components.at(elements - 1).c_str());

    return getOps()->getNotifyableWaitMsecs(
        parentSP,
        CLString::REGISTERED_GROUP_NAME,
        components.at(elements - 1),
        accessType,
        msecTimeout,
        pNotifyableSP);
}

}	/* End of 'namespace clusterlib' */
