/*
 * clusterlibstrings.h --
 *
 * Definition of ClusterlibStrings.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CLUSTERLIBSTRINGS_H_
#define	_CLUSTERLIBSTRINGS_H_

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
    static const std::string DISTRIBUTIONS;
    static const std::string PROPERTIES;

    static const std::string CLIENTSTATE;
    static const std::string CLIENTSTATEDESC;
    static const std::string ADDRESS;
    static const std::string LASTCONNECTED;
    static const std::string CLIENTVERSION;
    static const std::string CONNECTED;
    static const std::string BOUNCY;
    static const std::string READY;
    static const std::string ALIVE;
    static const std::string MASTERSETSTATE;
    static const std::string SUPPORTEDVERSIONS;

    static const std::string KEYVAL;

    static const std::string SHARDS;
    static const std::string GOLDENSHARDS;
    static const std::string MANUALOVERRIDES;

    static const std::string BID_SPLIT;

    static const std::string NOTIFYABLELOCK;
    static const std::string LEADERLOCK;

    static const std::string LOCKS;
    static const std::string QUEUES;
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

  private:
    ClusterlibStrings()
    {
        throw InvalidMethodException("Someone called the ClusterlibStrings "
                                     "default constructor!");
    }
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CLUSTERLIBSTRINGS_H_ */
