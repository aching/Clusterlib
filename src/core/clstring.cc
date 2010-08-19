/*
 * clstring.cc --
 *
 * Implementation of CLString and CLStringInternal.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

using namespace std;

namespace clusterlib {

const string CLString::CLString::REGISTERED_ROOT_NAME = "root";
const string CLString::CLString::REGISTERED_APPLICATION_NAME = "application";
const string CLString::CLString::REGISTERED_GROUP_NAME = "group";
const string CLString::CLString::REGISTERED_NODE_NAME = "node";
const string CLString::CLString::REGISTERED_PROCESSSLOT_NAME= "process slot";
const string CLString::CLString::REGISTERED_DATADISTRIBUTION_NAME = 
    "data distribution";
const string CLString::CLString::REGISTERED_PROPERTYLIST_NAME = 
    "property list";
const string CLString::CLString::REGISTERED_QUEUE_NAME = "queue";

const string CLString::DEFAULT_PROPERTYLIST = "_defaultPropertyList";
const string CLString::DEFAULT_RECV_QUEUE = "_defaultRecvQueue";
const string CLString::DEFAULT_RESP_QUEUE = "_defaultRespQueue";
const string CLString::DEFAULT_COMPLETED_QUEUE = "_defaultCompletedQueue";
const string CLString::DEFAULT_CLI_APPLICATION = "_cli";

const string CLString::CLString::NOTIFYABLE_LOCK = "_notifyableLock";
const string CLString::CLString::OWNERSHIP_LOCK = "_ownershipLock";
const string CLString::CLString::CHILD_LOCK = "_childLock";

const string CLString::PLK_STATE = "_plkState";
const string CLString::PLV_STATE_INITIAL = "_plvInitial";
const string CLString::PLV_STATE_PREPARING = "_plvPreparing";
const string CLString::PLV_STATE_RUNNING = "_plvRunning";
const string CLString::PLV_STATE_READY = "_plvReady";
const string CLString::PLV_STATE_REMOVED = "_plvRemoved";
const string CLString::PLV_STATE_COMPLETED = "_plvCompleted";
const string CLString::PLV_STATE_HALTING = "_plvHalting";
const string CLString::PLV_STATE_STOPPED = "_plvStopped";
const string CLString::PLV_STATE_FAILED = "_pvsFailed";
const string CLString::PLK_RPCMANAGER_REQ_POSTFIX = " current request";
const string CLString::PLK_RPCMANAGER_REQ_STATUS_POSTFIX =
    " current request status";
const string CLString::PLK_PORT_RANGE_START = "_portRangeStart";
const string CLString::PLK_PORT_RANGE_END = "_portRangeEnd";
const string CLString::PLK_USED_PORT_JSON_ARRAY = "_usedPortJsonArray";

const string CLString::RPC_START_PROCESS = "_startProcess";
const string CLString::RPC_STOP_PROCESS = "_stopProcess";
const string CLString::RPC_STOP_ACTIVENODE = "_stopActiveNode";
const string CLString::RPC_GENERIC = "_generic";

const string CLString::JSONOBJECTKEY_METHOD = "_method";
const string CLString::JSONOBJECTKEY_ADDENV = "_env";
const string CLString::JSONOBJECTKEY_PATH = "_path";
const string CLString::JSONOBJECTKEY_COMMAND = "_command";
const string CLString::JSONOBJECTKEY_RESPQUEUEKEY = "_respQueueKey";
const string CLString::JSONOBJECTKEY_NOTIFYABLEKEY = "_notifyableKey";
const string CLString::JSONOBJECTKEY_SIGNAL = "_signal";
const string CLString::JSONOBJECTKEY_TIME = "_time";

const string CLString::STATE_SET_MSECS = "_setMsecs";
const string CLString::STATE_SET_MSECS_AS_DATE = "_setMsecsAsDate";
const string CLString::ZK_RUOK_STATE_KEY = "Zookeeper ruok";
const string CLString::ZK_ENVI_STATE_KEY = "Zookeeper environment";
const string CLString::ZK_REQS_STATE_KEY = "Zookeeper reqs";
const string CLString::ZK_STAT_STATE_KEY = "Zookeeper statistics";
const string CLString::ZK_AGG_NODES_STATE_KEY = "Zookeeper aggregate state";

const string CLString::ROOT_DIR = "_rootDir";
const string CLString::APPLICATION_DIR = "_applicationDir";
const string CLString::GROUP_DIR = "_groupDir";
const string CLString::NODE_DIR = "_nodeDir";
const string CLString::PROCESSSLOT_DIR = "_processSlotDir";
const string CLString::DATADISTRIBUTION_DIR = "_dataDistributionDir";
const string CLString::PROPERTYLIST_DIR = "_propertyListDir";
const string CLString::QUEUE_DIR = "_queueDir";

const string CLString::KEY_SEPARATOR = "/";

const string CLStringInternal::ROOT_ZNODE = "/";
const string CLStringInternal::CLUSTERLIB = "_clusterlib";
const string CLStringInternal::CLUSTERLIB_VERSION = "_1.0";
const string CLStringInternal::SYNC = "_sync";
const string CLStringInternal::CURRENT_STATE_JSON_VALUE = 
    "_currentStateJsonValue";
const string CLStringInternal::DESIRED_STATE_JSON_VALUE = 
    "_desiredStateJsonValue";
const string CLStringInternal::QUEUE_PARENT = "_queueParent";
const string CLStringInternal::QUEUE_ELEMENT_PREFIX = "_queueElementPrefix";
const string CLStringInternal::PROCESSSLOT_INFO_JSON_OBJECT = 
    "_processSlotInfoJsonObject";
const string CLStringInternal::DEFAULT_JSON_OBJECT = "_defaultJsonObject";
const string CLStringInternal::KEYVAL_JSON_OBJECT = "_keyvalJsonObject";
const string CLStringInternal::PROCESSINFO_JSON_OBJECT = 
    "_processInfoJsonObject";
const string CLStringInternal::SHARD_JSON_OBJECT = "_shardJsonObject";
const string CLStringInternal::SEQUENCE_SPLIT = " ";
const string CLStringInternal::LOCK_DIR = "_lockDir";
const string CLStringInternal::BARRIER_DIR = "_barrierDir";
const string CLStringInternal::TRANSACTION_DIR = "_transactionDir";
const string CLStringInternal::END_EVENT = "_endEvent";
const string CLStringInternal::PARTIAL_LOCK_NODE = 
CLString::KEY_SEPARATOR + CLStringInternal::LOCK_DIR + 
    CLString::KEY_SEPARATOR;

}	/* End of 'namespace clusterlib' */
