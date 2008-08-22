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
 * Alphabetical order.
 */
class Application;
class Client;
class ClusterEventHandler;
class ClusterException;
class DataDistribution;
class Factory;
class FactoryOps;
class Group;
class HealthChecker;
class Node;
class NodeAddress;
class Notifyable;
class Server;
class TimerEventHandler;

/*
 * Objects attached to (and registered for)
 * events in the event system.
 */
class EventPayload;
class TimerPayload;
class ZooKeeperPayload;

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
typedef map<string, Node *>	        	NodeMap;
typedef map<string, Server *>			ServerMap;

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
