/*
 * forwarddecls.h --
 *
 * Forward declarations for classes that need to be
 * forward-declared.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_FORWARDDECLS_H_
#define	_FORWARDDECLS_H_

using namespace std;

namespace clusterlib
{

/*
 * Define the types associated with a hash function.
 */
typedef unsigned long long HashRange;
typedef int32_t            HashFunctionId;
typedef HashRange (HashFunction)(const string &key);

/*
 * Random bits owned by the user program. Clusterlib just
 * provides a way to attach these bits to events and cache
 * objects.
 */
typedef void *ClientData;

/*
 * Some types that must be declared early, to be able to define
 * maps and such.
 */
typedef int32_t TimerId;

/*
 * Alphabetical order of forward declared classes.
 */
class Application;
class Client;
class ClusterEventHandler;
class ClusterEventPayload;
class ClusterException;
class DataDistribution;
class Properties;
class Factory;
class FactoryOps;
class Group;
class HealthChecker;
class ManualOverride;
class Node;
class NodeAddress;
class Notifyable;
class Server;
class Shard;
class TimerEventHandler;
class TimerEventPayload;

/*
 * Vectors of pointers to these classes.
 */
typedef vector<string>				IdList;
typedef vector<Application *>			ApplicationList;
typedef vector<Client *>			ClientList;
typedef vector<DataDistribution *>		DataDistributionList;
typedef vector<Factory *>			FactoryList;
typedef vector<FactoryOps *>			FactoryOpsList;
typedef vector<Group *>				GroupList;
typedef vector<HealthChecker *>			HealthCheckerList;
typedef vector<Node *>				NodeList;
typedef	vector<NodeAddress *>			NodeAddressList;
typedef vector<Server *>			ServerList;
typedef vector<Shard *>				ShardList;
    
/*
 * Maps of pointers to these classes.
 */
typedef map<string, Application *>		ApplicationMap;
typedef map<string, Client *>			ClientMap;
typedef map<string, DataDistribution *>		DataDistributionMap;
typedef map<string, Factory *>			FactoryMap;
typedef map<string, FactoryOps *>		FactoryOpsMap;
typedef map<string, Group *>			GroupMap;
typedef map<string, HealthChecker *>		HealthCheckerMap;
typedef map<string, ManualOverride *>		ManualOverridesMap;
typedef map<string, Node *>	        	NodeMap;
typedef map<string, Server *>			ServerMap;
typedef map<string, string>                     KeyValMap;
typedef map<string, Properties *>	      	PropertiesMap;
typedef map<TimerId, TimerEventPayload *>	TimerRegistry;

/*
 * Type used for passing flags.
 */
typedef unsigned int ServerFlags;

/*
 * Values for ServerFlags. Can be combined by
 * OR-ing together.
 */
#define SF_NONE		0		/* No flags. */
#define SF_MANAGED	(1<<0)		/* A managed node? */
#define SF_CREATEREG	(1<<1)		/* Create in cluster if
                                         * it doesn't exist? */

};	/* End of 'namespace clusterlib' */

#endif	/* !_FORWARDDECLS_H_ */
