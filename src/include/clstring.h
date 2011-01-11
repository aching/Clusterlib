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

#ifndef	_CL_CLSTRING_H_
#define	_CL_CLSTRING_H_

namespace clusterlib {

/**
 * All user available static string variables are declared here.
 */
class CLString {
  public:
    /**
     * Registered Root name
     */
    const static std::string REGISTERED_ROOT_NAME;
    /**
     * Registered Application name
     */
    const static std::string REGISTERED_APPLICATION_NAME;
    /**
     * Registered Group name
     */
    const static std::string REGISTERED_GROUP_NAME;
    /**
     * Registered Node name
     */
    const static std::string REGISTERED_NODE_NAME;
    /**
     * Registered ProcessSlot name
     */
    const static std::string REGISTERED_PROCESSSLOT_NAME;
    /**
     * Registered DataDistribution name
     */
    const static std::string REGISTERED_DATADISTRIBUTION_NAME;
    /**
     * Registered PropertyList name
     */
    const static std::string REGISTERED_PROPERTYLIST_NAME;
    /**
     * Registered Queue name
     */
    const static std::string REGISTERED_QUEUE_NAME;

    /** 
     * Default PropertyList name for a Notifyable
     */
    const static std::string DEFAULT_PROPERTYLIST;
    /** 
     * Default recv queue name for a Notifyable
     */
    const static std::string DEFAULT_RECV_QUEUE;
    /** 
     * Default response queue name for a Notifyable
     */
    const static std::string DEFAULT_RESP_QUEUE;
    /** 
     * Default completed message queue (no response queue set),
     * unparseable, or debugging enabled.
     */
    const static std::string DEFAULT_COMPLETED_QUEUE;
    /**
     * Default CLI application for issuing JSON-RPC 
     */
    const static std::string DEFAULT_CLI_APPLICATION;

    /**
     * User lock on a Notifyable object.
     */
    const static std::string NOTIFYABLE_LOCK;
    /**
     * User lock that can denote "ownership" of a Notifyable object.
     */
    const static std::string OWNERSHIP_LOCK;
    /**
     * User and internal lock that allows manipulation of a Notifyable
     * object's children.  ClusterlibS uses this lock when creating
     * and removing Notifyable objects.  Users are allowed to
     * acquire/release this lock as well.
     */
    const static std::string CHILD_LOCK;

    /**
     * Defined PropertyList keys (prefix PLK) and values (prefix PLV).
     */
    const static std::string PLK_STATE;
    const static std::string PLV_STATE_INITIAL;
    const static std::string PLV_STATE_PREPARING;
    const static std::string PLV_STATE_RUNNING;
    const static std::string PLV_STATE_READY;
    const static std::string PLV_STATE_REMOVED;
    const static std::string PLV_STATE_COMPLETED;
    const static std::string PLV_STATE_HALTING;
    const static std::string PLV_STATE_STOPPED;
    const static std::string PLV_STATE_FAILED;

    const static std::string PLK_RPCMANAGER_REQ_POSTFIX;
    const static std::string PLK_RPCMANAGER_REQ_STATUS_POSTFIX;
    const static std::string PLK_PORT_RANGE_START;
    const static std::string PLK_PORT_RANGE_END;
    const static std::string PLK_USED_PORT_JSON_ARRAY;

    /*
     * Strings associated with clusterlib rpc methods
     */
    const static std::string RPC_START_PROCESS;
    const static std::string RPC_STOP_PROCESS;
    const static std::string RPC_STOP_ACTIVENODE;
    const static std::string RPC_GENERIC;

    /*
     * Known json object keys
     */
    const static std::string JSONOBJECTKEY_METHOD;
    const static std::string JSONOBJECTKEY_ADDENV;
    const static std::string JSONOBJECTKEY_PATH;
    const static std::string JSONOBJECTKEY_COMMAND;
    const static std::string JSONOBJECTKEY_RESPQUEUEKEY;
    const static std::string JSONOBJECTKEY_NOTIFYABLEKEY;
    const static std::string JSONOBJECTKEY_SIGNAL;
    const static std::string JSONOBJECTKEY_TIME;

    /*
     * Known state keys
     */
    const static std::string STATE_SET_MSECS;
    const static std::string STATE_SET_MSECS_AS_DATE;
    const static std::string ZK_RUOK_STATE_KEY;
    const static std::string ZK_ENVI_STATE_KEY;
    const static std::string ZK_REQS_STATE_KEY;
    const static std::string ZK_STAT_STATE_KEY;
    const static std::string ZK_AGG_NODES_STATE_KEY;

    /**
     * Directory of Root objects (only one)
     */
    const static std::string ROOT_DIR;
    /**
     * Directory of Application objects
     */
    const static std::string APPLICATION_DIR;
    /**
     * Directory of Group objects
     */
    const static std::string GROUP_DIR;
    /**
     * Directory of Node objects
     */
    const static std::string NODE_DIR;
    /**
     * Directory of ProcessSlot objects
     */
    const static std::string PROCESSSLOT_DIR;
    /**
     * Directory of DataDistribution objects
     */
    const static std::string DATADISTRIBUTION_DIR;
    /**
     * Directory of PropertyList objects
     */
    const static std::string PROPERTYLIST_DIR;
    /**
     * Directory of Queued objects
     */
    const static std::string QUEUE_DIR;

    /**
     * Separates all Notifyable keys into components.
     */
    const static std::string KEY_SEPARATOR;

  private:
    /**
     * No constructing.
     */
    CLString();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CLSTRING_H_ */
