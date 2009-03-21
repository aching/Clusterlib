/*
 * propertiesimpl.cc --
 *
 * Implementation of the PropertiesImpl class.
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
 * Do not allow getProperties() on a Properties object
 */
Properties *
PropertiesImpl::getProperties(bool create)
{
    throw ClusterException("getProperties() called on a Properties object!");
}

/*
 * Initialize the cached representation of this object.
 */
void 
PropertiesImpl::initializeCachedRepresentation()
{
    updatePropertiesMap();
}

/*
 * Update the properties map from the repository.
 */
void
PropertiesImpl::updatePropertiesMap()
{
    int32_t version;
    string keyValMap = getDelegate()->loadKeyValMap(getKey(), version);

    /*
     * Only update if this is a newer version.
     */
    if (version > getKeyValVersion()) {
        m_keyValMap.clear();
        unmarshall(keyValMap);
        m_keyValMapVersion = version;
        setValueChangeTime(FactoryOps::getCurrentTimeMillis());
    }
}

void 
PropertiesImpl::setProperty(const string &name, 
                            const string &value)
{
    m_keyValMap[name] = value;
}

void
PropertiesImpl::deleteProperty(const string &name)
{
    if (m_keyValMap.erase(name) != 1) {
        LOG_WARN(CL_LOG, 
                 "deleteProperty: Failed delete with name %s", 
                 name.c_str());
    }
}

void
PropertiesImpl::publish()
{
    TRACE(CL_LOG, "publish");

    Locker k(getKeyValMapLock());
    string marshalledKeyValMap = marshall();
    int32_t finalVersion;
	
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
PropertiesImpl::getPropertyKeys() const
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
PropertiesImpl::getProperty(const string &name, bool searchParent)
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
        return "";
    }

    /*
     * Key manipulation should only be done in clusterlib.cc, therefore
     * this should move to clusterlib.cc.
     */
    Properties *prop = NULL;
    string parentKey = getKey();
    do {
        /*
         * Generate the new parentKey by removing PROPERTIES and one
         * clusterlib object.
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
PropertiesImpl::marshall() const
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
PropertiesImpl::unmarshall(const string &marshalledKeyValMap) 
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
