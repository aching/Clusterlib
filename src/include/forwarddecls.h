/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_FORWARDDECLS_H_
#define	_CL_FORWARDDECLS_H_

/*
 * Forward declarations for classes that need to be
 * forward-declared.
 */

namespace clusterlib {

/*
 * Define the types associated with a hash function.
 */
typedef int32_t  HashFunctionId;

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
class CachedMetadata;
class CachedCurrentState;
class CachedCurrentStateImpl;
class CachedKeyValues;
class CachedKeyValuesImpl;
class CachedProcessInfo;
class CachedProcessInfoImpl;
class CachedProcessSlotInfo;
class CachedProcessSlotInfoImpl;
class CachedShards;
class CachedShardsImpl;
class CachedState;
class CachedStateImpl;
class CachedObjectChangeHandlers;
struct CallbackAndContext;
class CallbackAndContextManager;
class Client;
class ClientImpl;
class ClusterlibRPCRequest;
class ClusterlibRPCManager;
class ClusterlibRPCMethod;
class DataDistribution;
class DataDistributionImpl;
class DistributedLocks;
class Factory;
class FactoryOps;
class Group;
class GroupImpl;
class HashRange;
class HealthChecker;
class InternalChangeHandlers;
class JSONRPCMethodHandler;
class JSONRPCRequest;
class JSONRPCResponseHandler;
class Mutex;
class Node;
class NodeImpl;
class Notifyable;
class NotifyableImpl;
class Periodic;
class PredMutexCond;
class ProcessSlot;
class ProcessSlotImpl;
class PropertyList;
class PropertyListImpl;
class RegisteredNotifyable;
class RegisteredNotifyableImpl;
class RegisteredRootImpl;
class RegisteredApplicationImpl;
class RegisteredGroupImpl;
class RegisteredDatadistributionImpl;
class RegisteredNodeImpl;
class RegisteredQueueImpl;
class RegisteredProcessSlotImpl;
class RegisteredPropertyListImpl;
class Root;
class RootImpl;
class Queue;
class QueueImpl;
class SafeNotifyableMap;
class Server;
class ServerImpl;
class Shard;
class ShardTreeData;
class SignalMap;
class TimerEventHandler;
class TimerEventPayload;
class TimerService;
class Uint64HashRange;
class UnknownHashRange;
class UserEventHandler;
class UserEventPayload;

/*
 * Vectors of pointers to these classes.
 */
typedef std::vector<ClientImpl *>		    ClientImplList;
typedef std::vector<HealthChecker *>	            HealthCheckerList;
typedef std::vector<std::string>                    NameList;
typedef std::vector<boost::shared_ptr<Notifyable> > NotifyableList;
typedef std::vector<Shard *>			    ShardList;
    
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
typedef std::map<std::string, Node *>	               NodeMap;
typedef std::map<std::string, NodeImpl *>	       NodeImplMap;
typedef std::map<std::string, NotifyableImpl *>	       NotifyableImplMap;
typedef std::map<std::string, PropertyListImpl *>      PropertyListImplMap;
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

typedef std::multimap<const std::string, UserEventHandler *, ltstr>
					       EventHandlersMultimap;
typedef EventHandlersMultimap::iterator	       EventHandlersIterator;
typedef std::pair<EventHandlersIterator, EventHandlersIterator>
					       EventHandlersMultimapRange;

/*
 * Forward declaration of EventSource.
 */
template<typename E>
class EventSource;

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

}	/* End of 'namespace clusterlib' */

namespace zk {

class ZooKeeperAdapter;

}	/* End of 'namespace zk' */

#endif	/* !_CL_FORWARDDECLS_H_ */
