/*
 * clusterlibstrings.h --
 *
 * Definition of ClusterlibStrings.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CLUSTERLIBSTRINGS_H_
#define	_CL_CLUSTERLIBSTRINGS_H_

namespace clusterlib {

/**
 * Class containing static variables for all string constants.
 */
class ClusterlibStrings
{
  public:
    /*
     * All string constants used to name ZK nodes.
     */
    static const std::string ROOTNODE;
    static const std::string KEYSEPARATOR;

    static const std::string CLUSTERLIB;
    static const std::string CLUSTERLIB_VERSION;

    static const std::string SYNC;

    static const std::string ROOT;
    static const std::string APPLICATIONS;
    static const std::string GROUPS;
    static const std::string NODES;
    static const std::string PROCESSSLOTS;
    static const std::string DISTRIBUTIONS;
    static const std::string PROPERTYLISTS;
    static const std::string QUEUES;

    /*
     * Registered notifyable names
     */
    static const std::string REGISTERED_ROOT_NAME;
    static const std::string REGISTERED_APPLICATION_NAME;
    static const std::string REGISTERED_GROUP_NAME;
    static const std::string REGISTERED_NODE_NAME;
    static const std::string REGISTERED_PROCESSSLOT_NAME;
    static const std::string REGISTERED_DATADISTRIBUTION_NAME;
    static const std::string REGISTERED_PROPERTYLIST_NAME;
    static const std::string REGISTERED_QUEUE_NAME;
    
    static const std::string NOTIFYABLESTATE_JSON_OBJECT;
    static const std::string CURRENT_STATE_JSON_VALUE;
    static const std::string DESIRED_STATE_JSON_VALUE;

    static const std::string QUEUE_PARENT;
    static const std::string PROCESSSLOT_INFO_JSON_OBJECT;
    static const std::string PROCESSSLOTSUSAGE;
    static const std::string PROCESSSLOTSMAX;

    static const std::string JSON_PROCESSSTATE_STATE_KEY;
    static const std::string JSON_PROCESSSTATE_MSECS_KEY;
    static const std::string JSON_PROCESSSTATE_DATE_KEY;

    static const std::string DEFAULT_JSON_OBJECT;
    static const std::string DEFAULTPROPERTYLIST;
    static const std::string KEYVAL;
    static const std::string KEYVAL_JSON_OBJECT;
    static const std::string PROCESSINFO_JSON_OBJECT;

    /**
     * Defined PropertyList keys (prefix PLK) and values (prefix PLV).
     */
    static const std::string PLK_STATE;
    static const std::string PLV_STATE_INITIAL;
    static const std::string PLV_STATE_PREPARING;
    static const std::string PLV_STATE_RUNNING;
    static const std::string PLV_STATE_READY;
    static const std::string PLV_STATE_REMOVED;
    static const std::string PLV_STATE_COMPLETED;
    static const std::string PLV_STATE_HALTING;
    static const std::string PLV_STATE_STOPPED;
    static const std::string PLV_STATE_FAILED;

    static const std::string PLK_RPCMANAGER_REQ_POSTFIX;
    static const std::string PLK_RPCMANAGER_REQ_STATUS_POSTFIX;
    static const std::string PLK_PORT_RANGE_START;
    static const std::string PLK_PORT_RANGE_END;
    static const std::string PLK_USED_PORT_JSON_ARRAY;

    /** 
     * Default recv queue a notifyable 
     */
    static const std::string DEFAULT_RECV_QUEUE;

    /** 
     * Default response queue 
     */
    static const std::string DEFAULT_RESP_QUEUE;

    /** 
     * Default completed message queue (no response queue set),
     * unparseable, or debugging enabled.
     */
    static const std::string DEFAULT_COMPLETED_QUEUE;

    static const std::string QUEUE_ELEMENT_PREFIX;

    static const std::string SHARDS;
    static const std::string SHARD_JSON_OBJECT;

    static const std::string SEQUENCE_SPLIT;

    /**
     * User lock on a Notifyable object.
     */
    static const std::string NOTIFYABLE_LOCK;

    /**
     * User lock that can denote "ownership" of a Notifyable object.
     */
    static const std::string OWNERSHIP_LOCK;

    /**
     * Lock that allows manipulation of a Notifyable object's
     * children.  ClusterlibS uses this lock when creating and
     * removing Notifyable objects.  Users are allowed to
     * acquire/release this lock as well.
     */
    static const std::string CHILD_LOCK;

    static const std::string LOCKS;
    static const std::string BARRIERS;
    static const std::string TRANSACTIONS;

    static const std::string ENDEVENT;

    /**
     * Used to detect whether the ZooKeeper node is part of a lock.
     */
    static const std::string PARTIAL_LOCK_NODE;

    /**
     * Default CLI application for issuing JSON-RPC 
     */
    static const std::string DEFAULT_CLI_APPLICATION;

    static const std::string STATE_SET_MSECS;
    static const std::string STATE_SET_MSECS_AS_DATE;

    /*
     * Strings associated with clusterlib rpc methods
     */
    static const std::string RPC_START_PROCESS;
    static const std::string RPC_STOP_PROCESS;
    static const std::string RPC_STOP_ACTIVENODE;
    static const std::string RPC_GENERIC;

    /*
     * Known json object keys
     */
    static const std::string JSONOBJECTKEY_METHOD;
    static const std::string JSONOBJECTKEY_ADDENV;
    static const std::string JSONOBJECTKEY_PATH;
    static const std::string JSONOBJECTKEY_COMMAND;
    static const std::string JSONOBJECTKEY_RESPQUEUEKEY;
    static const std::string JSONOBJECTKEY_NOTIFYABLEKEY;
    static const std::string JSONOBJECTKEY_SIGNAL;
    static const std::string JSONOBJECTKEY_TIME;

    /*
     * Known state keys
     */
    static const std::string ZK_RUOK_STATE_KEY;
    static const std::string ZK_ENVI_STATE_KEY;
    static const std::string ZK_REQS_STATE_KEY;
    static const std::string ZK_STAT_STATE_KEY;
    static const std::string ZK_AGG_NODES_STATE_KEY;

  private:
    /**
     * No constructing.
     */
    ClusterlibStrings();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CLUSTERLIBSTRINGS_H_ */
