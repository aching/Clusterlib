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
	m_keyValMap.clear();
	unmarshall(keyValMap);
	m_keyValMapVersion = version;
    }
}
    
void 
Properties::setProperties(const vector<string> &keys, 
			  const vector<string> &values)
{
    if (keys.size() != values.size()) {
	throw ClusterException("keys and values are not the same size");
    }
    
    {
	Locker l1(getKeyValMapLock());
	for (uint32_t i = 0; i < keys.size(); i++) {
	    m_keyValMap[keys[i]] = values[i];
	}
	
	string marshalledKeyValMap = marshall();
	
	fprintf(stderr, 
		"Tried to set node %s to %s with version %d\n",
		getKey().c_str(), marshalledKeyValMap.c_str(), 
		getKeyValVersion());
	
	getDelegate()->updateProperties(getKey(),
					marshalledKeyValMap,
					getKeyValVersion());    
    }
}

string 
Properties::getProperty(const string &name)
{
    updateCachedRepresentation();

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
