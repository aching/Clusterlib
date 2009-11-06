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
#include <sys/types.h>
#include <linux/unistd.h>

#define LOG_LEVEL LOG_WARN

using namespace std;
using namespace boost;

namespace clusterlib
{

/*
 * Key creation and recognition.
 */
string
NotifyableKeyManipulator::createLocksKey(const string &notifyableKey)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::LOCKS);

    return res;
}

string
NotifyableKeyManipulator::createLockKey(const string &notifyableKey,
                                        const string &lockName)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::LOCKS);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(lockName);

    return res;
}

string
NotifyableKeyManipulator::createLockNodeKey(const string &notifyableKey,
                                            const string &lockName)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::LOCKS);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(lockName);
    res.append(ClusterlibStrings::KEYSEPARATOR);

    const int32_t bufLen = 128;
    char tmp[bufLen + 1];
    tmp[bufLen] = '\0';
    if (gethostname(tmp, bufLen) != 0) {
        throw SystemFailureException("acquire: gethostname failed");
    }
    res.append(tmp);
    res.append(":");

    /*
     * Lock nodes have the hostname, pid, and tid of the calling
     * thread.
     */
    snprintf(tmp, bufLen, "0x%x", (uint32_t) getpid());
    res.append(tmp);
    res.append("-");
    snprintf(tmp, bufLen, "0x%x", (uint32_t) pthread_self());
    res.append(tmp);
    
    res.append(ClusterlibStrings::BID_SPLIT);

    return res;
}
                                            
string
NotifyableKeyManipulator::createNodeKey(const string &groupKey,
                                        const string &nodeName)
{
    string res;
    res.append(groupKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::NODES);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(nodeName);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotKey(
    const string &nodeKey,
    const string &processSlotName)
{
    string res;
    res.append(nodeKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTS);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(processSlotName);

    return res;
}

string
NotifyableKeyManipulator::createGroupKey(const string &groupKey,
                                         const string &groupName)
{
    string res;
    res.append(groupKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::GROUPS);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(groupName);

    return res;
}

string
NotifyableKeyManipulator::createApplicationKey(const string &appName)
{
    string res;
        res.append(ClusterlibStrings::ROOTNODE);
        res.append(ClusterlibStrings::CLUSTERLIB);
        res.append(ClusterlibStrings::KEYSEPARATOR);
        res.append(ClusterlibStrings::CLUSTERLIBVERSION);
        res.append(ClusterlibStrings::KEYSEPARATOR);
        res.append(ClusterlibStrings::ROOT);
        res.append(ClusterlibStrings::KEYSEPARATOR);
        res.append(ClusterlibStrings::APPLICATIONS);
        res.append(ClusterlibStrings::KEYSEPARATOR);
        res.append(appName);

    return res;
}

string
NotifyableKeyManipulator::createRootKey()
{
    string res;
        res.append(ClusterlibStrings::ROOTNODE);
        res.append(ClusterlibStrings::CLUSTERLIB);
        res.append(ClusterlibStrings::KEYSEPARATOR);
        res.append(ClusterlibStrings::CLUSTERLIBVERSION);
        res.append(ClusterlibStrings::KEYSEPARATOR);
        res.append(ClusterlibStrings::ROOT);

    return res;
}

string
NotifyableKeyManipulator::createDataDistributionKey(const string &groupKey,
                                                    const string &distName)
{
    string res;
    res.append(groupKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::DISTRIBUTIONS);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(distName);

    return res;
}

string
NotifyableKeyManipulator::createPropertyListKey(const string &notifyableKey,
                                                const string &propListName)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROPERTYLIST);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(propListName);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotsUsageKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTSUSAGE);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotsMaxKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTSMAX);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotPortVecKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTPORTVEC);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotExecArgsKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTEXECARGS);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotRunningExecArgsKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTRUNNINGEXECARGS);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotPIDKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTPID);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotDesiredStateKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTDESIREDSTATE);

    return res;
}


string
NotifyableKeyManipulator::createProcessSlotCurrentStateKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTCURRENTSTATE);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotReservationKey(
    const string &notifyableKey)
{
    string res(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTRESERVATION);

    return res;
}

string
NotifyableKeyManipulator::createSyncEventKey(const int64_t &syncEventId)
{
    ostringstream oss;
    oss << syncEventId;
    return oss.str();
}

string
NotifyableKeyManipulator::getNotifyableKeyFromKey(const string &key)
{
    TRACE(CL_LOG, "getNotifyableKeyFromKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));

    if (static_cast<int32_t>(components.size()) < 
        ClusterlibInts::ROOT_COMPONENTS_COUNT) {
        return string();
    }
        
    /* 
     * Check if this key already matches a possible Zookeeper node
     * that represents a Notifyable.  Also, strip off one path section
     * and check again if that fails.  
     *
     * The current layout of Clusterlib objects in Zookeeper limits
     * any object to having only one layer beneath them as part of
     * that object.  If that policy changes, this function will have
     * to change.
     */
    if (NotifyableKeyManipulator::isNotifyableKey(components)) {
        LOG_DEBUG(CL_LOG,
                  "getNotifyableKeyFromKey: From key (%s), returned same key)",
                  key.c_str());
        return key;
    }
    if (NotifyableKeyManipulator::isNotifyableKey(components, 
                                                  components.size() -1)) {
        string res = key;
        uint32_t keySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR);
        if (keySeparator == string::npos) {
            LOG_ERROR(CL_LOG,
                      "getNotifyableKeyFromKey: Couldn't find key "
                      "separator in key (%s)", key.c_str());
            throw InconsistentInternalStateException(
                "getNotifyableKeyFromKey: Couldn't find key separator");
        }
        res.erase(keySeparator);
        LOG_DEBUG(CL_LOG,
                  "getNotifyableKeyFromKey: From key (%s), "
                  "returned stripped key (%s))",
                  key.c_str(),
                  res.c_str());

        return res;
    }

    return string();
}

void NotifyableKeyManipulator::splitNotifyableKey(const string &key,
                                                  vector<string> &components)
{
    TRACE(CL_LOG, "splitKey");

    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
}

bool
NotifyableKeyManipulator::isValidNotifyableName(const string &name)
{
    TRACE(CL_LOG, "isValidNotifyableName");

    if ((name.empty()) || 
        (name.find(ClusterlibStrings::KEYSEPARATOR) != string::npos) ||
        (name[0] == '_')) {
        return false;
    }
        
    return true;
}

bool
NotifyableKeyManipulator::isNotifyableKey(const vector<string> &components,
                                          int32_t elements)
{
    TRACE(CL_LOG, "isNotifyableKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isNotifyableKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw InvalidArgumentsException("isNotifyableKey: elements > size of "
                                        "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    /* Check all the clusterlib objects, otherwise return false. */
    if (isApplicationKey(components, elements)) {
        return true;
    }
    if (isRootKey(components, elements)) {
        return true;
    }
    if (isDataDistributionKey(components, elements)) {
        return true;
    }
    if (isGroupKey(components, elements)) {
        return true;
    }
    if (isPropertyListKey(components, elements)) {
        return true;
    }
    if (isNodeKey(components, elements)) {
        return true;
    }
    if (isProcessSlotKey(components, elements)) {
        return true;
    }

    return false;
}

bool
NotifyableKeyManipulator::isNotifyableKey(const string &key)
{
    TRACE(CL_LOG, "isNotifyableKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isNotifyableKey(components);
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
        throw InvalidArgumentsException("isApplicationKey: elements > size of "
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

bool
NotifyableKeyManipulator::isApplicationKey(const string &key)
{
    TRACE(CL_LOG, "isApplicationKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isApplicationKey(components);
}

bool
NotifyableKeyManipulator::isRootKey(const vector<string> &components,
                                    int32_t elements)
{
    TRACE(CL_LOG, "isRootKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isRootKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw InvalidArgumentsException("isRootKey: elements > size of "
                                        "components");
    }

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    if (elements != ClusterlibInts::ROOT_COMPONENTS_COUNT) {
        return false;
    }

    if ((components.at(ClusterlibInts::CLUSTERLIB_INDEX) != 
         ClusterlibStrings::CLUSTERLIB) ||
        (components.at(ClusterlibInts::ROOT_INDEX) != 
         ClusterlibStrings::ROOT)) {
        return false;
    } 

    return true;    
}

bool
NotifyableKeyManipulator::isRootKey(const string &key)
{
    TRACE(CL_LOG, "isRootKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isRootKey(components);
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
        throw InvalidArgumentsException(
            "isDataDistributionKey: elements > size of components");
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
        throw InvalidArgumentsException("isGroupKey: elements > size of "
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
NotifyableKeyManipulator::isPropertyListKey(const vector<string> &components, 
                                          int32_t elements)
{
    TRACE(CL_LOG, "isPropertyListKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isPropertyListKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw InvalidArgumentsException(
            "isPropertyListKey: elements > size of components");
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
    if ((!isRootKey(components, elements - 2)) &&
        (!isGroupKey(components, elements - 2)) &&
        (!isDataDistributionKey(components, elements - 2)) &&
        (!isNodeKey(components, elements - 2)) &&
        (!isProcessSlotKey(components, elements - 2))) {
        return false; 
    }

    /*
     * Check that the second to the last element is PROPERTYLIST and
     * that the properites name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::PROPERTYLIST) ||
        (components.at(elements - 1).empty() == true)) {
        return false;
    } 

    return true;    
}

bool
NotifyableKeyManipulator::isPropertyListKey(const string &key)
{
    TRACE(CL_LOG, "isPropertyListKey");
    
    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isPropertyListKey(components);    
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
        throw InvalidArgumentsException("isNodeKey: elements > size of "
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
     * Check that the second to the last element is NODES and
     * that the distribution name is not empty.
     */
    if ((components.at(elements - 2) != ClusterlibStrings::NODES) ||
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

bool
NotifyableKeyManipulator::isProcessSlotKey(const vector<string> &components, 
                                           int32_t elements)
{
    TRACE(CL_LOG, "isProcessSlotKey");

    if (elements > static_cast<int32_t>(components.size())) {
        LOG_FATAL(CL_LOG,
                  "isProcessSlotKey: elements %d > size of components %u",
                  elements,
                  components.size());
        throw InvalidArgumentsException("isProcessSlotKey: elements > size of "
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
    if (!isNodeKey(components, elements - 2)) {
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

bool
NotifyableKeyManipulator::isProcessSlotKey(const string &key)
{
    TRACE(CL_LOG, "isProcessSlotKey");

    vector<string> components;
    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
    return isProcessSlotKey(components);    
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
            LOG_ERROR(CL_LOG,
                      "removeObjectFromKey: Couldn't find last separator for "
                      " key %s",
                      res.c_str());
            return string();
        }
        res.erase(endKeySeparator);

        /*
         * If this key represents a valid Notifyable, then it should
         * be /APPLICATIONS|GROUPS|NODES|PROCESSSLOTS|
         *     DISTRIBUTIONS|PROPERTYLIST/name.
         */
        endKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR);
        if ((endKeySeparator == string::npos) ||
            (endKeySeparator == 0)) {
            LOG_ERROR(CL_LOG,
                      "removeObjectFromKey: Couldn't find second to last "
                      "separator for key %s",
                      res.c_str());
            return string();
        }
        beginKeySeparator = res.rfind(ClusterlibStrings::KEYSEPARATOR, 
                                      endKeySeparator - 1);
        if ((beginKeySeparator == string::npos) ||
            (beginKeySeparator == 0)) {
            LOG_ERROR(CL_LOG,
                      "removeObjectFromKey: Couldn't find third to last "
                      "separator for key %s",
                      res.c_str());
            return string();
        }
        
        /* 
         * Try to find a clusterlib object in this portion of the key 
         */
        string rootObject = res.substr(endKeySeparator + 1);
        string clusterlibObject = 
            res.substr(beginKeySeparator + 1, 
                       endKeySeparator - beginKeySeparator - 1);
        if ((!rootObject.compare(ClusterlibStrings::ROOT)) ||
            (!clusterlibObject.compare(ClusterlibStrings::APPLICATIONS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::GROUPS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::NODES)) ||
            (!clusterlibObject.compare(ClusterlibStrings::PROCESSSLOTS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::DISTRIBUTIONS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::PROPERTYLIST))) {
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

    /* 
     * Set to the full size of the vector.
     */
    if (elements == -1) {
        elements = components.size();
    }

    int32_t clusterlibObjectElements = elements;
    bool objectFound = false;
    while (objectFound == false) {
        /*
         * Get rid of the leaf node 
         */
        clusterlibObjectElements--;
        if (clusterlibObjectElements < 2) {
            return -1;
        }

        /*
         * If this key represents a valid Notifyable, then it should
         * be
         * /(APPLICATIONS|GROUPS|NODES|DISTRIBUTIONS|PROPERTYLIST)/name
         * or ROOT.  Try to find a clusterlib object in this component
         */
        if ((components.at(clusterlibObjectElements - 1).compare(
                 ClusterlibStrings::ROOT) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                ClusterlibStrings::APPLICATIONS) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                ClusterlibStrings::GROUPS) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                ClusterlibStrings::NODES) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                ClusterlibStrings::PROCESSSLOTS) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                ClusterlibStrings::DISTRIBUTIONS) == 0)||
            (components.at(clusterlibObjectElements - 2).compare(
                ClusterlibStrings::PROPERTYLIST) == 0)) {
            objectFound = true;
        }
    }

    LOG_DEBUG(CL_LOG, 
              "removeObjectFromComponents: Changed key from %s/%s to %s/%s",
              components.at(elements - 2).c_str(),
              components.at(elements - 1).c_str(),
              components.at(clusterlibObjectElements - 2).c_str(),
              components.at(clusterlibObjectElements - 1).c_str());

    return clusterlibObjectElements;
}

};	/* End of 'namespace clusterlib' */
