/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#define LOG_LEVEL LOG_WARN
#define MODULE_NAME "DataDistribution"

#include "clusterlib.h"

namespace clusterlib
{

static HashRange
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

static HashRange
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
    : Notifyable(f, key),
      mp_app(app),
      m_name(name),
      m_hashFnIndex(DD_HF_JENKINS),
      mp_hashFnPtr(fn)
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
    updateDistribution();
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
        throw ClusterException("Invalid data. Expecting 2 top level components");
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
    Application *app = NULL;

    split(components, marshalledShards, is_any_of(";"));
    for (i = components.begin(); i != components.end(); i++) {
        split(shardComponents, *i, is_any_of(","));
        if (shardComponents.size() != 5) {
            throw ClusterException("Malformed shard \"" +
                                   *i +
                                   "\", expecting 5 components");
        }
        nsp = getDelegate()->createShard(shardComponents[0],
                                         shardComponents[1],
                                         shardComponents[2],
                                         shardComponents[3],
                                         shardComponents[4]);

        if (nsp == NULL) {
            throw ClusterException("Could not create shard \"" +
                                   *i +
                                   "\"");
        }
                                   
        /*
         * Ensure that all shards point to the same application.
         */
        if (app == NULL) {
            app = nsp->getNode()->getGroup()->getApplication();
        } else {
            if (app != nsp->getNode()->getGroup()->getApplication()) {
                throw ClusterException("Distribution spanning multiple "
                                       "applications, unsupported!");
            }
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
                                    moComponents[3]);
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
    Group *gp;

    for (i = m_shards.begin(); i != m_shards.end(); i++) {
        np = (*i)->getNode();
        gp = np->getGroup();
        snprintf(buf,
                 1024,
                 "%lld,%lld,%s,%s,%s;",
                 (*i)->beginRange(),
                 (*i)->endRange(),
                 getApplication()->getName().c_str(),
                 gp->getName().c_str(),
                 np->getName().c_str());
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
 * Update the data distribution from ZK.
 */
void
DataDistribution::updateDistribution()
    throw(ClusterException)
{
    TRACE( CL_LOG, "updateDistribution" );
};

/*
 * Hash a key to a hash value using the current
 * hash function for this distribution.
 */
HashRange
DataDistribution::hashWork(const string &key)
{
    if (mp_hashFnPtr == NULL) {
        mp_hashFnPtr = s_hashFunctions[DD_HF_JENKINS];
    }
    return (*mp_hashFnPtr)(key);
};

/*
 * Return the node responsible for handling the work
 * represented by the given key.
 */
Node *
DataDistribution::findCoveringNode(const string &key)
{
    TRACE( CL_LOG, "findCoveringNode" );

#ifdef	USING_MANUAL_OVERRIDES
    /*
     * This section is compiled only when clusterlib
     * is configured to support manual overrides to
     * select nodes responsible for key-designated work.
     */
    NodeMap::iterator j;

    for (j = m_manualOverrides.begin();
         j != m_manualOverrides.end();
         j++) {
        if (find_regex(key, (*j)->first) != (*j)->first.end()) {
            return (*j)->second;
        }
    }
#endif	/* USING_MANUAL_OVERRIDES */

    HashRange hash = hashWork(key);
    ShardList::iterator i;

    for (i = m_shards.begin(); i != m_shards.end(); i++) {
        if ((*i)->contains(hash)) {
            return (*i)->getNode();
        }
    }

    return NULL;
};

/*
 * Constructor for class Shard for use by Factory.
 */
Shard::Shard(DataDistribution *dist,
             Node *node,
             HashRange beginRange,
             HashRange endRange)
    : mp_dist(dist),
      mp_node(node),
      m_beginRange(beginRange),
      m_endRange(endRange)
{
    TRACE( CL_LOG, "Shard" );
};

/*
 * Return the Node * if the hash value given falls within
 * the range of this shard.
 */
Node *
Shard::contains(HashRange hash)
{
    if ((hash >= beginRange()) && (hash <= endRange())) {
        return mp_node;
    }
    return NULL;
};

/*
 * Return true iff this shard covers the works represented
 * by this key.
 */
bool
Shard::covers(const string &key)
{
    HashRange hash = mp_dist->hashWork(key);

    if ((hash >= beginRange()) && (hash <= endRange())) {
        return true;
    }
    return false;
};

};       /* End of 'namespace clusterlib' */
