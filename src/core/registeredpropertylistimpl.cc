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
using namespace boost;

namespace clusterlib {

const string &
RegisteredPropertyListImpl::registeredName() const
{
    return CLString::REGISTERED_PROPERTYLIST_NAME;
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

    if ((name.compare(CLString::DEFAULT_PROPERTYLIST)) &&
        (!NotifyableKeyManipulator::isValidNotifyableName(name))) {
        LOG_WARN(CL_LOG,
                 "isValidName: Illegal PropertyList name '%s'",
                 name.c_str());
        return false;
    }
    else {
        return true;
    }
}

shared_ptr<NotifyableImpl>
RegisteredPropertyListImpl::createNotifyable(
    const string &notifyableName,
    const string &notifyableKey,
    const shared_ptr<NotifyableImpl> &parent,
    FactoryOps &factoryOps) const
{
    return dynamic_pointer_cast<NotifyableImpl>(
        shared_ptr<NotifyableImpl>(new PropertyListImpl(&factoryOps,
                                                        notifyableKey,
                                                        notifyableName,
                                                        parent)));
}

vector<string>
RegisteredPropertyListImpl::generateRepositoryList(
    const string &notifyableName,
    const string &notifyableKey) const
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
    if ((elements < CLNumericInternal::PROP_COMPONENTS_MIN_COUNT) ||
        (((elements - CLNumericInternal::APP_COMPONENTS_COUNT) % 2) != 0))  {
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
    if ((components.at(elements - 2) != CLString::PROPERTYLIST_DIR) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;        
}

}	/* End of 'namespace clusterlib' */
