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

namespace clusterlib
{

/*
 * Define the types associated with a hash function.
 */
typedef uint64_t HashRange;
typedef int32_t  HashFunctionId;
typedef HashRange (HashFunction)(const std::string &key);

/*
 * Random bits owned by the user program. Clusterlib just
 * provides a way to attach these bits to events and cache
 * objects.
 */
typedef void *ClientData;

/*
 * An event type.
 */
typedef int32_t Event;

/*
 * Some types that must be declared early, to be able to define
 * maps and such.
 */
typedef int32_t TimerId;

/*
 * Alphabetical order of forward declared classes.
 */
class Application;
class ApplicationImpl;
class CachedObjectChangeHandlers;
class Client;
class ClientImpl;
class ClusterEventHandler;
class ClusterEventPayload;
class ClusterException;
class DataDistribution;
class DataDistributionImpl;
class DistributedLocks;
class Factory;
class FactoryOps;
class Group;
class GroupImpl;
class HealthChecker;
class ManualOverride;
class Mutex;
class Node;
class NodeImpl;
class NodeAddress;
class Notifyable;
class NotifyableImpl;
struct PredMutexCond;
class Properties;
class PropertiesImpl;
class Server;
class ServerImpl;
class Shard;
class TimerEventHandler;
class TimerEventPayload;

/*
 * Vectors of pointers to these classes.
 */
typedef std::vector<ApplicationImpl *>		ApplicationImplList;
typedef std::vector<ClientImpl *>		ClientImplList;
typedef std::vector<DataDistributionImpl *>	DataDistributionImplList;
typedef std::vector<Factory *>			FactoryList;
typedef std::vector<FactoryOps *>		FactoryOpsList;
typedef std::vector<GroupImpl *>		GroupImplList;
typedef std::vector<HealthChecker *>		HealthCheckerList;
typedef std::vector<std::string>		NameList;
typedef std::vector<NodeImpl *>			NodeImplList;
typedef	std::vector<NodeAddress *>		NodeAddressList;
typedef std::vector<Server *>			ServerList;
typedef std::vector<Shard *>			ShardList;
    
/*
 * Maps of pointers to these classes.
 */
typedef std::map<std::string, Application *>	       ApplicationMap;
typedef std::map<std::string, ApplicationImpl *>       ApplicationImplMap;
typedef std::map<std::string, Client *>		       ClientMap;
typedef std::map<std::string, DataDistribution *>      DataDistributionMap;
typedef std::map<std::string, DataDistributionImpl *>  DataDistributionImplMap;
typedef std::map<std::string, Factory *>	       FactoryMap;
typedef std::map<std::string, FactoryOps *>	       FactoryOpsMap;
typedef std::map<std::string, Group *>		       GroupMap;
typedef std::map<std::string, GroupImpl *>	       GroupImplMap;
typedef std::map<std::string, HealthChecker *>	       HealthCheckerMap;
typedef std::map<std::string, std::string>             KeyValMap;
typedef std::map<std::string, ManualOverride *>	       ManualOverridesMap;
typedef std::map<std::string, Node *>	               NodeMap;
typedef std::map<std::string, NodeImpl *>	       NodeImplMap;
typedef std::map<std::string, PropertiesImpl *>	       PropertiesImplMap;
typedef std::map<std::string, Server *>		       ServerMap;
typedef std::map<TimerId, TimerEventPayload *>	       TimerRegistry;
typedef std::map<std::string, PredMutexCond *>         WaitMap;

/*
 * Support structures for multimap element comparison.
 */
struct ltstr
{
    bool operator()(std::string s1, std::string s2) const
    {
        return strcmp(s1.c_str(), s2.c_str()) < 0;
    }
};

/*
 * Multimaps.
 */
typedef std::multimap<const std::string, ServerImpl *, ltstr>	
                                               LeadershipElectionMultimap;
typedef LeadershipElectionMultimap::iterator   LeadershipIterator;
typedef std::pair<LeadershipIterator, LeadershipIterator>
					       LeadershipElectionMultimapRange;

typedef std::multimap<const std::string, ClusterEventHandler *, ltstr>
					       EventHandlersMultimap;
typedef EventHandlersMultimap::iterator	       EventHandlersIterator;
typedef std::pair<EventHandlersIterator, EventHandlersIterator>
					       EventHandlersMultimapRange;

/*
 * Type used for passing flags.
 */
typedef uint32_t ServerFlags;

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
