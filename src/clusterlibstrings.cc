/*
 * clusterlibstrings.cc --
 *
 * Implementation of ClusterlibStrings.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;

namespace clusterlib
{

/*
 * All the string constants needed to construct and deconstruct
 * ZK keys.
 */
const string ClusterlibStrings::ROOTNODE = "/";
const string ClusterlibStrings::KEYSEPARATOR = "/";

const string ClusterlibStrings::CLUSTERLIB = "_clusterlib";
const string ClusterlibStrings::CLUSTERLIBVERSION = "_1.0";

const string ClusterlibStrings::PROPERTIES = "_properties";
const string ClusterlibStrings::CONFIGURATION = "_configuration";
const string ClusterlibStrings::ALERTS = "_alerts";
const string ClusterlibStrings::SYNC = "_sync";

const string ClusterlibStrings::ROOT = "_root";
const string ClusterlibStrings::APPLICATIONS = "_applications";
const string ClusterlibStrings::GROUPS = "_groups";
const string ClusterlibStrings::NODES = "_nodes";
const string ClusterlibStrings::DISTRIBUTIONS = "_distributions";

const string ClusterlibStrings::CLIENTSTATE = "_clientState";
const string ClusterlibStrings::CLIENTSTATEDESC = "_clientStateDesc";
const string ClusterlibStrings::ADDRESS = "_address";
const string ClusterlibStrings::LASTCONNECTED = "_lastConnected";
const string ClusterlibStrings::CLIENTVERSION = "_clientVersion";
const string ClusterlibStrings::CONNECTED = "_connected";
const string ClusterlibStrings::BOUNCY = "_bouncy";
const string ClusterlibStrings::READY = "_ready";
const string ClusterlibStrings::ALIVE = "_alive";
const string ClusterlibStrings::MASTERSETSTATE = "_masterSetState";
const string ClusterlibStrings::SUPPORTEDVERSIONS = "_supportedVersions";

const string ClusterlibStrings::KEYVAL = "_keyval";    

const string ClusterlibStrings::SHARDS = "_shards";
const string ClusterlibStrings::GOLDENSHARDS = "_goldenShards";
const string ClusterlibStrings::MANUALOVERRIDES = "_manualOverrides";

const string ClusterlibStrings::BID_SPLIT = "=";

const string ClusterlibStrings::NOTIFYABLELOCK = "_notifyableLock";
const string ClusterlibStrings::LEADERLOCK = "_leaderLock";

const string ClusterlibStrings::LOCKS = "_locks";
const string ClusterlibStrings::QUEUES = "_queues";
const string ClusterlibStrings::BARRIERS = "_barriers";
const string ClusterlibStrings::TRANSACTIONS = "_transactions";

const string ClusterlibStrings::ENDEVENT = "_end event";

/*
 * All strings that are used as ZK values or part of values.
 */
const string ClusterlibStrings::INFLUX = "_influx";
const string ClusterlibStrings::HEALTHY = "_healthy";
const string ClusterlibStrings::UNHEALTHY = "_unhealthy";

/*
 * All strings that are used as propreties.
 */

const string ClusterlibStrings::HEARTBEATMULTIPLE = "_heartBeat.multiple";
const string ClusterlibStrings::HEARTBEATCHECKPERIOD= "heartBeat.checkPeriod";
const string ClusterlibStrings::HEARTBEATHEALTHY = "_heartBeat.healthy";
const string ClusterlibStrings::HEARTBEATUNHEALTHY = "_heatBeat.unhealthy";
const string ClusterlibStrings::TIMEOUTUNHEALTHYYTOR = 
    "_timeOut.unhealthyYToR";
const string ClusterlibStrings::TIMEOUTUNHEALTHYRTOD = 
    "_timeOut.unhealthyRToR";
const string ClusterlibStrings::TIMEOUTDISCONNECTYTOR = 
    "_timeOut.disconnectYToR";
const string ClusterlibStrings::TIMEOUTDISCONNECTRTOD = 
    "_timeOut.disconnectRToR";
const string ClusterlibStrings::NODESTATEGREEN = "_node.state.green";
const string ClusterlibStrings::NODEBOUNCYPERIOD = "_nodeBouncy.period";
const string ClusterlibStrings::NODEBOUNCYNEVENTS = "_nodeBouncy.nEvents";
const string ClusterlibStrings::NODEMOVEBACKPERIOD = "_nodeMoveBack.period";
const string ClusterlibStrings::CLUSTERUNMANAGED = "_cluster.unmanaged";
const string ClusterlibStrings::CLUSTERDOWN = "_cluster.down";
const string ClusterlibStrings::CLUSTERFLUXPERIOD = "_cluster.fluxPeriod";
const string ClusterlibStrings::CLUSTERFLUXNEVENTS = "_cluster.fluxNEvents";
const string ClusterlibStrings::HISTORYSIZE = "_history.size";
const string ClusterlibStrings::LEADERFAILLIMIT = "_leader.failLimit";
const string ClusterlibStrings::SERVERBIN = "_server.bin";

/*
 * Names associated with the special clusterlib master application.
 */
const string ClusterlibStrings::MASTER = "_master";

};	/* End of 'namespace clusterlib' */
