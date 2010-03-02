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

const string ClusterlibStrings::CONFIGURATION = "_configuration";
const string ClusterlibStrings::ALERTS = "_alerts";
const string ClusterlibStrings::SYNC = "_sync";

const string ClusterlibStrings::ROOT = "_root";
const string ClusterlibStrings::APPLICATIONS = "_applications";
const string ClusterlibStrings::GROUPS = "_groups";
const string ClusterlibStrings::NODES = "_nodes";
const string ClusterlibStrings::PROCESSSLOTS = "_processSlots";
const string ClusterlibStrings::DISTRIBUTIONS = "_distributions";
const string ClusterlibStrings::PROPERTYLISTS = "_propertyLists";
const string ClusterlibStrings::QUEUES = "_queues";

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
const string ClusterlibStrings::PROCESSSLOTSUSAGE = "_processSlotsUsage";
const string ClusterlibStrings::PROCESSSLOTSMAX = "_processSlotsMax";

const string ClusterlibStrings::PROCESSSLOTPORTVEC = "_processSlotPortVec";
const string ClusterlibStrings::PROCESSSLOTEXECARGS = 
    "_processSlotExecArgs";
const string ClusterlibStrings::PROCESSSLOTRUNNINGEXECARGS = 
    "_processSlotRunningExecArgs";
const string ClusterlibStrings::PROCESSSLOTPID = "_processSlotPID";
const string ClusterlibStrings::PROCESSSLOTDESIREDSTATE = 
    "_processSlotDesiredState";
const string ClusterlibStrings::PROCESSSLOTCURRENTSTATE = 
    "_processSlotCurrentState";
const string ClusterlibStrings::PROCESSSLOTRESERVATION = 
    "_processSlotReservation";

const string ClusterlibStrings::PROCESSSTATE_UNUSED = "_unused";
const string ClusterlibStrings::PROCESSSTATE_STARTED = "_started";
const string ClusterlibStrings::PROCESSSTATE_RUNNING = "_running";
const string ClusterlibStrings::PROCESSSTATE_STOPPED = "_stopped";
const string ClusterlibStrings::PROCESSSTATE_FINISHED = "_finished";
const string ClusterlibStrings::PROCESSSTATE_FAILED = "_failed";
const string ClusterlibStrings::PROCESSSTATE_INVALID = "_invalid";

const string ClusterlibStrings::ENABLED = "_enabled";
const string ClusterlibStrings::DISABLED = "_disabled";

const string ClusterlibStrings::DEFAULTPROPERTYLIST = "_defaultPropertyList";
const string ClusterlibStrings::KEYVAL = "_keyval";

const string ClusterlibStrings::PLK_STATE = "_state";
const string ClusterlibStrings::PLV_STATE_INITIAL = "_initial";
const string ClusterlibStrings::PLV_STATE_PREPARING = "_preparing";
const string ClusterlibStrings::PLV_STATE_READY = "_ready";
const string ClusterlibStrings::PLV_STATE_STOPPED = "_stopped";

const string ClusterlibStrings::DEFAULT_RECV_QUEUE = "_defaultRecvQueue";
const string ClusterlibStrings::DEFAULT_RESP_QUEUE = "_defaultRespQueue";
const string ClusterlibStrings::DEFAULT_COMPLETED_QUEUE = 
    "_defaultCompletedQueue";

const string ClusterlibStrings::QUEUEELEMENTPREFIX = "_queueElementPrefix";

const string ClusterlibStrings::SHARDS = "_shards";
const string ClusterlibStrings::GOLDENSHARDS = "_goldenShards";

const string ClusterlibStrings::SEQUENCE_SPLIT = "_";

const string ClusterlibStrings::NOTIFYABLELOCK = "_notifyableLock";
const string ClusterlibStrings::LEADERLOCK = "_leaderLock";

const string ClusterlibStrings::LOCKS = "_locks";
const string ClusterlibStrings::BARRIERS = "_barriers";
const string ClusterlibStrings::TRANSACTIONS = "_transactions";

const string ClusterlibStrings::ENDEVENT = "_end event";
    
const string ClusterlibStrings::PARTIALLOCKNODE = 
    ClusterlibStrings::KEYSEPARATOR + 
    ClusterlibStrings::LOCKS + 
    ClusterlibStrings::KEYSEPARATOR;

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

const string ClusterlibStrings::DEFAULT_CLI_APPLICATION = "_cli";

/*
 * Names associated with the special clusterlib master application.
 */
const string ClusterlibStrings::MASTER = "_master";

const string ClusterlibStrings::RPC_START_PROCESS = "_startProcess";
const string ClusterlibStrings::RPC_STOP_PROCESS = "_stopProcess";
const string ClusterlibStrings::RPC_GENERIC = "_generic";

const string ClusterlibStrings::JSONOBJECTKEY_METHOD = "_method";
const string ClusterlibStrings::JSONOBJECTKEY_ADDENV = "_env";
const string ClusterlibStrings::JSONOBJECTKEY_PATH = "_path";
const string ClusterlibStrings::JSONOBJECTKEY_COMMAND = "_command";
const string ClusterlibStrings::JSONOBJECTKEY_RESPQUEUEKEY = "_respQueueKey";
const string ClusterlibStrings::JSONOBJECTKEY_NOTIFYABLEKEY = "_notifyableKey";
const string ClusterlibStrings::JSONOBJECTKEY_SIGNAL = "_signal";
const string ClusterlibStrings::JSONOBJECTKEY_CONNECTEDID = "_connectedId";
const string ClusterlibStrings::JSONOBJECTKEY_TIME = "_time";

};	/* End of 'namespace clusterlib' */
