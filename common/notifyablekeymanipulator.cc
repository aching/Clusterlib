/*
 * notifyablekeymanipulatorclusterlib.cc --
 *
 * Implementation of the NotifyableKeyManipulator class.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"
#include "notifyablekeymanipulator.h"

#define LOG_LEVEL LOG_WARN

using namespace std;
using namespace boost;

namespace clusterlib
{

/*
 * Key creation and recognition.
 */
string
NotifyableKeyManipulator::createNodeKey(const string &groupKey,
                                        const string &nodeName,
                                        bool managed)
{
    string res =
        groupKey +
        ClusterlibStrings::KEYSEPARATOR +
        (managed ? 
         ClusterlibStrings::NODES : ClusterlibStrings::UNMANAGEDNODES) +
        ClusterlibStrings::KEYSEPARATOR +
        nodeName
        ;

    return res;
}

string
NotifyableKeyManipulator::createGroupKey(const string &groupKey,
                                         const string &groupName)
{
    string res = 
        groupKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::GROUPS +
        ClusterlibStrings::KEYSEPARATOR +
        groupName
        ;

    return res;
}

string
NotifyableKeyManipulator::createAppKey(const string &appName)
{
    string res =
        ClusterlibStrings::ROOTNODE +
        ClusterlibStrings::CLUSTERLIB +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::CLUSTERLIBVERSION +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::APPLICATIONS +
        ClusterlibStrings::KEYSEPARATOR +
        appName
        ;

    return res;
}

string
NotifyableKeyManipulator::createDistKey(const string &groupKey,
                                        const string &distName)
{
    string res =
        groupKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::DISTRIBUTIONS +
        ClusterlibStrings::KEYSEPARATOR +
        distName
        ;

    return res;
}

string
NotifyableKeyManipulator::createPropertiesKey(const string &notifyableKey)
{
    string res =
	notifyableKey +
        ClusterlibStrings::KEYSEPARATOR +
        ClusterlibStrings::PROPERTIES
	;

    return res;
}

bool
NotifyableKeyManipulator::isApplicationKey(const vector<string> &components,
                                           int32_t elements)
{
    TRACE(CL_LOG, "isApplicationKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isApplicationKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isApplicationKey: elements > size of "
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
        (components.at(ClusterlibInts::APP_INDEX) != 
         ClusterlibStrings::APPLICATIONS)) {
        return false;
    } 

    return true;    
}

bool
NotifyableKeyManipulator::isApplicationKey(const string &key)
{
    TRACE(CL_LOG, "isApplicationKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isApplicationKey(components);
}

bool
NotifyableKeyManipulator::isDataDistributionKey(
    const vector<string> &components, 
    int32_t elements)
{
    TRACE(CL_LOG, "isDataDistributionKey");
    
    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isDataDistributionKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isDataDistributionKey: elements > size of "
                               "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a distribution
     * and that after the Application key there are an even number of
     * elements left.
     */
    if ((elements < ClusterlibInts::DIST_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 0))  {
        return false;
    }

    /*
     * Check that the elements of the parent group are valid.
     */
    if (!isGroupKey(components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is DISTRIBUTIONS and
     * that the distribution name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::DISTRIBUTIONS) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

bool
NotifyableKeyManipulator::isDataDistributionKey(const string &key)
{
    TRACE(CL_LOG, "isDataDistributionKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isDataDistributionKey(components);    
}

bool
NotifyableKeyManipulator::isGroupKey(const vector<string> &components, 
                                     int32_t elements)
{
    TRACE(CL_LOG, "isGroupKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isGroupKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isGroupKey: elements > size of "
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
    if (elements == ClusterlibInts::APP_COMPONENTS_COUNT) {
        if (!isApplicationKey(components, elements)) {
            return false;
        }
    }
    else if (elements >= ClusterlibInts::APP_COMPONENTS_COUNT + 2) {
        if (!isGroupKey(components, elements - 2)) {
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

bool
NotifyableKeyManipulator::isGroupKey(const string &key)
{
    TRACE(CL_LOG, "isGroupKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isGroupKey(components);    
}

bool
NotifyableKeyManipulator::isPropertiesKey(const vector<string> &components, 
                                          int32_t elements)
{
    TRACE(CL_LOG, "isPropertiesKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isPropertiesKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isPropertiesKey: elements > size of "
                               "components");
    }
    
    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /*
     * Make sure that we have enough elements to have a properties
     * and that after the Application key there are an odd number of
     * elements left.
     */
    if ((elements < ClusterlibInts::PROP_COMPONENTS_MIN_COUNT) ||
        (((elements - ClusterlibInts::APP_COMPONENTS_COUNT) % 2) != 1))  {
        return false;
    }

    /*
     * Check that the each of the parent of the properties is a
     * Notifyable and that their parents are a group or application.
     */ 
    if (((components.at(elements - 3) != ClusterlibStrings::APPLICATIONS) &&
         (components.at(elements - 3) != ClusterlibStrings::GROUPS) &&
         (components.at(elements - 3) != ClusterlibStrings::DISTRIBUTIONS) &&
         (components.at(elements - 3) != ClusterlibStrings::UNMANAGEDNODES) &&
         (components.at(elements - 3) != ClusterlibStrings::NODES)) ||
        (components.at(elements - 1) != ClusterlibStrings::PROPERTIES)) {
        return false;
    }

    if (elements >= ClusterlibInts::APP_COMPONENTS_COUNT) {
        if ((!isGroupKey(components, elements - 1)) &&
            (!isDataDistributionKey(components, elements - 1)) &&
            (!isNodeKey(components, elements - 1))) {
            return false; 
        }
    }
    else {
        /*
         * Shouldn't happen.
         */
        return false;
    }
            
    return true;    
}

bool
NotifyableKeyManipulator::isPropertiesKey(const string &key)
{
    TRACE(CL_LOG, "isPropertiesKey");
    
    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isPropertiesKey(components);    
}

bool
NotifyableKeyManipulator::isNodeKey(const vector<string> &components, 
                                    int32_t elements)
{
    TRACE(CL_LOG, "isNodeKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isNodeKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw ClusterException("isNodeKey: elements > size of "
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
    if (!isGroupKey(components, elements - 2)) {
        return false;
    }

    /*
     * Check that the second to the last element is DISTRIBUTIONS and
     * that the distribution name is not empty.
     */
    if (((components.at(elements - 2) != ClusterlibStrings::NODES) &&
         (components.at(elements - 2) != ClusterlibStrings::UNMANAGEDNODES)) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

bool
NotifyableKeyManipulator::isNodeKey(const string &key)
{
    TRACE(CL_LOG, "isNodeKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isNodeKey(components);    
}

string 
NotifyableKeyManipulator::removeObjectFromKey(const string &key)
{
    TRACE(CL_LOG, "removeObjectFromKey");

    if (key.empty() ||
        (key.substr(key.size() - 1) == 
         ClusterlibStrings::ClusterlibStrings::KEYSEPARATOR)) {
        return string();
    }

    string res = key;
    bool objectFound = false;
    uint32_t beginKeySeparator = numeric_limits<uint32_t>::max();
    uint32_t endKeySeparator = numeric_limits<uint32_t>::max();
    while (objectFound == false) {
        /*
         * Get rid of the leaf node 
         */
        endKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR);
        if ((endKeySeparator == string::npos) ||
            (endKeySeparator == 0)) {
            return string();
        }
        res.erase(endKeySeparator);

        /*
         * If this key represents a valid Notifyable, then it should
         * be /APPLICATIONS|GROUPS|NODES|DISTRIBUTIONS/name.
         */
        endKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR);
        if ((endKeySeparator == string::npos) ||
            (endKeySeparator == 0)) {
            return string();
        }
        beginKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR, 
                                      endKeySeparator - 1);
        if ((beginKeySeparator == string::npos) ||
            (beginKeySeparator == 0)) {
            return string();
        }
        
        /* 
         * Try to find a clusterlib object in this portion of the key 
         */
        string clusterlibObject = 
            res.substr(beginKeySeparator + 1, 
                       endKeySeparator - beginKeySeparator - 1);
        if ((!clusterlibObject.compare(ClusterlibStrings::APPLICATIONS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::GROUPS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::NODES)) ||
            (!clusterlibObject.compare(ClusterlibStrings::DISTRIBUTIONS))) {
            objectFound = true;
        }
    }

    LOG_DEBUG(CL_LOG, 
              "removeObjectFromKey: Changed key %s to %s",
              key.c_str(), 
              res.c_str());

    return res;
}

int32_t 
NotifyableKeyManipulator::removeObjectFromComponents(
    const vector<string> &components,
    int32_t elements)
{
    TRACE(CL_LOG, "removeObjectFromComponents");

    if (components.empty() ||
        (components.back() == ClusterlibStrings::KEYSEPARATOR)) {
        return -1;
    }

    int32_t clusterlibObjectIndex = -1;
    bool objectFound = false;
    while (objectFound == false) {
        /*
         * Get rid of the leaf node 
         */
        elements--;
        if (elements < 2) {
            return -1;
        }

        /*
         * If this key represents a valid Notifyable, then it should
         * be /APPLICATIONS|GROUPS|NODES|DISTRIBUTIONS/name.
         */
        clusterlibObjectIndex = elements - 2;
        
        /* 
         * Try to find a clusterlib object in this component
         */
        if ((components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::APPLICATIONS) == 0) ||
            (components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::GROUPS) == 0) ||
            (components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::NODES) == 0) ||
            (components.at(clusterlibObjectIndex).compare(
                 ClusterlibStrings::DISTRIBUTIONS) == 0)) {
            objectFound = true;
        }
    }

    LOG_DEBUG(CL_LOG, 
              "removeObjectFromComponents: Changed key from %s/%s to %s/%s",
              components.at(elements - 2).c_str(),
              components.at(elements - 1).c_str(),
              components.at(clusterlibObjectIndex).c_str(),
              components.at(clusterlibObjectIndex + 1).c_str());

    /*
     * +2 since 1 for including the name of the clusterlib object and
     * 1 since this is the elements count and not the index.
     */
    return clusterlibObjectIndex + 2;
}

};	/* End of 'namespace clusterlib' */