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

#include <iostream>

namespace clusterlib
{

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
    static const std::string CLUSTERLIBVERSION;

    static const std::string CONFIGURATION;
    static const std::string ALERTS;
    static const std::string SYNC;

    static const std::string ROOT;
    static const std::string APPLICATIONS;
    static const std::string GROUPS;
    static const std::string NODES;
    static const std::string PROCESSSLOTS;
    static const std::string DISTRIBUTIONS;
    static const std::string PROPERTYLISTS;
    static const std::string QUEUES;

    static const std::string CLIENTSTATE;
    static const std::string CLIENTSTATEDESC;
    static const std::string ADDRESS;
    static const std::string LASTCONNECTED;
    static const std::string CLIENTVERSION;
    static const std::string CONNECTED;
    static const std::string QUEUE_PARENT;
    static const std::string BOUNCY;
    static const std::string READY;
    static const std::string ALIVE;
    static const std::string MASTERSETSTATE;
    static const std::string SUPPORTEDVERSIONS;
    static const std::string PROCESSSLOTSUSAGE;
    static const std::string PROCESSSLOTSMAX;

    static const std::string PROCESSSLOTPORTVEC;
    static const std::string PROCESSSLOTEXECARGS;
    static const std::string PROCESSSLOTRUNNINGEXECARGS;
    static const std::string PROCESSSLOTPID;
    static const std::string PROCESSSLOTDESIREDSTATE;
    static const std::string PROCESSSLOTCURRENTSTATE;
    static const std::string PROCESSSLOTRESERVATION;

    static const std::string PROCESSSTATE_UNUSED;
    static const std::string PROCESSSTATE_STARTED;
    static const std::string PROCESSSTATE_RUNNING;
    static const std::string PROCESSSTATE_STOPPED;
    static const std::string PROCESSSTATE_FINISHED;
    static const std::string PROCESSSTATE_FAILED;
    static const std::string PROCESSSTATE_INVALID;

    static const std::string ENABLED;
    static const std::string DISABLED;

    static const std::string DEFAULTPROPERTYLIST;
    static const std::string KEYVAL;

    /**
     * Defined PropertyList keys (prefix PLK) and values (prefix PLV).
     */
    static const std::string PLK_STATE;
    static const std::string PLV_STATE_INITIAL;
    static const std::string PLV_STATE_PREPARING;
    static const std::string PLV_STATE_READY;
    static const std::string PLV_STATE_STOPPED;

    static const std::string PLK_RPCMANAGER_REQ_POSTFIX;
    static const std::string PLK_RPCMANAGER_REQ_STATUS_POSTFIX;

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

    static const std::string QUEUEELEMENTPREFIX;

    static const std::string SHARDS;
    static const std::string GOLDENSHARDS;

    static const std::string SEQUENCE_SPLIT;

    static const std::string NOTIFYABLELOCK;
    static const std::string LEADERLOCK;

    static const std::string LOCKS;
    static const std::string BARRIERS;
    static const std::string TRANSACTIONS;

    static const std::string ENDEVENT;

    static const std::string PARTIALLOCKNODE;

    static const std::string INFLUX;
    static const std::string HEALTHY;
    static const std::string UNHEALTHY;

    /*
     * Names of predefined properties.
     */
    static const std::string HEARTBEATMULTIPLE;
    static const std::string HEARTBEATCHECKPERIOD;
    static const std::string HEARTBEATHEALTHY;
    static const std::string HEARTBEATUNHEALTHY;
    static const std::string TIMEOUTUNHEALTHYYTOR;
    static const std::string TIMEOUTUNHEALTHYRTOD;
    static const std::string TIMEOUTDISCONNECTYTOR;
    static const std::string TIMEOUTDISCONNECTRTOD;
    static const std::string NODESTATEGREEN;
    static const std::string NODEBOUNCYPERIOD;
    static const std::string NODEBOUNCYNEVENTS;
    static const std::string NODEMOVEBACKPERIOD;
    static const std::string CLUSTERUNMANAGED;
    static const std::string CLUSTERDOWN;
    static const std::string CLUSTERFLUXPERIOD;
    static const std::string CLUSTERFLUXNEVENTS;
    static const std::string HISTORYSIZE;
    static const std::string LEADERFAILLIMIT;
    static const std::string SERVERBIN;
    
    /*
     * Names associated with the special clusterlib master
     * application.
     */
    static const std::string MASTER;

    /**
     * Default CLI application for issuing JSON-RPC 
     */
    static const std::string DEFAULT_CLI_APPLICATION;

    /*
     * Strings associated with clusterlib rpc methods
     */
    static const std::string RPC_START_PROCESS;
    static const std::string RPC_STOP_PROCESS;
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
    static const std::string JSONOBJECTKEY_CONNECTEDID;
    static const std::string JSONOBJECTKEY_TIME;

  private:
    ClusterlibStrings()
    {
        throw InvalidMethodException("Someone called the ClusterlibStrings "
                                     "default constructor!");
    }
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CLUSTERLIBSTRINGS_H_ */
