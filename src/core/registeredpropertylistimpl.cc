/*
 * registeredpropertylistimpl.cc
 *
 * Implementation of the RegisteredPropertyListImpl class.
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
RegisteredPropertyListImpl::registeredName() const
{
    return ClusterlibStrings::REGISTERED_PROPERTYLIST_NAME;
}

string
RegisteredPropertyListImpl::generateKey(const string &parentKey, 
                                        const string &name) const
{
    return NotifyableKeyManipulator::createPropertyListKey(parentKey, name);
}

bool
RegisteredPropertyListImpl::isValidName(const string &name) const
{
    TRACE(CL_LOG, "isValidName");

    if ((name.compare(ClusterlibStrings::DEFAULTPROPERTYLIST)) &&
        (!NotifyableKeyManipulator::isValidNotifyableName(name))) {
        LOG_WARN(CL_LOG,
                 "isValidName: Illegal PropertyList name %s",
                 name.c_str());
        return false;
    }
    else {
        return true;
    }
}

NotifyableImpl *
RegisteredPropertyListImpl::createNotifyable(const string &notifyableName,
                                             const string &notifyableKey,
                                             NotifyableImpl *parent,
                                             FactoryOps &factoryOps) const
{
    return new PropertyListImpl(&factoryOps,
                                notifyableKey,
                                notifyableName,
                                parent);
}

vector<string>
RegisteredPropertyListImpl::generateRepositoryList(
    const std::string &notifyableName,
    const std::string &notifyableKey) const
{
    vector<string> resVec;
    resVec.push_back(notifyableKey);
    resVec.push_back(
        PropertyListImpl::createKeyValJsonObjectKey(notifyableKey));

    return resVec;
}

bool
RegisteredPropertyListImpl::isValidKey(const vector<string> &components,
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
     * Make sure that we have enough elements to have a property list
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::PROP_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent notifyable are valid.
     */
    vector<string> nameVec;
    if (!getOps()->isValidKey(nameVec, components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is PROPERTYLISTS and
     * that the property list name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::PROPERTYLISTS) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;        
}

NotifyableImpl *
RegisteredPropertyListImpl::getObjectFromComponents(
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
    NotifyableImpl *parent = getOps()->getNotifyableFromComponents(
        vector<string>(), components, parentGroupCount, accessType);
    if (parent == NULL) {
        LOG_WARN(CL_LOG, "getObjectFromComponents: Tried to get "
                 "parent with name %s",
                 components.at(parentGroupCount - 1).c_str());
        return NULL;
    }

    LOG_DEBUG(CL_LOG, 
              "getObjectFromComponents: parent key = %s, "
              "property list name = %s", 
              parent->getKey().c_str(),
              components.at(elements - 1).c_str());

    PropertyListImpl *propertyList = dynamic_cast<PropertyListImpl *>(
        getOps()->getNotifyable(
            parent,
            ClusterlibStrings::REGISTERED_PROPERTYLIST_NAME,
            components.at(elements - 1),
            accessType));
    if (dynamic_cast<Root *>(parent) == NULL) {
        parent->releaseRef();
    }
    return propertyList;
}

};	/* End of 'namespace clusterlib' */
