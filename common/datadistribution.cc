/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "DataDistribution"

#include "clusterlib.h"
#include <boost/regex.hpp>

namespace clusterlib
{

/*
 * Constants for identifying the various parts of a shard
 * specification.
 */
const int DataDistribution::SC_LOWBOUND_IDX	= 0;
const int DataDistribution::SC_HIBOUND_IDX	= 1;
const int DataDistribution::SC_APPNAME_IDX	= 2;
const int DataDistribution::SC_GROUPNAME_IDX	= 3;
const int DataDistribution::SC_NODENAME_IDX	= 4;

static DataDistribution::HashRange
jenkinsHashImpl(const string &key)
{
    // adapted from jenkin's one-at-a-time hash
    uint32_t hash = 0;
    size_t i;
    
    for (i = 0; i < key.length(); i++) {
        hash += key.at(i);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
};

static DataDistribution::HashRange
md5HashImpl(const string &key)
{
    return 0;
}

/*
 * The array of hash functions.
 */
DataDistribution::HashFunction *
DataDistribution::s_hashFunctions[] =
{
    NULL,		/* DD_HF_USERDEF */
    &md5HashImpl,	/* DD_HF_MD5 */
    &jenkinsHashImpl,	/* DD_HF_JENKINS */
    NULL		/* DD_HF_END */
};

/*
 * Constructor.
 */
DataDistribution::DataDistribution(Application *app,
                                   const string &name,
                                   const string &key,
                                   FactoryOps *f,
                                   HashFunction *fn)
    : Notifyable(f, key, name),
      mp_app(app),
      m_hashFnIndex(DD_HF_JENKINS),
      mp_hashFnPtr(fn),
      m_modified(false)
{
    TRACE( CL_LOG, "DataDistribution" );

    m_shards.clear();
    m_manualOverrides.clear();
    
    /*
     * Set up the hash function to use.
     */
    if (m_hashFnIndex == DD_HF_USERDEF) {
        if (mp_hashFnPtr == NULL) {
            mp_hashFnPtr = s_hashFunctions[DD_HF_JENKINS];
        }
    } else {
        mp_hashFnPtr = s_hashFunctions[m_hashFnIndex];
    }

    /*
     * Read the data distribution from ZK.
     */
    updateCachedRepresentation();
};

/*
 * Unmarshall a string-form of the data distribution.
 */
void
DataDistribution::unmarshall(const string &marshalledData)
    throw(ClusterException)
{
    TRACE( CL_LOG, "unmarshall" );

    vector<string> components;

    /*
     * The marshalled form is "shards\nmanualOverrides"
     */
    split( components, marshalledData, is_any_of( "\n" ) ); 
    if (components.size() != 2) {
        throw ClusterException("Invalid data. Expecting 2 top "
                               "level components");
    }
    unmarshallShards(components[0], m_shards);
    unmarshallOverrides(components[1], m_manualOverrides);
};

/*
 * Unmarshall a stringified sequence of shards. Each shard
 * is stringified to "begin,end,appname,groupname,nodename;"
 */
void
DataDistribution::unmarshallShards(const string &marshalledShards,
                                   ShardList &l)
    throw(ClusterException)
{
    TRACE( CL_LOG, "unmarshallShards" );

    vector<string> components;
    vector<string> shardComponents;
    vector<string>::iterator i;
    Shard *nsp;

    split(components, marshalledShards, is_any_of(";"));
    for (i = components.begin(); i != components.end(); i++) {
        split(shardComponents, *i, is_any_of(","));
        if (shardComponents.size() != 5) {
            throw ClusterException("Malformed shard \"" +
                                   *i +
                                   "\", expecting 5 components");
        }

        /*
         * Resolve the node lazily -- only load it if we're actually
         * asked to access it via this shard.
         */
        nsp = new Shard(this,
                        getDelegate()
			   ->createNodeKey(shardComponents[SC_APPNAME_IDX],
                                           shardComponents[SC_GROUPNAME_IDX],
                                           shardComponents[SC_NODENAME_IDX],
                                           true),
                        atoll(shardComponents[SC_LOWBOUND_IDX].c_str()),
                        atoll(shardComponents[SC_HIBOUND_IDX].c_str()));
        if (nsp == NULL) {
            throw ClusterException("Could not create shard \"" +
                                   *i +
                                   "\"");
        }

        /*
         * Add the shard to our cache.
         */
        l.push_back(nsp);
    }        
};

/*
 * Unmarshall a stringified sequence of manual overrides.
 * Each override is stringified to "pattern,appname,groupname,nodename;".
 */
void
DataDistribution::unmarshallOverrides(const string &marshalledOverrides,
                                      NodeMap &m)
    throw(ClusterException)
{
    TRACE( CL_LOG, "unmarshallOverrides" );

    vector<string> components;
    vector<string> moComponents;
    vector<string>::iterator i;
    Node *np;
    Application *app = NULL;

    split(components, marshalledOverrides, is_any_of(";"));
    for (i = components.begin(); i != components.end(); i++) {
        split(moComponents, *i, is_any_of(","));
        if (moComponents.size() != 4) {
            throw ClusterException("Malformed manual override \"" +
                                   *i +
                                   "\", expecting 4 components");
        }
        np = getDelegate()->getNode(moComponents[1],
                                    moComponents[2],
                                    moComponents[3],
                                    true);
        if (np == NULL) {
            throw ClusterException("Could not find node for manual "
                                   "override \"" +
                                   *i +
                                   "\"");
        }

        /*
         * Ensure that all manual overrides point to the same
         * application to handle the work.
         */
        if (app == NULL) {
            app = np->getGroup()->getApplication();
        } else {
            if (app != np->getGroup()->getApplication()) {
                throw ClusterException("Distribution manual overrides "
                                       "spanning multiple applications, "
                                       "unsupported!");
            }
        }

        /*
         * Add the manual override to our cache.
         */
        m[moComponents[0]] = np;
    }                                   
};

/*
 * Marshall a data distribution to string form.
 */
string
DataDistribution::marshall()
{
    TRACE( CL_LOG, "marshall" );

    return marshallShards() + "\n" + marshallOverrides();
};

/*
 * Marshall a data distribution's set of shards to string form.
 * Each shard gets marshalled to "begin,end,appname,groupname,nodename;".
 */
string
DataDistribution::marshallShards()
{
    TRACE( CL_LOG, "marshallShards" );

    string res = "";
    ShardList::iterator i;
    char buf[1024];
    Node *np;
    const char *nodeName;
    const char *groupName;
    const char *appName = getApplication()->getName().c_str();

    for (i = m_shards.begin(); i != m_shards.end(); i++) {
        np = (*i)->getNode();
        if (np != NULL) {
            nodeName = np->getName().c_str();
            groupName = np->getGroup()->getName().c_str();
        } else {
            nodeName =
                getDelegate()->nodeNameFromKey((*i)->getNodeKey()).c_str();
            groupName = 
                getDelegate()->groupNameFromKey((*i)->getNodeKey()).c_str();
        }
        snprintf(buf,
                 1024,
                 "%lld,%lld,%s,%s,%s;",
                 (*i)->beginRange(),
                 (*i)->endRange(),
                 appName,
                 groupName,
                 nodeName);            
        res += buf;
    }
    return res;
};

/*
 * Marshall a data distribution's manual overrides to string form.
 * Each manual override is marshalled to
 * "pattern,appname,groupname,nodename;".
 */
string
DataDistribution::marshallOverrides()
{
    TRACE( CL_LOG, "marshallOverrides" );

    string res = "";
    NodeMap::iterator i;
    char buf[1024];
    Node *np;
    Group *gp;

    for (i = m_manualOverrides.begin();
         i != m_manualOverrides.end(); 
         i++) {
        np = (*i).second;
        gp = np->getGroup();
        snprintf(buf,
                 1024,
                 "%s,%s,%s,%s;",
                 (*i).first.c_str(),
                 getApplication()->getName().c_str(),
                 gp->getName().c_str(),
                 np->getName().c_str());
        res += buf;
    }
    return res;
};

/*
 * Update the data distribution from the cluster.
 */
void
DataDistribution::updateCachedRepresentation()
    throw(ClusterException)
{
    TRACE( CL_LOG, "updateCachedRepresentation" );

    string shards = getDelegate()->loadShards(getKey());
    string overrides = getDelegate()->loadManualOverrides(getKey());
    ShardList::iterator si;
    NodeMap::iterator mi;

    for (si = m_shards.begin(); si != m_shards.end(); si++) {
        delete *si;
    }
    m_shards.clear();
    m_manualOverrides.clear();

    unmarshallShards(shards, m_shards);
    unmarshallOverrides(overrides, m_manualOverrides);    
};

/*
 * Hash a key to a node using the current
 * hash function for this distribution.
 */
Node *
DataDistribution::map(const string &key)
    throw(ClusterException)
{
    TRACE( CL_LOG, "map" );

#ifdef	ENABLING_MANUAL_OVERRIDES
    /*
     * Search the manual overrides for a match.
     */
    NodeMap::iterator j;
    cmatch what;

    for (j = m_manualOverrides.begin();
         j != m_manualOverrides.end();
         j++) {
        regex expression((*j).first);

        if (regex_match(key.c_str(), what, expression)) {
            return (*j).second;
        }
    }
#endif

    /*
     * Use the shard mapping.
     */
    HashRange hash = hashWork(key);
    return map(hash);
};

/*
 * Given a hash value, retrieve the node to which it is mapped.
 */
Node *
DataDistribution::map(HashRange hash)
    throw(ClusterException)
{
    TRACE( CL_LOG, "map" );

    return m_shards[getShardIndex(hash)].getNode();
}

/*
 * Compute the hash range for this key.
 */
DataDistribution::HashRange
DataDistribution::hashWork(const string &key)
{
    if (mp_hashFnPtr == NULL) {
        mp_hashFnPtr = s_hashFunctions[DD_HF_JENKINS];
    }
    return (*mp_hashFnPtr)(key);
};

/*
 * Returns the manual override item that matches this
 * key if one exists (the first one found, in an
 * unspecified order) or the empty string if none.
 */
string
DataDistribution::matchesManualOverride(const string &key)
    throw(ClusterException)
{
    TRACE( CL_LOG, "matchesManualOverrides" );

#ifdef	ENABLING_MANUAL_OVERRIDES
    /*
     * Search the manual overrides for a match.
     */
    NodeMap::iterator j;
    cmatch what;

    for (j = m_manualOverrides.begin();
         j != m_manualOverrides.end();
         j++) {
        regex expression((*j).first);

        if (regex_match(key.c_str(), what, expression)) {
            return (*j).first;
        }
    }
#endif

    return string("");
}

/*
 * Is the distribution covered?
 */
bool
DataDistribution::isCovered()
    throw(ClusterException)
{
    TRACE( CL_LOG, "isCovered" );

    return true;
}

/*
 * Assign new shards.
 */
void
DataDistribution::setShards(vector<unsigned long long> &upperBounds)
    throw(ClusterException)
{
    TRACE( CL_LOG, "setShards" );
}

/*
 * Get the shard index for a hash value or key.
 */
uint32_t
DataDistribution::getShardIndex(const string &key)
    throw(ClusterException)
{
    TRACE( CL_LOG, "getShardIndex" );

    return getShardIndex(hashWork(key));
}
uint32_t
DataDistribution::getShardIndex(HashRange hash)
    throw(ClusterException)
{
    TRACE( CL_LOG, "getShardIndex" );

    /*
     * Use the shard mapping. This is a linear search; better efficiency
     * can for sure be obtained by doing a binary search or some other
     * faster algorithm that relies on the shards being sorted in
     * increasing hash range order.
     */
    ShardList::iterator i;
    unsigned int j;

    for (i = m_shards.begin(), j = 0; i != m_shards.end(); j++, i++) {
        if ((*i)->contains(hash)) {
            return j;
        }
    }

    throw ClusterException(string("") +
                           "Hash for \"" +
                           key +
                           "\" is unasigned!");                           
};

/*
 * Retrieve the shard details for a given shard index.
 */

/*
 * Retrieve -- or load -- the node of this shard.
 */
Node *
DataDistribution::Shard::getNode()
{
    /*
     * If the node is already loaded, return it.
     */
    if (mp_node != NULL) {
        return mp_node;
    }
    /*
     * Otherwise load and return the node from the key.
     */
    mp_node = mp_dist->getDelegate()->getNodeFromKey(m_nodeKey, false);
    return mp_node;
}

/*
 * Return the Node * if the hash value given falls within
 * the range of this shard.
 */
Node *
DataDistribution::Shard::contains(HashRange hash)
{
    if ((hash >= m_beginRange) && (hash <= m_endRange)) {
        return getNode();
    }
    return NULL;
};

/*
 * Return true iff this shard covers the works represented
 * by this key.
 */
bool
DataDistribution::Shard::covers(const string &key)
{
    HashRange hash = mp_dist->hashWork(key);

    if ((hash >= m_beginRange) && (hash <= m_endRange)) {
        return true;
    }
    return false;
};

};       /* End of 'namespace clusterlib' */
