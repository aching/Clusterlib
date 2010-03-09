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
        : CliCommand("setloglevel", NULL)
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
    : CliCommand("removenotifyable", client) 
{
    vector<CliCommand::ArgType> argTypeVec;
    argTypeVec.push_back(CliCommand::NotifyableArg);
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
        ntp->remove();
        cout << "Removed notifyable with name (" 
             << name << ") and key (" << key << ")" << endl;
}

string
RemoveNotifyable::helpMessage()
{
    return "Remove the notifyable specified by the NotifyableArg";
}

RemoveNotifyable::~RemoveNotifyable() {}

GetLockBids::GetLockBids(Client *client) 
    : CliCommand("getlockbids", client, 0)
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
    : CliCommand("getchildren", client, 0) 
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
    : CliCommand("getattributes", client) 
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
        int32_t healthyNodes = 0, unhealthyNodes = 0;
        names = group->getNodeNames();
        for (NameList::const_iterator iter = names.begin(); 
             iter != names.end(); 
             ++iter) {
            if (group->getNode(*iter)->isHealthy()) {
                healthyNodes++;
            }
            else {
                unhealthyNodes++;
            }
        }
        CliFormat::attributeOut("healthyNodes", healthyNodes);
        CliFormat::attributeOut("unhealthyNodes", unhealthyNodes);
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
        CliFormat::attributeOut("covered", dataDistribution->isCovered());
    }
    else if (dynamic_cast<Node *>(ntp) != NULL) {
        CliFormat::attributeOut("type", "Node");
        Node *node = dynamic_cast<Node *>(ntp);
        int64_t connectionTime = -1, clientStateTime = -1;
        string clientState, clientStateDesc;
        node->getClientState(&clientStateTime, &clientState, &clientStateDesc);
        CliFormat::attributeOut("client state time", clientStateTime);
        CliFormat::attributeOut("client state", clientState);
        CliFormat::attributeOut("client state desc", clientStateDesc);
        CliFormat::attributeOut("connected", 
                                node->isConnected(NULL, &connectionTime));
        CliFormat::attributeOut("connection time", connectionTime);
        CliFormat::attributeOut("healthy", node->isHealthy());
        CliFormat::attributeOut("use process slot", 
                                node->getUseProcessSlots());
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
            "port vec", 
            json::JSONCodec::encode(processSlot->getJsonPortVec()));
        CliFormat::attributeOut(
            "exec args", 
            json::JSONCodec::encode(processSlot->getJsonExecArgs()));
        CliFormat::attributeOut(
            "running exec args", 
            json::JSONCodec::encode(
                    processSlot->getJsonRunningExecArgs()));
        CliFormat::attributeOut("PID", processSlot->getPID());
        CliFormat::attributeOut(
            "desired process state", 
            ProcessSlot::getProcessStateAsString(
                processSlot->getDesiredProcessState()));
        CliFormat::attributeOut(
            "current process state", 
            ProcessSlot::getProcessStateAsString(
                processSlot->getCurrentProcessState()));
    }
    else if (dynamic_cast<PropertyList *>(ntp) != NULL) {
        CliFormat::attributeOut("type", "Property List");
        PropertyList *propertyList = 
            dynamic_cast<PropertyList *>(ntp);
        vector<string> keyVec = propertyList->getPropertyListKeys();
        for (vector<string>::iterator it = keyVec.begin();
             it != keyVec.end();
             it++) {
            CliFormat::attributeOut(*it, propertyList->getProperty(*it));
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
}

string
GetAttributes::helpMessage()
{
    return "Get more information about a notifyable.";
}


GetAttributes::~GetAttributes() {}

AddApplication::AddApplication(Client *client) 
    : CliCommand("addapplication", client, 0) 
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
    Application *application = root->getApplication(getStringArg(1), true);
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
    : CliCommand("addgroup", client) 
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
    group = group->getGroup(getStringArg(1), true);
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
    : CliCommand("adddatadistribution", client) 
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
        group->getDataDistribution(getStringArg(1), true);
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
    : CliCommand("addnode", client) 
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
    Node *node = group->getNode(getStringArg(1), true);
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
    : CliCommand("addpropertylist", client) 
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
        ntp->getPropertyList(getStringArg(1), true);
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
    : CliCommand("addqueue", client) 
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
        ntp->getQueue(getStringArg(1), true);
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
        m_params->printCommandNames();
        cout << endl;
        m_params->printArgNames();
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

JSONRPCCommand::JSONRPCCommand(Client *client, Queue *respQueue) 
    : CliCommand("jsonrpc", client),
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
    try {
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
        if (!getNativeArg(1).compare(
                ClusterlibStrings::RPC_START_PROCESS)) {
            req.reset(new StartProcessRequest(getClient()));
        }
        else if (!getNativeArg(1).compare(
                     ClusterlibStrings::RPC_STOP_PROCESS)) {
            req.reset(new StopProcessRequest(getClient()));
        }
        else {
            /* Try the generic request if the method is not recognized. */
            req.reset(new GenericRequest(getClient(), getNativeArg(1)));
        }
        
        req->prepareRequest(paramArr);
        req->sendRequest(queueKey.c_str());
        req->waitResponse();
        respObj = req->getResponse();
        cout << "response: " << JSONCodec::encode(respObj);
    }
    catch (const JSONRPCInvocationException &ex) {
        throw Exception("JSONRPCCommand failed to parse your JSON-RPC "
                        "request " + getNativeArg(1) + 
                        string(" with params ") + getNativeArg(2) + ": " +
                        ex.what());
    }
}

string
JSONRPCCommand::helpMessage()
{
    return "NotifyableArg is the queue to send the request to.  "
        "StringArg is the method name.  StringArg is the encoded "
        "JSON-RPC parameter array.";
}

JSONRPCCommand::~JSONRPCCommand() {}

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
    : CliCommand("BoolArg", NULL) { } 

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
    : CliCommand("IntegerArg", NULL) {}

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
    : CliCommand("StringArg", NULL) {}

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
    : CliCommand("NotifyableArg", client) {}

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

};	/* End of 'namespace clusterlib' */
