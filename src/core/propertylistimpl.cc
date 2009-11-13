/*
 * propertylistimpl.cc --
 *
 * Implementation of the PropertysListImpl class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"
#include <boost/regex.hpp>

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

using namespace std;
using namespace boost;

namespace clusterlib
{

/*
 * Do not allow getPropertyList() on a Properties object
 */
PropertyList *
PropertyListImpl::getPropertyList(const string &name,
                                bool create)
{
    throw InvalidMethodException(
        "getPropertyList() called on a PropertyList object!");
}

void 
PropertyListImpl::initializeCachedRepresentation()
{
    TRACE(CL_LOG, "initializeCachedRepresentation");

    updatePropertyListMap();
}

void
PropertyListImpl::removeRepositoryEntries()
{
    getOps()->removePropertyList(this);
}

/*
 * Update the property list map from the repository.
 */
void
PropertyListImpl::updatePropertyListMap()
{
    int32_t version;
    string keyValMap = getOps()->loadKeyValMap(getKey(), version);

    LOG_DEBUG(CL_LOG, 
              "updatePropertiesMap: Property list key = %s, "
              "new version = %d, old version = %d, new keyValMap = %s",
              getKey().c_str(),
              version,
              getKeyValVersion(),
              keyValMap.c_str());

    /*
     * Only update if this is a newer version.
     */
    if (version > getKeyValVersion()) {
        Locker l1(getSyncLock());
        m_keyValMap.clear();
        unmarshall(keyValMap);
        setKeyValVersion(version);
        setValueChangeTime(TimerService::getCurrentTimeMillis());
    }
    else {
        LOG_WARN(CL_LOG,
                 "updatePropertyListMap: Have a newer version (%d) "
                 "than the repository (%d)",
                 getKeyValVersion(),
                 version);
    }
}

void 
PropertyListImpl::setProperty(const string &name, 
                              const string &value)
{
    TRACE(CL_LOG, "setProperty");

    throwIfRemoved();

    if (name.find(ClusterlibStrings::PROPERTYLISTDELIMITER) != string::npos) {
        throw InvalidArgumentsException(
            "setProperty: Cannot use delimiter " + 
            ClusterlibStrings::PROPERTYLISTDELIMITER + " in the name");
    }
    if (value.find(ClusterlibStrings::PROPERTYLISTDELIMITER) != string::npos) {
        throw InvalidArgumentsException(
            "setProperty: Cannot use delimiter " + 
            ClusterlibStrings::PROPERTYLISTDELIMITER + " in the value");
    }
    m_keyValMap[name] = value;
}

void
PropertyListImpl::deleteProperty(const string &name)
{
    TRACE(CL_LOG, "deleteProperty");

    throwIfRemoved();

    Locker(getSyncLock());
    if (m_keyValMap.erase(name) != 1) {
        LOG_WARN(CL_LOG, 
                 "deleteProperty: Failed delete with name %s", 
                 name.c_str());
    }
}

void
PropertyListImpl::publish()
{
    TRACE(CL_LOG, "publish");

    Locker k(getSyncLock());
    string marshalledKeyValMap = marshall();
    int32_t finalVersion;
	
    getOps()->updatePropertyList(getKey(),
                                 marshalledKeyValMap,
                                 getKeyValVersion(),
                                 finalVersion);
    
    /* 
     * Since we should have the lock, the data should be identical to
     * the zk data.  When the lock is released, clusterlib events will
     * try to push this change again.  
     */
    setKeyValVersion(finalVersion);
}

vector<string>
PropertyListImpl::getPropertyListKeys() const
{
    TRACE(CL_LOG, "getPropertyListKeys");

    throwIfRemoved();

    vector<string> keys;

    Locker(getSyncLock());
    for (KeyValMap::const_iterator kvIt = m_keyValMap.begin();
         kvIt != m_keyValMap.end(); 
         ++kvIt) {
        keys.push_back(kvIt->first);
    }
    
    return keys;
}
        
string 
PropertyListImpl::getProperty(const string &name, bool searchParent)
{
    TRACE(CL_LOG, "getProperty");
    
    throwIfRemoved();

    Locker(getSyncLock());

    KeyValMap::const_iterator ssIt = m_keyValMap.find(name);

    if (ssIt != m_keyValMap.end()) {
        LOG_DEBUG(CL_LOG,
                  "getProperty: Found name = %s with val %s "
                  "in Properties key %s",
                  name.c_str(),
                  ssIt->second.c_str(),
                  getKey().c_str());
	return ssIt->second;
    }
    else if (searchParent == false) {
        /*
         * Don't try the parent if not explicit
         */
        return "";
    }

    /*
     * Key manipulation should only be done in clusterlib.cc, therefore
     * this should move to clusterlib.cc.
     */
    PropertyList *prop = NULL;
    string parentKey = getKey();
    do {
        /*
         * Generate the new parentKey by removing this PropertyList
         * object and one clusterlib object.
         */
        parentKey = NotifyableKeyManipulator::removeObjectFromKey(parentKey);
        parentKey = NotifyableKeyManipulator::removeObjectFromKey(parentKey);

        if (parentKey.empty()) {
            LOG_DEBUG(CL_LOG,
                      "getProperty: Giving up with new key %s from old key %s",
                      parentKey.c_str(),
                      getKey().c_str());
            return string();
        }
        parentKey.append(ClusterlibStrings::KEYSEPARATOR);
        parentKey.append(ClusterlibStrings::PROPERTYLIST);
        parentKey.append(ClusterlibStrings::KEYSEPARATOR);
        parentKey.append(getName());

        LOG_DEBUG(CL_LOG,
                  "getProperty: Trying new key %s from old key %s",
                  parentKey.c_str(),
                  getKey().c_str());
        
        prop = getOps()->getPropertyListFromKey(parentKey);
    } while (prop == NULL);
    
    return prop->getProperty(name, searchParent);
}

string 
PropertyListImpl::marshall() const
{
    string res;
    for (KeyValMap::const_iterator kvIt = m_keyValMap.begin();
         kvIt != m_keyValMap.end(); 
         ++kvIt) {
        res.append(kvIt->first).append("=").append(kvIt->second);
	res.append(ClusterlibStrings::PROPERTYLISTDELIMITER);
    }
    return res;
}

bool 
PropertyListImpl::unmarshall(const string &marshalledKeyValMap) 
{
    vector<string> nameValueList;
    split(nameValueList, 
          marshalledKeyValMap, 
          is_any_of(ClusterlibStrings::PROPERTYLISTDELIMITER));
    if (nameValueList.empty()) {
        return false;
    }
    KeyValMap keyValMap;
    for (vector<string>::iterator sIt = nameValueList.begin();
         sIt != nameValueList.end() - 1; 
         ++sIt) {
        if (*sIt != "") {
            vector<string> pair;
            split(pair, *sIt, is_any_of("="));
            if (pair.size() != 2) {
		stringstream s;
		s << pair.size();
		LOG_FATAL(CL_LOG,
                          "unmarshall: key-val pair (%d component(s)) = %s", 
                          pair.size(),
                          (*sIt).c_str());
		throw InconsistentInternalStateException(
                    "Malformed property list \"" +
                    *sIt +
                    "\", expecting 2 components " +
                    "and instead got " + s.str().c_str());
            }
            keyValMap[pair[0]] = pair[1];
        }
    }
    m_keyValMap = keyValMap;

    return true;
}

};	/* End of 'namespace clusterlib' */
