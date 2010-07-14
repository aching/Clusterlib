/*
 * generalcommands.cc --
 *
 * Implementation of the all the basic commands.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include <iostream>
#include <sys/utsname.h>
#include <string.h>
#include <sys/wait.h>
#include "clusterlibinternal.h"
#include "cliparams.h"
#include "cliformat.h"
#include "generalcommands.h"

using namespace std;
using namespace json;
using namespace json::rpc;

namespace clusterlib 
{

const string SetLogLevel::LEVEL_ARG = "level";

SetLogLevel::SetLogLevel()
        : CliCommand("setLogLevel", NULL)
{
    addArg(SetLogLevel::LEVEL_ARG,
           "", 
           "The new log level (0-5), 0 indicates no logging", 
           IntegerArg, 
           true);
}

void
SetLogLevel::action()
{
    int32_t level = getArg(LEVEL_ARG).getIntegerArg();
    
    switch (level) {
        case 0:
            log4cxx::Logger::getRootLogger()->getLoggerRepository()->
                setThreshold(log4cxx::Level::getOff());
            cout << "SetLogLevel: Switched to off" << endl;
            break;
        case 1:
            log4cxx::Logger::getRootLogger()->getLoggerRepository()->
                setThreshold(log4cxx::Level::getFatal());
            cout << "SetLogLevel: Switched to fatal" << endl;
            break;
        case 2:
            log4cxx::Logger::getRootLogger()->getLoggerRepository()->
                setThreshold(log4cxx::Level::getError());
            cout << "SetLogLevel: Switched to error" << endl;
            break;
        case 3:
            log4cxx::Logger::getRootLogger()->getLoggerRepository()->
                setThreshold(log4cxx::Level::getInfo());
            cout << "SetLogLevel: Switched to info" << endl;
            break;
        case 4:
            log4cxx::Logger::getRootLogger()->getLoggerRepository()->
                setThreshold(log4cxx::Level::getDebug());
            cout << "SetLogLevel: Switched to debug" << endl;
            break;
        case 5:
            log4cxx::Logger::getRootLogger()->getLoggerRepository()->
                setThreshold(log4cxx::Level::getAll());
            cout << "SetLogLevel: Switched to all" << endl;
            break;
        default:
            cout << "SetLogLevel: Level " << level 
                 << " not defined.  Should be between 0 - 5." << endl;
    }
}

string
SetLogLevel::helpMessage()
{
    return "Set the logging level of the top level logger.";
}

SetLogLevel::~SetLogLevel() 
{
}

RemoveNotifyable::RemoveNotifyable(Client *client) 
    : CliCommand("removeNotifyable", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "The notifyable to remove", 
           NotifyableArg, 
           true);
    addArg(CliCommand::CHILDREN_ARG,
           "false", 
           "True indicates remove the children as well", 
           BoolArg, 
           false);
}

void
RemoveNotifyable::action() 
{
    Notifyable *ntp = getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
        getClient()->getRoot());
    if (ntp == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    
    string key = ntp->getKey();
    string name = ntp->getName();
    bool removeChildren = 
        getArg(CliCommand::CHILDREN_ARG).getBoolArg();
    ntp->remove(removeChildren);
    cout << "Removed notifyable with name (" 
         << name << ") and key (" << key << ")" << endl;
}

string
RemoveNotifyable::helpMessage()
{
    return "Remove a notifyable with or without children";
}

RemoveNotifyable::~RemoveNotifyable() {}

GetLockBids::GetLockBids(Client *client) 
    : CliCommand("getLockBids", client)
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           getClient()->getRoot()->getKey(),
           "Locks bids on this object", 
           NotifyableArg,
           false);
    addArg(CliCommand::CHILDREN_ARG,
           "false", 
           "True indicates get lock bids of the children too", 
           BoolArg, 
           false);
}

void 
GetLockBids::action()
{
    Notifyable *ntp = getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
        getClient()->getRoot());
    if (ntp == NULL) {
        throw InvalidMethodException(
            "GetLockBids: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    
    bool children = getArg(CliCommand::CHILDREN_ARG).getBoolArg();
    NameList lockBids = ntp->getLockBids(children);
    NameList::const_iterator lockBidsIt;
    for (lockBidsIt = lockBids.begin(); 
         lockBidsIt != lockBids.end();
         ++lockBidsIt) {
        cout << *lockBidsIt << endl;
    }
}

string
GetLockBids::helpMessage()
{
    return "Find the lock bids for a Notifyable and possibly its children.";
}

GetLockBids::~GetLockBids() {}

GetChildren::GetChildren(Client *client) 
    : CliCommand("getChildren", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           getClient()->getRoot()->getKey(),
           "The notifyable to get the children", 
           NotifyableArg,
           false);
}

void 
GetChildren::action() 
{
    Notifyable *ntp = getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
        getClient()->getRoot());
    if (ntp == NULL) {
        throw InvalidMethodException(
            "GetChildren: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    
    NotifyableList::const_iterator nlIt;
    NotifyableList nl = ntp->getMyChildren();
    CliParams *params = CliParams::getInstance();
    for (nlIt = nl.begin(); nlIt != nl.end(); nlIt++) {
        cout << (*nlIt)->getKey() << endl;
        params->addToKeySet((*nlIt)->getKey());
    }
}

string
GetChildren::helpMessage()
{
    return "Find children on a Notifyable.";
}

GetChildren::~GetChildren() {}

GetAttributes::GetAttributes(Client *client) 
    : CliCommand("getAttributes", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "",
           "The notifyable to find the attributes", 
           NotifyableArg,
           true);
}

void
GetAttributes::action() 
{
    NotifyableList::const_iterator it;
    Notifyable *ntp = getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
        getClient()->getRoot());
    if (ntp == NULL) {
        throw InvalidMethodException(
            "GetAttributes: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }

    NameList names;
    NameList::const_iterator nameIt;
    CliParams *params = CliParams::getInstance();
    if (dynamic_cast<Root *>(ntp) != NULL) {
        CliFormat::attributeOut("type", "Root");
        Root *root = dynamic_cast<Root *>(ntp);
        names = root->getApplicationNames();
        if (!names.empty()) {
            for (nameIt = names.begin(); nameIt != names.end(); nameIt++) {
                params->addToKeySet(
                    root->getApplication(*nameIt)->getKey());
            }
            CliFormat::attributeOut("applications", names);
        }
    }
    else if (dynamic_cast<Group *>(ntp) != NULL) {
        if (dynamic_cast<Application *>(ntp) != NULL) {
            CliFormat::attributeOut("type", "Application");
        }
        else {
            CliFormat::attributeOut("type", "Group");
        }
        Group *group = dynamic_cast<Group *>(ntp);
        names = group->getGroupNames();
        if (!names.empty()) {
            CliFormat::attributeOut("groups", names);
            for (nameIt = names.begin(); nameIt != names.end(); nameIt++) {
                params->addToKeySet(
                    group->getGroup(*nameIt)->getKey());
            }
        }
        names = group->getDataDistributionNames();
        if (!names.empty()) {
            CliFormat::attributeOut("data distributions", names);
            for (nameIt = names.begin(); nameIt != names.end(); nameIt++) {
                params->addToKeySet(
                    group->getDataDistribution(*nameIt)->getKey());
            }
        }
        names = group->getNodeNames();
        if (!names.empty()) {
            CliFormat::attributeOut("nodes", names);
            for (nameIt = names.begin(); nameIt != names.end(); nameIt++) {
                params->addToKeySet(
                    group->getNode(*nameIt)->getKey());
            }
        }
    }
    else if (dynamic_cast<DataDistribution *>(ntp) != NULL) {
        CliFormat::attributeOut("type", "Data Distribution");
        DataDistribution *dataDistribution = 
            dynamic_cast<DataDistribution *>(ntp);
        CliFormat::attributeOut("covered", 
                                dataDistribution->cachedShards().isCovered());
    }
    else if (dynamic_cast<Node *>(ntp) != NULL) {
        CliFormat::attributeOut("type", "Node");
        Node *node = dynamic_cast<Node *>(ntp);
        bool useProcessSlots = false;
        int32_t maxProcessSlots = -1;
        useProcessSlots = node->cachedProcessSlotInfo().getEnable();
        maxProcessSlots = node->cachedProcessSlotInfo().getMaxProcessSlots();
        CliFormat::attributeOut("use process slots", useProcessSlots);
        CliFormat::attributeOut("max process slots", maxProcessSlots);

        names = node->getProcessSlotNames();
        if (!names.empty()) {
            for (nameIt = names.begin(); nameIt != names.end(); nameIt++) {
                params->addToKeySet(
                    node->getProcessSlot(*nameIt)->getKey());
            }
            CliFormat::attributeOut("process slots", names);
        }
    }
    else if (dynamic_cast<ProcessSlot *>(ntp)) {
        CliFormat::attributeOut("type", "Process Slot");
        ProcessSlot *processSlot = dynamic_cast<ProcessSlot *>(ntp);
        CliFormat::attributeOut(
            "port arr", 
            json::JSONCodec::encode(
                processSlot->cachedProcessInfo().getPortArr()));
    }
    else if (dynamic_cast<Queue *>(ntp)) {
        CliFormat::attributeOut("type", "Queue");
        Queue *queue = dynamic_cast<Queue *>(ntp);
        CliFormat::attributeOut("size", 
                                json::JSONCodec::encode(queue->size()));
    }
    else if (dynamic_cast<PropertyList *>(ntp) != NULL) {
        CliFormat::attributeOut("type", "Property List");
        PropertyList *propertyList = 
            dynamic_cast<PropertyList *>(ntp);
        vector<JSONValue::JSONString> keyVec = 
            propertyList->cachedKeyValues().getKeys();
        for (vector<JSONValue::JSONString>::const_iterator it = keyVec.begin();
             it != keyVec.end();
             ++it) {
            JSONValue jsonValue;
            bool found = propertyList->cachedKeyValues().get(*it, jsonValue);
            if (found) {
                CliFormat::attributeOut(
                    *it, jsonValue.get<JSONValue::JSONString>());
            }
        }
    }
    
    /* 
     * All notifyables can have property lists (if not a property
     * list)
     */
    if (dynamic_cast<PropertyList *>(ntp) == NULL) {
        names = ntp->getPropertyListNames();
        if (!names.empty()) {
            for (nameIt = names.begin(); nameIt != names.end(); nameIt++) {
                params->addToKeySet(
                    ntp->getPropertyList(*nameIt)->getKey());
            }
            CliFormat::attributeOut("property lists", names);
        }
    }

    CliFormat::attributeOut(
        "current state", 
        JSONCodec::encode(ntp->cachedCurrentState().getHistoryArray()));
    CliFormat::attributeOut(
        "desired state", 
        JSONCodec::encode(ntp->cachedDesiredState().getHistoryArray()));
    string id;
    int64_t msecs;
    bool hasOwner = ntp->getOwnershipInfo(&id, &msecs);
    CliFormat::attributeOut("has owner", hasOwner); 
    if (hasOwner) {
        CliFormat::attributeOut("owner id", id);
        CliFormat::attributeOut(
            "owner date connected", TimerService::getMsecsTimeString(msecs)); 
    }
}

string
GetAttributes::helpMessage()
{
    return "Get more information about a notifyable.";
}

GetAttributes::~GetAttributes() {}

SetCurrentState::SetCurrentState(Client *client) 
    : CliCommand("setCurrentState", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "Change current state of this notifyable", 
           NotifyableArg, 
           true);
    addArg(CliCommand::KEY_ARG,
           "", 
           "The current state key", 
           StringArg, 
           true);
    addArg(CliCommand::VALUE_ARG,
           "", 
           "The new value as a JSONString", 
           JsonArg, 
           true);
}

void
SetCurrentState::action() 
{
    Notifyable *ntp = getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
        getClient()->getRoot());
    if (ntp == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    
    ntp->cachedCurrentState().set(getArg(CliCommand::KEY_ARG).getStringArg(), 
                                  getArg(CliCommand::VALUE_ARG).getJsonArg());
    ntp->cachedCurrentState().publish();

    cout << "Set current state for notifyable with name= " 
         << ntp->getName() << " key='" 
         << getArg(CliCommand::KEY_ARG).getStringArg() << "',value='"
         << getArg(CliCommand::VALUE_ARG).getNativeArg() << "'" << endl;
}

string
SetCurrentState::helpMessage()
{
    return "Set a current state key-value for a notifyable.";
}

SetCurrentState::~SetCurrentState() {}

SetDesiredState::SetDesiredState(Client *client) 
    : CliCommand("setDesiredState", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "Change desired state of this notifyable", 
           NotifyableArg, 
           true);
    addArg(CliCommand::KEY_ARG,
           "", 
           "The desired state key", 
           StringArg, 
           true);
    addArg(CliCommand::VALUE_ARG,
           "", 
           "The new value as a JSONString", 
           JsonArg, 
           true);
}

void
SetDesiredState::action() 
{
    Notifyable *ntp = getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
        getClient()->getRoot());
    if (ntp == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    
    ntp->cachedDesiredState().set(getArg(CliCommand::KEY_ARG).getStringArg(), 
                                  getArg(CliCommand::VALUE_ARG).getJsonArg());
    ntp->cachedDesiredState().publish();

    cout << "Set desired state for notifyable with name= " 
         << ntp->getName() << " key='" 
         << getArg(CliCommand::KEY_ARG).getStringArg() << "',value='"
         << getArg(CliCommand::VALUE_ARG).getNativeArg() << "'" << endl;
}

string
SetDesiredState::helpMessage()
{
    return "Set the desired state for a notifyable specified by the "
        "NotifyableArg.  The first StringArg is the state key and the second"
        " StringArg is the encoded JSONValue (i.e. \"value\").";
}

SetDesiredState::~SetDesiredState() {}

AddApplication::AddApplication(Client *client) 
    : CliCommand("addApplication", client) 
{
    addArg(CliCommand::NOTIFYABLE_NAME_ARG, 
           "", 
           "Name of the new application", 
           StringArg,
           true);
} 

void 
AddApplication::action() 
{
    Root *root = getClient()->getRoot();
    if (root == NULL) {
        throw InconsistentInternalStateException(
            "action: Failed to get root");
    }
    Application *application = root->getApplication(    
        getArg(NOTIFYABLE_NAME_ARG).getStringArg(), CREATE_IF_NOT_FOUND);
    if (application == NULL) {
        throw InvalidMethodException(
            "action: Failed to get application " + 
            getArg(NOTIFYABLE_NAME_ARG).getStringArg());
    }
}

string 
AddApplication::helpMessage()
{
    return "Add an application.";
}
 
AddApplication::~AddApplication() {}

AddGroup::AddGroup(Client *client) 
    : CliCommand("addGroup", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "Add the group on this notifyable", 
           NotifyableArg, 
           true);
    addArg(CliCommand::NOTIFYABLE_NAME_ARG, 
           "", 
           "Name of the new group", 
           StringArg,
           true);
}

void
AddGroup::action() 
{
    Group *group = dynamic_cast<Group *>(
        getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
            getClient()->getRoot()));
    if (group == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    group = group->getGroup(
        getArg(NOTIFYABLE_NAME_ARG).getStringArg(), CREATE_IF_NOT_FOUND);
    if (group == NULL) {
        throw InvalidMethodException(
            "action: Failed to get group " + 
            getArg(NOTIFYABLE_NAME_ARG).getStringArg());
    }
}
 
string
AddGroup::helpMessage()
{
    return "Add a group.";
}

AddGroup::~AddGroup() {}

AddDataDistribution::AddDataDistribution(Client *client) 
    : CliCommand("addDataDistribution", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "Add the data distribution on this notifyable", 
           NotifyableArg, 
           true);
    addArg(CliCommand::NOTIFYABLE_NAME_ARG, 
           "", 
           "Name of the new data distribution", 
           StringArg,
           true);
} 

void
AddDataDistribution::action() 
{
    Group *group = dynamic_cast<Group *>(
        getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
            getClient()->getRoot()));
    if (group == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    DataDistribution *dataDistribution = 
        group->getDataDistribution(
            getArg(NOTIFYABLE_NAME_ARG).getStringArg(), CREATE_IF_NOT_FOUND);
    if (dataDistribution == NULL) {
        throw InvalidMethodException(
            "action: Failed to get data distribution " + 
            getArg(NOTIFYABLE_NAME_ARG).getStringArg());
    }
}
 
string
AddDataDistribution::helpMessage()
{
    return "Add a data distribution.";
}
 
AddDataDistribution::~AddDataDistribution() {}

AddNode::AddNode(Client *client) 
    : CliCommand("addNode", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "Add the node on this notifyable", 
           NotifyableArg, 
           true);
    addArg(CliCommand::NOTIFYABLE_NAME_ARG, 
           "", 
           "Name of the new node", 
           StringArg,
           true);
} 

void
AddNode::action() 
{
    Group *group = dynamic_cast<Group *>(
        getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
            getClient()->getRoot()));
    if (group == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    Node *node = 
        group->getNode(
            getArg(NOTIFYABLE_NAME_ARG).getStringArg(), CREATE_IF_NOT_FOUND);
    if (node == NULL) {
        throw InvalidMethodException(
            "action: Failed to get node " + 
            getArg(NOTIFYABLE_NAME_ARG).getStringArg());
    }
}
 
string
AddNode::helpMessage()
{
    return "Add a node.";
}
 
AddNode::~AddNode() {}

AddPropertyList::AddPropertyList(Client *client) 
    : CliCommand("addPropertyList", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "Add the property list on this notifyable", 
           NotifyableArg, 
           true);
    addArg(CliCommand::NOTIFYABLE_NAME_ARG, 
           "", 
           "Name of the new property list", 
           StringArg,
           true);
}
 
void
AddPropertyList::action() 
{
    Group *group = dynamic_cast<Group *>(
        getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
            getClient()->getRoot()));
    if (group == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    PropertyList *propertyList = 
        group->getPropertyList(
            getArg(NOTIFYABLE_NAME_ARG).getStringArg(), CREATE_IF_NOT_FOUND);
    if (propertyList == NULL) {
        throw InvalidMethodException(
            "action: Failed to get property List " + 
            getArg(NOTIFYABLE_NAME_ARG).getStringArg());
    }
}

string
AddPropertyList::helpMessage()
{
    return "Add a property list.";
}

AddPropertyList::~AddPropertyList() {}

AddQueue::AddQueue(Client *client) 
    : CliCommand("addQueue", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "Add the queue on this notifyable", 
           NotifyableArg, 
           true);
    addArg(CliCommand::NOTIFYABLE_NAME_ARG, 
           "", 
           "Name of the new queue", 
           StringArg,
           true);
} 

void
AddQueue::action() 
{
    Group *group = dynamic_cast<Group *>(
        getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
            getClient()->getRoot()));
    if (group == NULL) {
        throw InvalidMethodException(
            "action: Failed to get notifyable for key " +
            getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }
    Queue *queue = 
        group->getQueue(
            getArg(NOTIFYABLE_NAME_ARG).getStringArg(), CREATE_IF_NOT_FOUND);
    if (queue == NULL) {
        throw InvalidMethodException(
            "action: Failed to get queue " + 
            getArg(NOTIFYABLE_NAME_ARG).getStringArg());
    }
}

string
AddQueue::helpMessage()
{
    return "NotifyableArg is the notifyable "
        "where the queue may be added. StringArg is the "
        "name of the new queue.";
}
 
AddQueue::~AddQueue() {}

GetZnode::GetZnode(Factory *factory, Client *client)
    : CliCommand("getZnode", NULL), m_factory(factory) 
{
    addArg(CliCommand::ZKNODE_ARG,
           "", 
           "The zknode to get", 
           StringArg, 
           true);
}

void
GetZnode::action() 
{
    string zkNodePath = getArg(CliCommand::ZKNODE_ARG).getStringArg();
    if (zkNodePath.empty()) {
        throw Exception("action: Failed to get " + 
                        getArg(CliCommand::ZKNODE_ARG).getNativeArg());
    }
    string zkNodeData;
    bool found = 
        m_factory->getRepository()->getNodeData(zkNodePath, zkNodeData);
    if (!found) {
        zkNodeData = "(not found)";
    }
    CliFormat::attributeOut("data", zkNodeData);
}

string
GetZnode::helpMessage()
{
    return "Specify a znode to look up its value";
}

GetZnode::~GetZnode() {}

GetZnodeChildren::GetZnodeChildren(Factory *factory, Client *client)
    : CliCommand("getZnodeChildren", NULL), m_factory(factory) 
{
    addArg(CliCommand::ZKNODE_ARG,
           "", 
           "The zknode to get children on", 
           StringArg, 
           true);
}

void
GetZnodeChildren::action() 
{
    string zkNodePath = getArg(CliCommand::ZKNODE_ARG).getStringArg();
    if (zkNodePath.empty()) {
        throw Exception("action: Failed to get " + 
                        getArg(CliCommand::ZKNODE_ARG).getNativeArg());
    }
    vector<string> zkNodeChildren;
    string attributePrefix = "child";
    bool found = 
        m_factory->getRepository()->getNodeChildren(zkNodePath, 
                                                    zkNodeChildren);
    if (!found) {
        CliFormat::attributeOut(attributePrefix, "(none)");
    }
    else {
        stringstream ss;
        CliParams *params = CliParams::getInstance();
        for (size_t i = 0; i <  zkNodeChildren.size(); ++i) {
            ss.str("");
            ss << attributePrefix << i;
            CliFormat::attributeOut(ss.str(), zkNodeChildren[i]);
            params->addToKeySet(zkNodeChildren[i]);
        }
    }
}

GetZnodeChildren::~GetZnodeChildren() {}

string
GetZnodeChildren::helpMessage()
{
    return "Specify a znode to look up its children";
}

const string Help::COMMAND_NAME_ARG = "cmd";

Help::Help(CliParams *cliParams) 
    : CliCommand("?", NULL),
      m_params(cliParams)
{
    addArg(Help::COMMAND_NAME_ARG,
           "", 
           "Name of the command", 
           StringArg, 
           false);
}

void 
Help::action() 
{
    if (getArg(Help::COMMAND_NAME_ARG).isDefaultValue() == true) {
        m_params->printCommandNamesByGroup();
        setArg(Help::COMMAND_NAME_ARG, getCommandName());
        action();
    }
    else {
        /* Find the command and print out the options for it */
        CliCommand *command = m_params->getCommandByName(
            getArg(Help::COMMAND_NAME_ARG).getStringArg());
        if (command == NULL) {
            cout << "Unknown commmand or arg '" 
                 << getArg(Help::COMMAND_NAME_ARG).getStringArg()
                 << "'" << endl;
        }
        else {
            cout << "Command:    " << command->getCommandName() << endl
                 << "Usage:      " << command->helpMessage() << endl
                 << "Parameters: " << endl 
                 << command->generateArgUsage();
        }
    }
}

string
Help::helpMessage() 
{
    return "Get the overall command list or information about"
        " a specific command.";
}
 
Help::~Help() {}

AddAlias::AddAlias(CliParams *cliParams) 
    : CliCommand("addAlias", NULL),
      m_params(cliParams)
{
    addArg(CliCommand::KEY_ARG,
           "", 
           "The alias key", 
           StringArg, 
           true);
    addArg(CliCommand::VALUE_ARG,
           "", 
           "The new value that replaces the old one", 
           StringArg, 
           true);
} 

void 
AddAlias::action() 
{
    m_params->addAlias(
        getArg(CliCommand::KEY_ARG).getStringArg(),
        getArg(CliCommand::VALUE_ARG).getStringArg());
}

string
AddAlias::helpMessage() 
{
    return "Create an alias for replacing values.  For example, "
        "for key ntp, 'root' --> '/_clusterlib/1.0/root' ";
}
 
AddAlias::~AddAlias() {}

RemoveAlias::RemoveAlias(CliParams *cliParams) 
    : CliCommand("removeAlias", NULL),
      m_params(cliParams)
{
    addArg(CliCommand::KEY_ARG,
           "", 
           "The alias key", 
           StringArg, 
           true);
}

void 
RemoveAlias::action() 
{
    m_params->removeAlias(getArg(CliCommand::KEY_ARG).getStringArg());
}

string
RemoveAlias::helpMessage() 
{
    return "Remove an alias.";
}
 
RemoveAlias::~RemoveAlias() {}

GetAliasReplacement::GetAliasReplacement(CliParams *cliParams) 
    : CliCommand("getAliasReplacement", NULL),
      m_params(cliParams)
{
    addArg(CliCommand::KEY_ARG,
           "", 
           "The alias key", 
           StringArg, 
           true);
}

void 
GetAliasReplacement::action() 
{
    cout << "AliasReplacement: " 
         << m_params->getAliasReplacement(
             getArg(CliCommand::KEY_ARG).getStringArg());
}

string
GetAliasReplacement::helpMessage() 
{
    return "Get the alias replacement if it exists.";
}
 
GetAliasReplacement::~GetAliasReplacement() {}

const string JSONRPCCommand::REQUEST_ARG = "req";
const string JSONRPCCommand::PARAM_ARRAY_ARG = "params";

JSONRPCCommand::JSONRPCCommand(Client *client, Queue *respQueue) 
    : CliCommand("jsonRpc", client),
      m_respQueue(respQueue)
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "The queue to put the request on", 
           NotifyableArg, 
           true);
    addArg(JSONRPCCommand::REQUEST_ARG,
           "", 
           "The name of the request", 
           StringArg, 
           true);
    addArg(JSONRPCCommand::PARAM_ARRAY_ARG,
           "", 
           "The JSONArray of parameters", 
           JsonArg,
           true);
} 

void
JSONRPCCommand::action() 
{
    Queue *queue = dynamic_cast<Queue *>(
        getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
            getClient()->getRoot()));
    if (queue == NULL) {
        throw Exception("JSONRPCCommand failed to get the queue " +
                        getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }

    JSONValue::JSONObject respObj;
    JSONValue paramValue = 
        getArg(JSONRPCCommand::PARAM_ARRAY_ARG).getJsonArg();
    /*
     * Add in the response queue to the paramArr.
     */
    JSONValue::JSONArray paramArr = 
        paramValue.get<JSONValue::JSONArray>();
    JSONValue::JSONObject paramObj = 
        paramArr[0].get<JSONValue::JSONObject>();
    paramObj[ClusterlibStrings::JSONOBJECTKEY_RESPQUEUEKEY] = 
        m_respQueue->getKey();
    paramArr.clear();
    paramArr.push_back(paramObj);
    auto_ptr<GenericRequest> req;
    /* Use the generic request. */
    req.reset(new GenericRequest(
                  getClient(), 
                  getArg(JSONRPCCommand::REQUEST_ARG).getNativeArg()));
    req->setRPCParams(paramArr);
    req->setDestination(queue->getKey());
    req->sendRequest();
    req->waitResponse();
    if ((req->getResponseError()).type() != typeid(JSONValue::JSONNull)) {
        throw JSONRPCInvocationException(
            string("GenericRequest failed with error") +
            JSONCodec::encode(req->getResponseError()));
    }
    respObj = req->getResponse();
    cout << "response: " << JSONCodec::encode(respObj);
}

string
JSONRPCCommand::helpMessage()
{
    return "Send a JSON-RPC (GenericRequest).";
}

JSONRPCCommand::~JSONRPCCommand() {}

const string ManageProcessSlot::DESIRED_STATE_ARG = "ds";

ManageProcessSlot::ManageProcessSlot(Client *client) 
    : CliCommand("manageProcessSlot", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "The ProcessSlot to manage", 
           NotifyableArg, 
           true);
    addArg(ManageProcessSlot::DESIRED_STATE_ARG,
           "", 
           "0 = run once, 1 = run continous, "
           "2 = shutdown, 3 = application shutdown", 
           IntegerArg, 
           true);
} 

void
ManageProcessSlot::action() 
{
    ProcessSlot *processSlot = 
        dynamic_cast<ProcessSlot * >(
            getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
                getClient()->getRoot()));
    if (processSlot == NULL) {
        throw Exception("action: failed to get " + 
                        getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }

    JSONValue::JSONString managedState;
    int32_t desiredState = 
        getArg(ManageProcessSlot::DESIRED_STATE_ARG).getIntegerArg();
    switch (desiredState) {
        case 0:
            managedState = ProcessSlot::PROCESS_STATE_RUN_ONCE_VALUE;
            break;
        case 1:
            managedState = ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE;
            break;
        case 2:
            managedState = ProcessSlot::PROCESS_STATE_EXIT_VALUE;
            break;
        case 3:
            managedState = ProcessSlot::PROCESS_STATE_CLEANEXIT_VALUE;
            break;
        default:
            throw Exception(
                string("action: Invalid argument ") + 
                getArg(ManageProcessSlot::DESIRED_STATE_ARG).getNativeArg());
    }

    NotifyableLocker l(processSlot);

    processSlot->cachedDesiredState().set(
        ProcessSlot::PROCESS_STATE_KEY,
        managedState);
    processSlot->cachedDesiredState().set(
        ProcessSlot::PROCESS_STATE_SET_MSECS_KEY,
        TimerService::getCurrentTimeMsecs());
    processSlot->cachedDesiredState().publish();
}

string
ManageProcessSlot::helpMessage()
{
    return "Manage a ProcessSlot to start/stop.";
}
 
ManageProcessSlot::~ManageProcessSlot() {}

const string ManageActiveNode::START_ARG = "start";

ManageActiveNode::ManageActiveNode(Client *client) 
    : CliCommand("manageActiveNode", client) 
{
    addArg(CliCommand::NOTIFYABLE_ARG,
           "", 
           "The ActiveNode notifyable to manage", 
           NotifyableArg, 
           true);
    addArg(ManageActiveNode::START_ARG,
           "", 
           "True indicates start, false indicates shutdown", 
           BoolArg,
           true);
} 

void
ManageActiveNode::action() 
{
    Node *node = dynamic_cast<Node * >(
        getArg(CliCommand::NOTIFYABLE_ARG).getNotifyableArg(
            getClient()->getRoot()));
    if (node == NULL) {
        throw Exception("action: failed to get " + 
                        getArg(CliCommand::NOTIFYABLE_ARG).getNativeArg());
    }

    NotifyableLocker l(node);

    bool start = getArg(ManageActiveNode::START_ARG).getBoolArg();
    node->cachedDesiredState().set(
        Node::ACTIVENODE_SHUTDOWN, !start);
    node->cachedDesiredState().publish();
}

string
ManageActiveNode::helpMessage()
{
    return "Manage an ActiveNode.";
}
 
ManageActiveNode::~ManageActiveNode() {}

const string AggZookeeperState::ZKSERVER_LIST_ARG = "zklist";

AggZookeeperState::AggZookeeperState()
    : CliCommand("aggZookeeperState", NULL) 
{
    addArg(AggZookeeperState::ZKSERVER_LIST_ARG,
           "", 
           "The comma-separated list of zookeeper servers", 
           StringArg, 
           true);
}

void
AggZookeeperState::action() 
{
    ZookeeperPeriodicCheck singleCheck(
        0, 
        getArg(AggZookeeperState::ZKSERVER_LIST_ARG).getStringArg(),
        NULL);
    singleCheck.run();
    JSONValue::JSONObject aggStatObj = singleCheck.getAggNodeState();
    JSONValue::JSONObject::const_iterator aggStatObjIt;
    for (aggStatObjIt = aggStatObj.begin();
         aggStatObjIt != aggStatObj.end();
         ++aggStatObjIt) {
        cout << aggStatObjIt->first << "=";
        if (aggStatObjIt->second.type() == typeid(JSONValue::JSONArray)) {
            JSONValue::JSONArray arr = 
                aggStatObjIt->second.get<JSONValue::JSONArray>();
            JSONValue::JSONArray::const_iterator arrIt;
            for (arrIt = arr.begin(); arrIt != arr.end(); ++arrIt) {
                cout << arrIt->get<JSONValue::JSONString>();
                if (arrIt != arr.end() - 1) {
                    cout << ",";
                }
            }
        }
        else {
            cout << aggStatObjIt->second.get<JSONValue::JSONInteger>();
        }
        cout << "\n";
    }
}

string
AggZookeeperState::helpMessage()
{
    return "Get the aggregate zookeeper instance state.";
}
 
AggZookeeperState::~AggZookeeperState() {}


Quit::Quit(CliParams *cliParams) 
    : CliCommand("quit", NULL),
      m_params(cliParams) {} 

void
Quit::action() 
{
    m_params->setFinished();
    cout << "Exiting.." << endl;
}

string 
Quit::helpMessage()
{
    return "Exit the shell.";
}

Quit::~Quit() {}

BoolArg::BoolArg()
    : CliCommand("boolArg", NULL) { } 

void
BoolArg::action() 
{
    cout << helpMessage() << endl;
}

string
BoolArg::helpMessage()
{
    return "Valid values: 'true' or 'false'";
}

BoolArg::~BoolArg() {}

IntegerArg::IntegerArg() 
    : CliCommand("integerArg", NULL) {}

void 
IntegerArg::action() 
{
    cout << helpMessage() << endl;
}

string 
IntegerArg::helpMessage()
{
    return "Valid values: Anything that can be parsed by atoi " 
        "(i.e. -1 or 1234)";
}

IntegerArg::~IntegerArg() {}

StringArg::StringArg() 
    : CliCommand("stringArg", NULL) {}

void
StringArg::action() 
{
    cout << helpMessage() << endl;
}

string 
StringArg::helpMessage()
{
    return "Valid values: Anything normal printable string "
        "characters (no spaces)).";
}

StringArg::~StringArg() {}

NotifyableArg::NotifyableArg(Client *client) 
    : CliCommand("notifyableArg", client) {}

void
NotifyableArg::action() 
{
    cout << helpMessage() << endl;
}

string
NotifyableArg::helpMessage()
{
    return "Valid values: Any key that is a valid notifyable "
        "(i.e. " + getClient()->getRoot()->getKey() 
            + ")";
}
 
NotifyableArg::~NotifyableArg() {}

JsonArg::JsonArg() 
    : CliCommand("jsonArg", NULL) {}

void
JsonArg::action() 
{
    cout << helpMessage() << endl;
}

string
JsonArg::helpMessage()
{
    return "Valid values: A valid json string (i.e. \"hi\")";
}
 
JsonArg::~JsonArg() {}

}	/* End of 'namespace clusterlib' */
