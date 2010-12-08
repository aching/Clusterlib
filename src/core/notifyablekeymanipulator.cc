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

using namespace std;
using namespace boost;

namespace clusterlib {

string
NotifyableKeyManipulator::createLocksKey(const string &notifyableKey)
{
    string res;
    res.append(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::LOCK_DIR);

    return res;
}

string
NotifyableKeyManipulator::createLockKey(const string &notifyableKey,
                                        const string &lockName)
{
    string res;
    res.append(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::LOCK_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(lockName);

    return res;
}

string
NotifyableKeyManipulator::createLockNodeKey(
    const string &notifyableKey,
    const string &lockName,
    DistributedLockType distributedLockType)
{
    string res;
    res.append(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::LOCK_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(lockName);
    res.append(CLString::KEY_SEPARATOR);
    res.append(ProcessThreadService::getHostnamePidTid());
    res.append(CLStringInternal::SEQUENCE_SPLIT);
    res.append(distributedLockTypeToString(distributedLockType));
    res.append(CLStringInternal::SEQUENCE_SPLIT);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotKey(
    const string &nodeKey,
    const string &processSlotName)
{
    string res;
    res.append(nodeKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::PROCESSSLOT_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(processSlotName);

    return res;
}

string
NotifyableKeyManipulator::createGroupKey(const string &groupKey,
                                         const string &groupName)
{
    string res;
    res.append(groupKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::GROUP_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(groupName);

    return res;
}

string
NotifyableKeyManipulator::createPropertyListChildrenKey(
    const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::PROPERTYLIST_DIR);

    return res;
}

string
NotifyableKeyManipulator::createQueueChildrenKey(
    const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::QUEUE_DIR);

    return res;
}

string
NotifyableKeyManipulator::createRootKey()
{
    string res;
    res.append(CLStringInternal::ROOT_ZNODE);
    res.append(CLStringInternal::CLUSTERLIB);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::CLUSTERLIB_VERSION);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::ROOT_DIR);
    
    return res;
}

string
NotifyableKeyManipulator::createApplicationChildrenKey(const string &rootKey)
{
    string res;
    res.append(rootKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::APPLICATION_DIR);

    return res;
}

string
NotifyableKeyManipulator::createApplicationKey(const string &appName)
{
    string res;
        res.append(CLStringInternal::ROOT_ZNODE);
        res.append(CLStringInternal::CLUSTERLIB);
        res.append(CLString::KEY_SEPARATOR);
        res.append(CLStringInternal::CLUSTERLIB_VERSION);
        res.append(CLString::KEY_SEPARATOR);
        res.append(CLString::ROOT_DIR);
        res.append(CLString::KEY_SEPARATOR);
        res.append(CLString::APPLICATION_DIR);
        res.append(CLString::KEY_SEPARATOR);
        res.append(appName);

    return res;
}

string
NotifyableKeyManipulator::createGroupChildrenKey(const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::GROUP_DIR);

    return res;
}

string
NotifyableKeyManipulator::createDataDistributionChildrenKey(
    const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::DATADISTRIBUTION_DIR);

    return res;
}

string
NotifyableKeyManipulator::createNodeChildrenKey(const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::NODE_DIR);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotChildrenKey(const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::PROCESSSLOT_DIR);

    return res;
}
                                            
string
NotifyableKeyManipulator::createNodeKey(const string &groupKey,
                                        const string &nodeName)
{
    string res;
    res.append(groupKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::NODE_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(nodeName);

    return res;
}

string
NotifyableKeyManipulator::createJSONObjectKey(const string &notifyableKey)
{
    string res;
    res.append(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::DEFAULT_JSON_OBJECT);

    return res;
}

string
NotifyableKeyManipulator::createDataDistributionKey(const string &groupKey,
                                                    const string &distName)
{
    string res;
    res.append(groupKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::DATADISTRIBUTION_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(distName);

    return res;
}

string
NotifyableKeyManipulator::createPropertyListKey(const string &notifyableKey,
                                                const string &propListName)
{
    string res;
    res.append(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::PROPERTYLIST_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(propListName);

    return res;
}

string
NotifyableKeyManipulator::createQueueKey(const string &notifyableKey,
                                         const string &queueName)
{
    string res;
    res.append(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLString::QUEUE_DIR);
    res.append(CLString::KEY_SEPARATOR);
    res.append(queueName);

    return res;
}

string
NotifyableKeyManipulator::createQueuePrefixKey(
    const string &notifyableKey)
{
    string res = NotifyableKeyManipulator::createQueueParentKey(notifyableKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::QUEUE_ELEMENT_PREFIX);

    return res;
}

string
NotifyableKeyManipulator::createQueueParentKey(
    const string &queueKey)
{
    string res(queueKey);
    res.append(CLString::KEY_SEPARATOR);
    res.append(CLStringInternal::QUEUE_PARENT);
   
    return res;
}

string
NotifyableKeyManipulator::createSyncEventKey(const int64_t &syncEventId)
{
    ostringstream oss;
    oss << syncEventId;
    return oss.str();
}

void NotifyableKeyManipulator::splitNotifyableKey(const string &key,
                                                  vector<string> &components)
{
    TRACE(CL_LOG, "splitKey");

    split(components, key, is_any_of(CLString::KEY_SEPARATOR));
}

bool
NotifyableKeyManipulator::isValidNotifyableName(const string &name)
{
    TRACE(CL_LOG, "isValidNotifyableName");

    /* 
     * Check for empty string, the key separater, or the first
     * char as _. 
     */
    if ((name.empty()) || 
        (name.find(CLString::KEY_SEPARATOR) != string::npos) ||
        (name[0] == '_')) {
        return false;
    }

    /* Must be all printable characters */
    for (size_t i = 0; i < name.size(); i++) {
        if ((name[i] < 32) || (name[i] > 126)) {
            return false;
        }
    }
        
    return true;
}

string 
NotifyableKeyManipulator::removeObjectFromKey(const string &key)
{
    TRACE(CL_LOG, "removeObjectFromKey");

    if (key.empty() ||
        (key.substr(key.size() - 1) == 
         CLString::KEY_SEPARATOR)) {
        return string();
    }

    string res = key;
    bool objectFound = false;
    size_t beginKeySeparator = string::npos;
    size_t endKeySeparator = string::npos;
    while (objectFound == false) {
        /*
         * Get rid of the leaf node 
         */
        endKeySeparator = res.rfind(CLString::KEY_SEPARATOR);
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
         *     DISTRIBUTIONS|PROPERTYLISTS|QUEUE/name.
         */
        endKeySeparator = res.rfind(CLString::KEY_SEPARATOR);
        if ((endKeySeparator == string::npos) ||
            (endKeySeparator == 0)) {
            LOG_ERROR(CL_LOG,
                      "removeObjectFromKey: Couldn't find second to last "
                      "separator for key %s",
                      res.c_str());
            return string();
        }
        beginKeySeparator = res.rfind(CLString::KEY_SEPARATOR, 
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
        if ((!rootObject.compare(CLString::ROOT_DIR)) ||
            (!clusterlibObject.compare(CLString::APPLICATION_DIR)) ||
            (!clusterlibObject.compare(CLString::GROUP_DIR)) ||
            (!clusterlibObject.compare(CLString::NODE_DIR)) ||
            (!clusterlibObject.compare(CLString::PROPERTYLIST_DIR)) ||
            (!clusterlibObject.compare(CLString::DATADISTRIBUTION_DIR)) ||
            (!clusterlibObject.compare(CLString::PROCESSSLOT_DIR)) ||
            (!clusterlibObject.compare(CLString::QUEUE_DIR))) {
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
        (components.back() == CLString::KEY_SEPARATOR)) {
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
         * /(APPLICATIONS|GROUPS|NODES|DISTRIBUTIONS|
         *   PROPERTYLISTSS|QUEUES)/name
         * or ROOT.  Try to find a clusterlib object in this component
         */
        if ((components.at(clusterlibObjectElements - 1).compare(
                 CLString::ROOT_DIR) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                CLString::APPLICATION_DIR) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                CLString::GROUP_DIR) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                CLString::NODE_DIR) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                CLString::PROCESSSLOT_DIR) == 0) ||
            (components.at(clusterlibObjectElements - 2).compare(
                CLString::DATADISTRIBUTION_DIR) == 0)||
            (components.at(clusterlibObjectElements - 2).compare(
                CLString::PROPERTYLIST_DIR) == 0)||
            (components.at(clusterlibObjectElements - 2).compare(
                CLString::QUEUE_DIR) == 0)) {
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

string
NotifyableKeyManipulator::removeComponentFromKey(const string &key)
{
    size_t keySeparator = key.rfind(CLString::KEY_SEPARATOR);
    if (keySeparator == string::npos) {
        return string();
    }
    return key.substr(0, keySeparator - 1); 
}

}	/* End of 'namespace clusterlib' */
