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

/*
 * The actual key manipulation class.
 */
class NotifyableKeyManipulator
{
  public:    
    /* 
     * Generate valid keys for various clusterlib objects given that
     * the inputs are valid.  Does not create the objects or check
     * that they exist. 
     */
    static std::string createNodeKey(const std::string &groupKey,
                                     const std::string &nodeName,
                                     bool managed);
    static std::string createGroupKey(const std::string &groupKey,
                                      const std::string &groupName);
    static std::string createAppKey(const std::string &appName);
    static std::string createDistKey(const std::string &groupKey,
                                     const std::string &distName);
    static std::string createPropertiesKey(const std::string &notifyableKey);

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
     * key, not if an actual Properties exists for that key.
     * 
     * @param components A vector of components in the key parsed by split
     *                   (i.e. first component should be "")
     * @param elements The number of elements to check with (should 
     *                 be <= components.size()).  If it is -1, then use 
     *                 components.size().
     * @return true if key is valid, false if not valid
     */
    static bool isPropertiesKey(const std::vector<std::string> &components, 
                                int32_t elements = -1);

    /**
     * Checks if the key is valid, not if an actual Properties exists
     * for that key.
     * 
     * @param key A key to test if it is an application
     * @return true if key is valid, false if not valid
     */
    static bool isPropertiesKey(const std::string &key);

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
     * Remove the leaf node and returns the closest clusterlib
     * Notifyable object key.  For example, if the initial key is
     * .../nodes/foo-server/properties, it will return
     * .../nodes/foo-server.  If the key is
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
     * .../nodes/foo-server/properties, it will return elements that include
     * .../nodes/foo-server.  If the components form 
     * .../group/client/nodes/foo-server, it will return elements that include
     * .../group/client.  If the components are .../applications/foo-app, it
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
