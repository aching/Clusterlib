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
 * Update the cached representation of this object.
 */
void 
Properties::updateCachedRepresentation()
{
    int32_t version;
    string keyValMap = getDelegate()->loadKeyValMap(getKey(), version);

    {
	Locker l(getKeyValMapLock());
	/* Only update if this is a newer version */
	if (version > getKeyValVersion()) {
	    m_keyValMap.clear();
	    unmarshall(keyValMap);
	    m_keyValMapVersion = version;
	}
    }
}

void 
Properties::setProperty(const string &name, 
			const string &value)
{
    m_keyValMap[name] = value;
}

void
Properties::publish()
{
    TRACE( CL_LOG, "publish" );

    Locker k(getKeyValMapLock());
    string marshalledKeyValMap = marshall();
    
    LOG_INFO( CL_LOG,  
	      "Tried to set node %s to %s with version %d\n",
	      getKey().c_str(), marshalledKeyValMap.c_str(), 
	      getKeyValVersion());
	
    getDelegate()->updateProperties(getKey(),
				    marshalledKeyValMap,
				    getKeyValVersion());
    
    /* Since we should have the lock, the data should be identical to
     * the zk data.  When the lock is released, clusterlib events will
     * try to push this change again.  */
    setKeyValVersion(getKeyValVersion() + 1);
}

string 
Properties::getProperty(const string &name)
{
    map<string, string>::const_iterator i = m_keyValMap.find(name);    
    if (i != m_keyValMap.end()) {
	return i->second;
    }
    
    string parent = getDelegate()->removeObjectFromKey(getKey());
    if (parent.empty() || (!parent.compare(getKey()))) {
	return string();
    }

    Properties *prop = getDelegate()->getProperties(parent);
    if (prop == NULL) {
	return string();
    }

    return prop->getProperty(name);
}
        
string 
Properties::marshall() const
{
    string res;
    for (KeyValMap::const_iterator i = m_keyValMap.begin();
         i != m_keyValMap.end();
         ++i)
    {
	if (res.length() > 0) {
            res.append( ";" );
        }
        res.append( i->first ).append( "=" ).append( i->second );
    }
    return res;
}

bool 
Properties::unmarshall(const string &marshalledKeyValMap) 
{
    vector<string> nameValueList;
    split( nameValueList, marshalledKeyValMap, is_any_of( ";" ) );
    if (nameValueList.empty()) {
        return false;
    }
    KeyValMap keyValMap;
    for (vector<string>::iterator i = nameValueList.begin();
         i != nameValueList.end();
         ++i)
    {
        if (*i != "") {
            vector<string> pair;
            split( pair, *i, is_any_of( "=" ) );
            if (pair.size() != 2) {
                return false;
            }
            keyValMap[pair[0]] = pair[1];
        }
    }
    m_keyValMap = keyValMap;
    return true;
}

};	/* End of 'namespace clusterlib' */
