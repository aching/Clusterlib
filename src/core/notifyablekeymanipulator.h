/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_NOTIFYABLEKEYMANIPULATOR_H_
#define	_CL_NOTIFYABLEKEYMANIPULATOR_H_

namespace clusterlib {

/**
 * Creates/modifies the appropriate key for all key manipulation.
 */
class NotifyableKeyManipulator
{
  public:    
    /* 
     * Generate valid keys for various clusterlib objects given that
     * the inputs are valid.  Does not create the objects or check
     * that they exist. 
     */
    static std::string createLocksKey(const std::string &notifyableKey);
    static std::string createLockKey(const std::string &notifyableKey,
                                     const std::string &lockName);
    static std::string createLockNodeKey(
        const std::string &notifyableKey,
        const std::string &lockName,
        DistributedLockType distributedLockType);

    /**
     * Generate the ProcessSlot key
     *
     * @param nodeKey the name of the Node key
     * @param processSlotName the name of the ProcessSlot
     * @return Generated ProcessSlot key
     */
    static std::string createProcessSlotKey(
        const std::string &nodeKey,
        const std::string &processSlotName);

    /**
     * Create the key where all property list children are stored
     * 
     * @param parentKey the key of the parent
     * @return the generated children key
     */
    static std::string createPropertyListChildrenKey(
        const std::string &parentKey);

    /**
     * Create the key where all queue children are stored
     * 
     * @param parentKey the key of the parent
     * @return the generated children key
     */
    static std::string createQueueChildrenKey(
        const std::string &parentKey);

    /**
     * Generate the root key
     *
     * @return the generated root key
     */
    static std::string createRootKey();

    /**
     * Create the key where all application children are stored
     * 
     * @param rootKey the key of the root
     * @return the generated children key
     */
    static std::string createApplicationChildrenKey(
        const std::string &rootKey);

    /**
     * Generate the application key
     *
     * @param appName the name of the application
     * @return the generated application key
     */
    static std::string createApplicationKey(const std::string &appName);

    /**
     * Create the key where all group children are stored
     * 
     * @param parentKey the key of the parent
     * @return the generated children key
     */
    static std::string createGroupChildrenKey(
        const std::string &parentKey);

    /**
     * Create the key where all data distribution children are stored
     * 
     * @param parentKey the key of the parent
     * @return the generated children key
     */
    static std::string createDataDistributionChildrenKey(
        const std::string &parentKey);

    /**
     * Create the key where all node children are stored
     * 
     * @param parentKey the key of the parent
     * @return the generated children key
     */
    static std::string createNodeChildrenKey(
        const std::string &parentKey);

    /**
     * Generate the group key
     *
     * @param groupKey the name of the group key
     * @param groupName the name of the group
     * @return the generated group key
     */
    static std::string createGroupKey(const std::string &groupKey,
                                      const std::string &groupName);

    /**
     * Create the key where all ProcessSlot children are stored
     * 
     * @param parentKey the key of the parent
     * @return the generated children key
     */
    static std::string createProcessSlotChildrenKey(
        const std::string &parentKey);

    /**
     * Generate the node key
     *
     * @param groupKey the name of the group key
     * @param nodeName the name of the node
     * @return the generated node key
     */
    static std::string createNodeKey(const std::string &groupKey,
                                     const std::string &nodeName);

    /**
     * Generate the a generic JSON object key for a notifyable
     * 
     * @param notifyableKey the name of the notifyableKey
     * @return the generated JSON object key
     */
    static std::string createJSONObjectKey(
        const std::string &notifyableKey);

    /**
     * Generate the data distribution key
     *
     * @param groupKey the name of the group key
     * @param distName the name of the data distribution
     * @return the generated data distribution key
     */
    static std::string createDataDistributionKey(const std::string &groupKey,
                                                 const std::string &distName);

    /**
     * Generate the property list key
     *
     * @param notifyableKey the name of the notifyable key
     * @param propListName the name of the property list
     * @return the generated property list key
     */
    static std::string createPropertyListKey(const std::string &notifyableKey,
                                             const std::string &propListName);

    /**
     * Generate the queue key
     *
     * @param notifyableKey the name of the notifyable key
     * @param queueName the name of the queue
     * @return the generated queue key
     */
    static std::string createQueueKey(const std::string &notifyableKey,
                                      const std::string &queueName);

    /**
     * Generate the a queue parent for a queue
     * 
     * @param queueKey the name of the queue
     * @return the generated queue parent key
     */
    static std::string createQueueParentKey(
        const std::string &queueKey);

    static std::string createQueuePrefixKey(
        const std::string &notifyableKey);

    /**
     * Create a sync event key that will be used to figure out which
     * PredMutexCond to signal.
     */
    static std::string createSyncEventKey(const int64_t &syncEventId);

    /**
     * If there is a lot of checking Notifyable key strings, it will
     * be more efficient to use this function to first split the key
     * into components and then use the component Notifyable functions
     * to do the checks.
     *
     * @param key the key to split
     * @param components a reference to the output components vector
     */
    static void splitNotifyableKey(const std::string &key, 
                                   std::vector<std::string> &components);

    /**
     * Clusterlib object names cannot have any '/' or be any reserved
     * clusterlib keywords.
     *
     * @return true if name is allowed, false otherwise
     */
    static bool isValidNotifyableName(const std::string &name);

    /**
     * Remove the leaf node and returns the closest clusterlib
     * Notifyable object key.  For example, if the initial key is
     * .../_nodes/foo-server/_propertyList/defaultProps, it will return
     * .../_nodes/foo-server.  If the key is
     * .../group/client/nodes/foo-server, it will return
     * .../group/client.  If the key is .../applications/foo-app, it
     * will return an empty string since they is nothing left. The key
     * must not end in a CLString::KEY_SEPARATOR.
     *
     * @param key a path to be trimmed * @return trimmed key or empty 
     *            string if no parent clusterlib object key
     * @return the string without the last object
     */
    static std::string removeObjectFromKey(const std::string &key);

    /**
     * Remove the leaf node and returns the closest clusterlib
     * Notifyable object key.  For example, if the initial components form
     * .../_nodes/foo-server/_propertyList/defaultProps, it will 
     * return elements that include
     * .../_nodes/foo-server.  If the components form 
     * .../_group/client/_nodes/foo-server, it will 
     * return elements that include
     * .../_group/client.  If the components are .../applications/foo-app, it
     * will return an empty string since they is nothing left. The key
     * must not end in a CLString::KEY_SEPARATOR.
     *
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return size of the trimmed key or -1 if no Notifyable parent possible
     */
    static int32_t removeObjectFromComponents(
        const std::vector<std::string> &components,
        int32_t elements);

    /**
     * Remove one of the components from the path.  For example, if
     * /A/B/C, this will return /A/B.  If /A/B/C/, this will return
     * /A/B/C.  If no key separators are found, then empty string.
     *
     * @param key the original key that will have a component removed
     * @return the stripped key (or empty if no key separator)
     */
    static std::string removeComponentFromKey(const std::string &key);

};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NOTIFYABLEKEYMANIPULATOR_H_ */
