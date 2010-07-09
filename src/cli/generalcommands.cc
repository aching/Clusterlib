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
using namespace clusterlib;
using namespace json;
using namespace json::rpc;

namespace clusterlib {

SetLogLevel::SetLogLevel()
        : CliCommand("setLogLevel", NULL)
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::IntegerArg);
    setArgTypeVec(argTypeVec);
}

void
SetLogLevel::action()
{
    int32_t level = getIntArg(0);
    
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
    return "Specify a level between 0 - 5.  0 indicates no logging.";    
}

SetLogLevel::~SetLogLevel() {};

RemoveNotifyable::RemoveNotifyable(Client *client) 
    : CliCommand("removeNotifyable", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::BoolArg);
    setArgTypeVec(argTypeVec);
}

void
RemoveNotifyable::action() 
{
    Notifyable *ntp = getNotifyableArg(0);
    if (ntp == NULL) {
        throw InvalidMethodException(
            "RemoveNotifyable: Failed to get notifyable for key " +
            getNativeArg(0));
    }
    
    string key = ntp->getKey();
    string name = ntp->getName();
    bool removeChildren = getBoolArg(1);
    ntp->remove(removeChildren);
    cout << "Removed notifyable with name (" 
         << name << ") and key (" << key << ")" << endl;
}

string
RemoveNotifyable::helpMessage()
{
    return "Remove a notifyable\n"
        "0 (NotifyableArg) - The notifyable to remove.\n"
        "1 (BoolArg) - If true, do remove the notifyable even if it has\n"
        "              children. Otherwise, do not.\n";
}

RemoveNotifyable::~RemoveNotifyable() {}

GetLockBids::GetLockBids(Client *client) 
    : CliCommand("getLockBids", client, 0)
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    setArgTypeVec(argTypeVec);
}

void 
GetLockBids::action()
{
    NotifyableList::const_iterator it;
    Notifyable *ntp = getClient()->getRoot();
    if (getArgCount() > 0) {
        ntp = getNotifyableArg(0);
        if (ntp == NULL) {
            throw InvalidMethodException(
                "GetLockBids: Failed to get notifyable for key " +
                getNativeArg(0));
        }
    }
    bool children = true;
    if (getArgCount() > 1) {
        children = getBoolArg(1);
    }
    
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
    return "Lock bids will be looked for on the NotifyableArg or the "
        "root if no argument is used.";
}

GetLockBids::~GetLockBids() {}

GetChildren::GetChildren(Client *client) 
    : CliCommand("getChildren", client, 0) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    setArgTypeVec(argTypeVec);
}

void 
GetChildren::action() 
{
    NotifyableList::const_iterator it;
    Notifyable *ntp = getClient()->getRoot();
    if (getArgCount() == 1) {
        ntp = getNotifyableArg(0);
        if (ntp == NULL) {
            throw InvalidMethodException(
                "GetChildren: Failed to get notifyable for key " +
                getNativeArg(0));
        }
    }
    
    NotifyableList nl = ntp->getMyChildren();
    CliParams *params = CliParams::getInstance();
    for (it = nl.begin(); it != nl.end(); it++) {
        cout << (*it)->getKey() << endl;
        params->addToKeySet((*it)->getKey());
    }
}

string
GetChildren::helpMessage()
{
    return "Children will be looked for under NotifyableArg (the parent)"
        " or the root if no argument is used.  Optionally, the BoolArg "
        " may be used to specify if lock bids on child nodes are to be "
        "displayed as well.";
}

GetChildren::~GetChildren() {}

GetAttributes::GetAttributes(Client *client) 
    : CliCommand("getAttributes", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    setArgTypeVec(argTypeVec);
}

void
GetAttributes::action() 
{
    NotifyableList::const_iterator it;
    Notifyable *ntp = getNotifyableArg(0);
    if (ntp == NULL) {
        throw InvalidMethodException(
            "GetAttributes: Failed to get notifyable for key " +
            getNativeArg(0));
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
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}

void
SetCurrentState::action() 
{
    Notifyable *ntp = getNotifyableArg(0);
    if (ntp == NULL) {
        throw InvalidMethodException(
            "SetCurrentState: Failed to get notifyable for key " +
            getNativeArg(0));
    }
    
    ntp->cachedCurrentState().set(getStringArg(1), 
                                  JSONCodec::decode(getStringArg(2)));
    ntp->cachedCurrentState().publish();

    cout << "Set current state for notifyable with name= " 
         << ntp->getName() << " key='" << getStringArg(1) << "',value='"
         << getStringArg(2) << "'" << endl;
}

string
SetCurrentState::helpMessage()
{
    return "Set the current state for a notifyable specified by the "
        "NotifyableArg.  The first StringArg is the state key and the second"
        " StringArg is the encoded JSONValue (i.e. \"value\").";
}

SetCurrentState::~SetCurrentState() {}

SetDesiredState::SetDesiredState(Client *client) 
    : CliCommand("setDesiredState", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}

void
SetDesiredState::action() 
{
    Notifyable *ntp = getNotifyableArg(0);
    if (ntp == NULL) {
        throw InvalidMethodException(
            "SetDesiredState: Failed to get notifyable for key " +
            getNativeArg(0));
    }
    
    ntp->cachedDesiredState().set(getStringArg(1), 
                                  JSONCodec::decode(getStringArg(2)));
    ntp->cachedDesiredState().publish();

    cout << "Set desired state for notifyable with name= " 
         << ntp->getName() << " key='" << getStringArg(1) << "',value='"
         << getStringArg(2) << "'" << endl;
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
    : CliCommand("addApplication", client, 0) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
} 

void 
AddApplication::action() 
{
    Root *root = getClient()->getRoot();
    if (getArgCount() == 1) {
        root = dynamic_cast<Root *>(getNotifyableArg(0));
    }
    if (root == NULL) {
        throw Exception("AddApplication failed to get " + getNativeArg(0));
    }
    Application *application = root->getApplication(getStringArg(1), CREATE_IF_NOT_FOUND);
    if (application == NULL) {
        throw InvalidMethodException(
            "AddApplication: Failed to get application " + 
            getStringArg(1));
    }
}

string 
AddApplication::helpMessage()
{
    return "NotifyableArg is the notifyable "
        "where the application may be added. StringArg is the "
        "name of the new application.";
}
 
AddApplication::~AddApplication() {}

AddGroup::AddGroup(Client *client) 
    : CliCommand("addGroup", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}

void
AddGroup::action() 
{
    Group *group = 
        dynamic_cast<Group *>(getNotifyableArg(0));
    if (group == NULL) {
        throw Exception("AddGroup failed to get " + getNativeArg(0));
    }
    group = group->getGroup(getStringArg(1), CREATE_IF_NOT_FOUND);
    if (group == NULL) {
        throw InvalidMethodException("AddGroup: Failed to get group " + 
                                     getStringArg(1));
    }
}
 
string
AddGroup::helpMessage()
{
    return "NotifyableArg is the notifyable "
        "where the group may be added. StringArg is the "
        "name of the new group.";
}

AddGroup::~AddGroup() {}

AddDataDistribution::AddDataDistribution(Client *client) 
    : CliCommand("addDataDistribution", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
} 

void
AddDataDistribution::action() 
{
    Group *group = 
        dynamic_cast<Group *>(getNotifyableArg(0));
    if (group == NULL) {
        throw Exception("AddDataDistribution failed to get " + 
                        getNativeArg(0));
    }
    DataDistribution *dataDistribution = 
        group->getDataDistribution(getStringArg(1), CREATE_IF_NOT_FOUND);
    if (dataDistribution == NULL) {
        throw InvalidMethodException(
            "AddDataDistribution: Failed to get data distribution " + 
            getStringArg(1));
    }
}
 
string
AddDataDistribution::helpMessage()
{
    return "NotifyableArg is the notifyable "
        "where the data distribution may be added. StringArg is the "
        "name of the new data distribution.";
}
 
AddDataDistribution::~AddDataDistribution() {}

AddNode::AddNode(Client *client) 
    : CliCommand("addNode", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
} 

void
AddNode::action() 
{
    Group *group = 
        dynamic_cast<Group *>(getNotifyableArg(0));
    if (group == NULL) {
        throw Exception("AddNode failed to get " + getNativeArg(0));
    }
    Node *node = group->getNode(getStringArg(1), CREATE_IF_NOT_FOUND);
    if (node == NULL) {
        throw InvalidMethodException("AddNode: Failed to get node " + 
                                     getStringArg(1));
    }
}
 
string
AddNode::helpMessage()
{
    return "NotifyableArg is the notifyable "
        "where the node may be added. StringArg is the "
        "name of the new node.";
}
 
AddNode::~AddNode() {}

AddPropertyList::AddPropertyList(Client *client) 
    : CliCommand("addPropertyList", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}
 
void
AddPropertyList::action() 
{
    Notifyable *ntp = getNotifyableArg(0);
    if (ntp == NULL) {
        throw Exception("AddPropertyList failed to get " + 
                        getNativeArg(0));
    }
    PropertyList *propertyList = 
        ntp->getPropertyList(getStringArg(1), CREATE_IF_NOT_FOUND);
    if (propertyList == NULL) {
        throw InvalidMethodException(
            "AddPropertyList: Failed to get property list " + 
            getStringArg(1));
    }
}

string
AddPropertyList::helpMessage()
{
    return "NotifyableArg is the notifyable "
        "where the property list may be added. StringArg is the "
        "name of the new property list.";
}

AddPropertyList::~AddPropertyList() {}

AddQueue::AddQueue(Client *client) 
    : CliCommand("addQueue", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
} 

void
AddQueue::action() 
{
    Notifyable *ntp = getNotifyableArg(0);
    if (ntp == NULL) {
        throw Exception("AddQueue failed to get " + 
                        getNativeArg(0));
    }
    Queue *queue = 
        ntp->getQueue(getStringArg(1), CREATE_IF_NOT_FOUND);
    if (queue == NULL) {
        throw InvalidMethodException(
            "AddQueue: Failed to get queue " + 
            getStringArg(1));
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
    : CliCommand("getZnode", NULL, 0), m_factory(factory) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}

void
GetZnode::action() 
{
    string zkNodePath = getStringArg(0);
    if (zkNodePath.empty()) {
        throw Exception("action: Failed to get " + getNativeArg(0));
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
    return "Specify a znode (StringArg) to look up its value";
}

GetZnode::~GetZnode() {}

GetZnodeChildren::GetZnodeChildren(Factory *factory, Client *client)
    : CliCommand("getZnodeChildren", NULL, 0), m_factory(factory) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}

void
GetZnodeChildren::action() 
{
    string zkNodePath = getStringArg(0);
    if (zkNodePath.empty()) {
        throw Exception("action: Failed to get " + getNativeArg(0));
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
    return "Specify a znode (StringArg) to look up its children";
}

Help::Help(CliParams *cliParams) 
    : CliCommand("?", NULL, 0),
      m_params(cliParams)
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
} 

void 
Help::action() 
{
    if (getArgCount() == 0) {
        m_params->printCommandNamesByGroup();
    }
    else {
        /* Find the command and print out the options for it */
        CliCommand *command = m_params->getCommandByName(getStringArg(0));
        if (command == NULL) {
            cout << "Unknown commmand or arg '" << getStringArg(0) 
                 << "'" << endl;
        }
        else {
            cout << "Usage: " << command->getCommandName() << " "
                 << command->getArgString() << endl;
            string helpMessage = command->helpMessage();
            if (!helpMessage.empty()) {
                cout << helpMessage << endl;;
            }
        }
    }
}

string
Help::helpMessage() 
{
    return "Specify a command (i.e. quit) or a parameter type "
        "(i.e. StringArg).";
}
 
Help::~Help() {}

AddAlias::AddAlias(CliParams *cliParams) 
    : CliCommand("addAlias", NULL),
      m_params(cliParams)
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::StringArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
} 

void 
AddAlias::action() 
{
    m_params->addAlias(getStringArg(0), getStringArg(1));
}

string
AddAlias::helpMessage() 
{
    return "First StringArg is the new alias, that maps to the second "
        "StringArg.  For example, 'root' --> '/_clusterlib/1.0/root' ";
}
 
AddAlias::~AddAlias() {}

RemoveAlias::RemoveAlias(CliParams *cliParams) 
    : CliCommand("removeAlias", NULL),
      m_params(cliParams)
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}

void 
RemoveAlias::action() 
{
    m_params->removeAlias(getStringArg(0));
}

string
RemoveAlias::helpMessage() 
{
    return "First StringArg is the alias to remove.";
}
 
RemoveAlias::~RemoveAlias() {}

GetAliasReplacement::GetAliasReplacement(CliParams *cliParams) 
    : CliCommand("getAliasReplacement", NULL),
      m_params(cliParams)
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
}

void 
GetAliasReplacement::action() 
{
    cout << "AliasReplacement: " 
         << m_params->getAliasReplacement(getStringArg(0));
}

string
GetAliasReplacement::helpMessage() 
{
    return "First StringArg is the alias to find.";
}
 
GetAliasReplacement::~GetAliasReplacement() {}

JSONRPCCommand::JSONRPCCommand(Client *client, Queue *respQueue) 
    : CliCommand("jsonRpc", client),
      m_respQueue(respQueue)
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::StringArg);
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec); 
} 

void
JSONRPCCommand::action() 
{
    Queue *queue = dynamic_cast<Queue *>(getNotifyableArg(0));
    if (queue == NULL) {
        throw Exception("JSONRPCCommand failed to get the queue " +
                        getNativeArg(0));
    }

    JSONValue::JSONObject respObj;
    JSONValue paramValue = JSONCodec::decode(getNativeArg(2));
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
    string queueKey = getNativeArg(0);
    auto_ptr<json::rpc::JSONRPCRequest> req;
    /* Use the generic request. */
    req.reset(new GenericRequest(getClient(), getNativeArg(1)));
    
    req->setDestination(queueKey.c_str());
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
    return "NotifyableArg is the queue to send the request to.  "
        "StringArg is the method name.  StringArg is the encoded "
        "JSON-RPC parameter array.";
}

JSONRPCCommand::~JSONRPCCommand() {}

StartProcessSlot::StartProcessSlot(Client *client) 
    : CliCommand("startProcessSlot", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::BoolArg);
    setArgTypeVec(argTypeVec);
} 

void
StartProcessSlot::action() 
{
    ProcessSlot *processSlot = 
        dynamic_cast<ProcessSlot * >(getNotifyableArg(0));
    if (processSlot == NULL) {
        throw Exception("StartProcessSlot failed to get " + 
                        getNativeArg(0));
    }

    JSONValue::JSONString startState;
    if (getBoolArg(1) == false) {
        startState = ProcessSlot::PROCESS_STATE_RUN_ONCE_VALUE;
    }
    else {
        startState = ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE;
    }

    processSlot->acquireLock();

    processSlot->cachedDesiredState().set(
        ProcessSlot::PROCESS_STATE_KEY,
        startState);
    processSlot->cachedDesiredState().set(
        ProcessSlot::PROCESS_STATE_SET_MSECS_KEY,
        TimerService::getCurrentTimeMsecs());
    processSlot->cachedDesiredState().publish();

    processSlot->releaseLock();
}

string
StartProcessSlot::helpMessage()
{
    return "Set the desired state to start the process on a ProcessSlot.  "
        "NotifyableArg is the ProcessSlot notifyable.  BoolArg if true sets"
        " the state to ProcessSlot::PROCESS_STATE_RUN_CONTINUOUSLY_VALUE, otherwise"
        " sets the state to ProcessSlot::PROCESS_STATE_RUN_ONCE_VALUE.";
}
 
StartProcessSlot::~StartProcessSlot() {}

StopProcessSlot::StopProcessSlot(Client *client) 
    : CliCommand("stopProcessSlot", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::BoolArg);
    setArgTypeVec(argTypeVec);
} 

void
StopProcessSlot::action() 
{
    ProcessSlot *processSlot = 
        dynamic_cast<ProcessSlot * >(getNotifyableArg(0));
    if (processSlot == NULL) {
        throw Exception("StopProcessSlot failed to get " + 
                        getNativeArg(0));
    }

    processSlot->acquireLock();

    if (getBoolArg(1) == false) {
        processSlot->cachedDesiredState().set(
            ProcessSlot::PROCESS_STATE_KEY,
            ProcessSlot::PROCESS_STATE_EXIT_VALUE);
    }
    else {
        processSlot->cachedDesiredState().set(
            ProcessSlot::PROCESS_STATE_KEY,
            ProcessSlot::PROCESS_STATE_CLEANEXIT_VALUE);
    }
    processSlot->cachedDesiredState().set(
        ProcessSlot::PROCESS_STATE_SET_MSECS_KEY,
        TimerService::getCurrentTimeMsecs());
    processSlot->cachedDesiredState().publish();

    processSlot->releaseLock();
}

string
StopProcessSlot::helpMessage()
{
    return "Set the desired state to stop the process on a ProcessSlot.\n"
        "0 (NotifyableArg) - The ProcessSlot notifyable.\n"
        "1 (BoolArg) - If true, do a clean exit (application implemented),\n"
        "              otherwise, kill the process.\n";
}
 
StopProcessSlot::~StopProcessSlot() {}

StopActiveNode::StopActiveNode(Client *client) 
    : CliCommand("stopActiveNode", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
    argTypeVec.push_back(CliCommand::BoolArg);
    setArgTypeVec(argTypeVec);
} 

void
StopActiveNode::action() 
{
    Node *node = dynamic_cast<Node * >(getNotifyableArg(0));
    if (node == NULL) {
        throw Exception("StopActiveNode failed to get " + 
                        getNativeArg(0));
    }
    node->acquireLock();

    bool shutdown = true;
    if (getArgCount() > 1) {
        shutdown = getBoolArg(1);
    }
    node->cachedDesiredState().set(
        Node::ACTIVENODE_SHUTDOWN, shutdown);
    node->cachedDesiredState().publish();

    node->releaseLock();
}

string
StopActiveNode::helpMessage()
{
    return "Shutdown an ActiveNode.  NotifyableArg is the Node notifyable. "
        " BoolArg is optional, if not set, defaults to true";
}
 
StopActiveNode::~StopActiveNode() {}

AggZookeeperState::AggZookeeperState()
    : CliCommand("aggZookeeperState", NULL) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::StringArg);
    setArgTypeVec(argTypeVec);
} 

void
AggZookeeperState::action() 
{
    ZookeeperPeriodicCheck singleCheck(0, getStringArg(0), NULL);
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
    return "Get the aggregate zookeeper instance state.\n"
        "0 (StringArg) - The comma separated list of servers and ports.\n"
        "                For example 'wmdev1003:2221,wmdev1004:2221'.\n";
}
 
AggZookeeperState::~AggZookeeperState() {}


Quit::Quit(CliParams *cliParams) 
    : CliCommand("quit", NULL),
      m_params(cliParams) { } 

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

JsonArg::JsonArg(Client *client) 
    : CliCommand("jsonArg", client) {}

void
JsonArg::action() 
{
    cout << helpMessage() << endl;
}

string
JsonArg::helpMessage()
{
    return "Valid values: Any key that is a valid json (i.e. \"hi\")";
}
 
JsonArg::~JsonArg() {}

}	/* End of 'namespace clusterlib' */
