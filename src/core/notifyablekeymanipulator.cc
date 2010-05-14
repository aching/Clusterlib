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
    res.append(ProcessThreadService::getHostnamePidTid());

    /* 
     * Our unique sequence number splitter to make readability easier.
     */
    res.append(ClusterlibStrings::SEQUENCE_SPLIT);
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
NotifyableKeyManipulator::createPropertyListChildrenKey(
    const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROPERTYLISTS);

    return res;
}

string
NotifyableKeyManipulator::createQueueChildrenKey(
    const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::QUEUES);

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
NotifyableKeyManipulator::createApplicationChildrenKey(const string &rootKey)
{
    string res;
    res.append(rootKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::APPLICATIONS);

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
NotifyableKeyManipulator::createGroupChildrenKey(const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::GROUPS);

    return res;
}

string
NotifyableKeyManipulator::createDataDistributionChildrenKey(
    const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::DISTRIBUTIONS);

    return res;
}

string
NotifyableKeyManipulator::createNodeChildrenKey(const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::NODES);

    return res;
}

string
NotifyableKeyManipulator::createProcessSlotChildrenKey(const string &parentKey)
{
    string res;
    res.append(parentKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROCESSSLOTS);

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
NotifyableKeyManipulator::createNotifyableStateJSONObjectKey(
    const string &notifyableKey)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::NOTIFYABLESTATE_JSON_OBJECT);

    return res;
}

string
NotifyableKeyManipulator::createJSONObjectKey(const string &notifyableKey)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::DEFAULT_JSON_OBJECT);

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
NotifyableKeyManipulator::createShardsKey(const string &distKey)
{
    string res;
    res.append(distKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::SHARDS);

    return res;
}

string
NotifyableKeyManipulator::createPropertyListKey(const string &notifyableKey,
                                                const string &propListName)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::PROPERTYLISTS);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(propListName);

    return res;
}

string
NotifyableKeyManipulator::createKeyValsKey(const string &propListKey)
{
    string res;
    res.append(propListKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::KEYVAL);

    return res;
}

string
NotifyableKeyManipulator::createQueueKey(const string &notifyableKey,
                                         const string &queueName)
{
    string res;
    res.append(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::QUEUES);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(queueName);

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
NotifyableKeyManipulator::createQueuePrefixKey(
    const string &notifyableKey)
{
    string res = NotifyableKeyManipulator::createQueueParentKey(notifyableKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::QUEUEELEMENTPREFIX);

    return res;
}

string
NotifyableKeyManipulator::createQueueParentKey(
    const string &queueKey)
{
    string res(queueKey);
    res.append(ClusterlibStrings::KEYSEPARATOR);
    res.append(ClusterlibStrings::QUEUE_PARENT);
   
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

    split(components, key, is_any_of(ClusterlibStrings::KEYSEPARATOR));
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
        (name.find(ClusterlibStrings::KEYSEPARATOR) != string::npos) ||
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
         ClusterlibStrings::ClusterlibStrings::KEYSEPARATOR)) {
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
         *     DISTRIBUTIONS|PROPERTYLISTS|QUEUE/name.
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
            (!clusterlibObject.compare(ClusterlibStrings::PROPERTYLISTS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::DISTRIBUTIONS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::PROCESSSLOTS)) ||
            (!clusterlibObject.compare(ClusterlibStrings::QUEUES))) {
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
         * /(APPLICATIONS|GROUPS|NODES|DISTRIBUTIONS|
         *   PROPERTYLISTSS|QUEUES)/name
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
                ClusterlibStrings::PROPERTYLISTS) == 0)||
            (components.at(clusterlibObjectElements - 2).compare(
                ClusterlibStrings::QUEUES) == 0)) {
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
    size_t keySeparator = key.rfind(ClusterlibStrings::KEYSEPARATOR);
    if (keySeparator == string::npos) {
        return string();
    }
    return key.substr(0, keySeparator - 1); 
}

};	/* End of 'namespace clusterlib' */
