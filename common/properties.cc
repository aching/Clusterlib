/*
 * properties.cc --
 *
 * Implementation of the Properties class.
 *
 * ============================================================================
 * $Header:$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlib.h"
#include <boost/regex.hpp>

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "ClusterLib"

namespace clusterlib
{

/*
 * Do not allow getProperties() on a Properties object
 */
Properties *
Properties::getProperties(bool create)
{
    throw ClusterException("getProperties() called on a Properties object!");
}

/*
 * Update the cached representation of this object.  Assumed to
 * already have the lock.
 */
void 
Properties::updateCachedRepresentation()
{
    int32_t version;
    string keyValMap = getDelegate()->loadKeyValMap(getKey(), version);

    /* Only update if this is a newer version */
    if (version > getKeyValVersion()) {
        m_keyValMap.clear();
        unmarshall(keyValMap);
        m_keyValMapVersion = version;
    }
}

void 
Properties::setProperty(const string &name, 
			const string &value)
{
    m_keyValMap[name] = value;
}

void
Properties::deleteProperty(const string &name)
{
    if (m_keyValMap.erase(name) != 1) {
        LOG_WARN(CL_LOG, 
                 "deleteProperty: Failed delete with name %s", 
                 name.c_str());
    }
}

void
Properties::publish()
{
    TRACE(CL_LOG, "publish");

    Locker k(getKeyValMapLock());
    string marshalledKeyValMap = marshall();
    int32_t finalVersion;

    LOG_INFO(CL_LOG,  
             "Tried to set node %s to %s with version %d\n",
             getKey().c_str(), 
             marshalledKeyValMap.c_str(), 
             getKeyValVersion());
	
    getDelegate()->updateProperties(getKey(),
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
Properties::getPropertyKeys() const
{
    vector<string> keys;

    Locker(getKeyValMapLock());
    for (KeyValMap::const_iterator kvIt = m_keyValMap.begin();
         kvIt != m_keyValMap.end(); 
         ++kvIt) {
        keys.push_back(kvIt->first);
    }
    
    return keys;
}
        
string 
Properties::getProperty(const string &name, bool searchParent)
{
    Locker(getKeyValMapLock());

    KeyValMap::const_iterator ssIt = m_keyValMap.find(name);
    if (ssIt != m_keyValMap.end()) {
	return ssIt->second;
    }
    else if (searchParent == false) {
        /*
         * Don't try the parent if not explicit
         */
        return string();
    }

    Properties *prop = NULL;
    string parentKey = getKey();
    do {
        /*
         * Generate the new parentKey by removing PROPERTIES and one
         * clusterlib object.
         */
        parentKey = getDelegate()->removeObjectFromKey(parentKey);
        parentKey = getDelegate()->removeObjectFromKey(parentKey);
     
        if (parentKey.empty()) {
            LOG_DEBUG(CL_LOG,
                      "getProperty: Giving up with new key %s from old key %s",
                      parentKey.c_str(),
                      getKey().c_str());
            return string();
        }
        parentKey.append(ClusterlibStrings::PATHSEPARATOR);
        parentKey.append(ClusterlibStrings::PROPERTIES);

        LOG_DEBUG(CL_LOG,
                  "getProperty: Trying new key %s from old key %s",
                  parentKey.c_str(),
                  getKey().c_str());
        
        prop = getDelegate()->getPropertiesFromKey(parentKey);
    } while (prop == NULL);
    
    return prop->getProperty(name, searchParent);
}

string 
Properties::marshall() const
{
    string res;
    for (KeyValMap::const_iterator kvIt = m_keyValMap.begin();
         kvIt != m_keyValMap.end(); 
         ++kvIt) {
        res.append(kvIt->first).append("=").append(kvIt->second);
	res.append(";");
    }
    return res;
}

bool 
Properties::unmarshall(const string &marshalledKeyValMap) 
{
    vector<string> nameValueList;
    split(nameValueList, marshalledKeyValMap, is_any_of(";"));
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
		throw ClusterException("Malformed property \"" +
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
