/*
 * notifyablekeymanipulator.h --
 *
 * Contains the key manipulation class.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_NOTIFYABLEKEYMANIPULATOR_H_
#define	_NOTIFYABLEKEYMANIPULATOR_H_

namespace clusterlib
{

/**
 * Definition of NotifyableKeyManipulator.
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
    static std::string createLockNodeKey(const std::string &notifyableKey,
                                         const std::string &lockName);
    static std::string createNodeKey(const std::string &groupKey,
                                     const std::string &nodeName);
    static std::string createProcessSlotKey(
        const std::string &nodeKey,
        const std::string &processSlotName);
    static std::string createGroupKey(const std::string &groupKey,
                                      const std::string &groupName);
    static std::string createApplicationKey(const std::string &appName);
    static std::string createRootKey();
    static std::string createDataDistributionKey(const std::string &groupKey,
                                                 const std::string &distName);
    static std::string createPropertyListKey(const std::string &notifyableKey,
                                             const std::string &propListName);
    static std::string createProcessSlotsUsageKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotsMaxKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotPortVecKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotExecArgsKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotRunningExecArgsKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotPIDKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotDesiredStateKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotCurrentStateKey(
        const std::string &notifyableKey);
    static std::string createProcessSlotReservationKey(
        const std::string &notifyableKey);

    /**
     * Create a sync event key that will be used to figure out which
     * PredMutexCond to signal.
     */
    static std::string createSyncEventKey(const int64_t &syncEventId);

    /**
     * Notifyables have one key that represent the object name in the
     * Zookeeper repository.  Each notifyable may have zookeeper nodes
     * attached to it (at most one level deep).  Any zookeeper node
     * beyond one level is not part of that object.  This function
     * will get the a key that refers to a Notifyable from any input
     * key.  For example, if the key is
     * .../group/client/nodes/foo-server/connected, it will return
     * .../group/client/nodes/foo-server. If the input key is not
     * related to any Notifyable, return an empty string.
     * 
     * @param key the key that should contain path that is part of a
     *            clusterlib object
     * @return the potential notifyable key, empty if no possible key.
     */
    static std::string getNotifyableKeyFromKey(const std::string &key);

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
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Notifyable exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isNotifyableKey(const std::vector<std::string> &components, 
                                int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Notifyable exists
     * for that key.
     * 
     * @param key A key to test if it is a notifyable
     * @return true if key is valid, false if not valid
     */
    static bool isNotifyableKey(const std::string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Application exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isApplicationKey(const std::vector<std::string> &components, 
                                 int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Application exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    static bool isApplicationKey(const std::string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Root exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isRootKey(const std::vector<std::string> &components, 
                                 int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Root exists
     * for that key.
     * 
     * @param key A key to test if it is an root
     * @return true if key is valid, false if not valid
     */
    static bool isRootKey(const std::string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual DataDistribution exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isDataDistributionKey(
        const std::vector<std::string> &components, 
        int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual DataDistribution exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    static bool isDataDistributionKey(const std::string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Group exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isGroupKey(const std::vector<std::string> &components, 
                           int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Group exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    static bool isGroupKey(const std::string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual PropertyList exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isPropertyListKey(const std::vector<std::string> &components, 
                                int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual PropertyList exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    static bool isPropertyListKey(const std::string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual Node exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isNodeKey(const std::vector<std::string> &components, 
                          int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Node exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    static bool isNodeKey(const std::string &key);

    /**
     * Checks if the components (assumed from a split) make up a valid
     * key, not if an actual ProcessSlot exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isProcessSlotKey(const std::vector<std::string> &components, 
                                 int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual ProcessSlot exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    static bool isProcessSlotKey(const std::string &key);

    /**
     * Remove the leaf node and returns the closest clusterlib
     * Notifyable object key.  For example, if the initial key is
     * .../_nodes/foo-server/_propertyList/defaultProps, it will return
     * .../_nodes/foo-server.  If the key is
     * .../group/client/nodes/foo-server, it will return
     * .../group/client.  If the key is .../applications/foo-app, it
     * will return an empty string since they is nothing left. The key
     * must not end in a KEYSEPARATOR.
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
     * must not end in a KEYSEPARATOR.
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

};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NOTIFYABLEKEYMANIPULATOR_H_ */
