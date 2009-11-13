/*
 * cli.cc --
 *
 * Implementation of the command line interface
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

using namespace std;
using namespace clusterlib;

class SetLogLevel : public CliCommand
{
  public:
    SetLogLevel()
        : CliCommand("setloglevel", NULL)
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::IntegerArg);
        setArgTypeVec(argTypeVec);
    }
    virtual void action()
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
    virtual string helpMessage()
    {
        return "Specify a level between 0 - 5.  0 indicates no logging.";
        
    }
    virtual ~SetLogLevel() {};
};

/**
 * Remove a notifyable
 */
class RemoveNotifyable : public CliCommand
{
  public:
    RemoveNotifyable(Client *client) 
        : CliCommand("removenotifyable", client) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        setArgTypeVec(argTypeVec);
    }
    virtual void action() 
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
    virtual ~RemoveNotifyable() {}
};

/**
 * Get the children of a notifyable
 */
class GetChildren : public CliCommand
{
  public:
    GetChildren(Client *client) 
        : CliCommand("getchildren", client, 0) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        setArgTypeVec(argTypeVec);
    }
    virtual void action() 
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
    virtual string helpMessage()
    {
        return "Children will be looked for under NotifyableArg (the parent)"
            " or the root if no argument is used.";
    }
    virtual ~GetChildren() {}
};

/**
 * Get the attributes of a notifyable
 */
class GetAttributes : public CliCommand
{
  public:
    GetAttributes(Client *client) 
        : CliCommand("getattributes", client) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        setArgTypeVec(argTypeVec);
    }
    virtual void action() 
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
            CliFormat::attributeOut("client state", node->getClientState());
            CliFormat::attributeOut("connected", node->isConnected());
            CliFormat::attributeOut("connection time", 
                                    node->getConnectionTime());
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
    virtual string helpMessage()
    {
        return "Get more information about a notifyable.";
    }
    virtual ~GetAttributes() {}
};

/**
 * Add an application to the clusterlib hierarchy
 */
class AddApplication : public CliCommand
{
  public:
    AddApplication(Client *client) 
        : CliCommand("addapplication", client, 0) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        argTypeVec.push_back(CliCommand::StringArg);
        setArgTypeVec(argTypeVec);
    } 
    virtual void action() 
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
    virtual string helpMessage()
    {
        return "NotifyableArg is the notifyable "
            "where the application may be added. StringArg is the "
            "name of the new application.";
    }
    virtual ~AddApplication() {}
};

/**
 * Add a group to the clusterlib hierarchy
 */
class AddGroup : public CliCommand
{
  public:
    AddGroup(Client *client) 
        : CliCommand("addgroup", client) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        argTypeVec.push_back(CliCommand::StringArg);
        setArgTypeVec(argTypeVec);
    } 
    virtual void action() 
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
    virtual string helpMessage()
    {
        return "NotifyableArg is the notifyable "
            "where the group may be added. StringArg is the "
            "name of the new group.";
    }
    virtual ~AddGroup() {}
};

/**
 * Add a data distribution to the clusterlib hierarchy
 */
class AddDataDistribution : public CliCommand
{
  public:
    AddDataDistribution(Client *client) 
        : CliCommand("adddatadistribution", client) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        argTypeVec.push_back(CliCommand::StringArg);
        setArgTypeVec(argTypeVec);
    } 
    virtual void action() 
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
    virtual string helpMessage()
    {
        return "NotifyableArg is the notifyable "
            "where the data distribution may be added. StringArg is the "
            "name of the new data distribution.";
    }
    virtual ~AddDataDistribution() {}
};

/**
 * Add a node to the clusterlib hierarchy
 */
class AddNode : public CliCommand
{
  public:
    AddNode(Client *client) 
        : CliCommand("addnode", client) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        argTypeVec.push_back(CliCommand::StringArg);
        setArgTypeVec(argTypeVec);
    } 
    virtual void action() 
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
    virtual string helpMessage()
    {
        return "NotifyableArg is the notifyable "
            "where the node may be added. StringArg is the "
            "name of the new node.";
    }
    virtual ~AddNode() {}
};

/**
 * Add a property list to the clusterlib hierarchy
 */
class AddPropertyList : public CliCommand
{
  public:
    AddPropertyList(Client *client) 
        : CliCommand("addpropertylist", client) 
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::NotifyableArg);
        argTypeVec.push_back(CliCommand::StringArg);
        setArgTypeVec(argTypeVec);
    } 
    virtual void action() 
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
    virtual string helpMessage()
    {
        return "NotifyableArg is the notifyable "
            "where the property list may be added. StringArg is the "
            "name of the new property list.";
    }
    virtual ~AddPropertyList() {}
};

/**
 * Get commands and arguments.
 */
class Help : public CliCommand
{
  public:
    Help(CliParams *cliParams) 
        : CliCommand("?", NULL, 0),
          m_params(cliParams)
    {
        vector<CliCommand::ArgType> argTypeVec;
        argTypeVec.push_back(CliCommand::StringArg);
        setArgTypeVec(argTypeVec);
    } 
    virtual void action() 
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
    virtual string helpMessage() 
    {
        return "Specify a command (i.e. quit) or a parameter type "
            "(i.e. StringArg).";
    }
    virtual ~Help() {}

  private:
    CliParams *m_params;
};

/**
 * Quit
 */
class Quit : public CliCommand
{
  public:
    Quit(CliParams *cliParams) 
        : CliCommand("quit", NULL),
          m_params(cliParams) { } 
    virtual void action() 
    {
        m_params->setFinished();
        cout << "Exiting.." << endl;
    }
    virtual string helpMessage()
    {
        return "Exit the shell.";
    }
    virtual ~Quit() {}

  private:
    CliParams *m_params;
};

/**
 * BoolArg
 */
class BoolArg : public CliCommand
{
  public:
    BoolArg(): CliCommand("BoolArg", NULL) { } 
    virtual void action() 
    {
        cout << helpMessage() << endl;
    }
    virtual string helpMessage()
    {
        return "Valid values: 'true' or 'false'";
    }
    virtual ~BoolArg() {}
};

/**
 * IntegerArg
 */
class IntegerArg : public CliCommand
{
  public:
    IntegerArg() : CliCommand("IntegerArg", NULL) {}
    virtual void action() 
    {
        cout << helpMessage() << endl;
    }
    virtual string helpMessage()
    {
        return "Valid values: Anything that can be parsed by atoi " 
            "(i.e. -1 or 1234)";
    }
    virtual ~IntegerArg() {}
};

/**
 * StringArg
 */
class StringArg : public CliCommand
{
  public:
    StringArg() : CliCommand("StringArg", NULL) {}

    virtual void action() 
    {
        cout << helpMessage() << endl;
    }
    virtual string helpMessage()
    {
        return "Valid values: Anything normal printable string "
            "characters (no spaces)).";
    }
    virtual ~StringArg() {}
};

/**
 * NotifyableArg
 */
class NotifyableArg : public CliCommand
{
  public:
    NotifyableArg(Client *client) 
        : CliCommand("NotifyableArg", client) {}

    virtual void action() 
    {
        cout << helpMessage() << endl;
    }
    virtual string helpMessage()
    {
        return "Valid values: Any key that is a valid notifyable "
            "(i.e. " + getClient()->getRoot()->getKey() 
            + ")";
    }
    virtual ~NotifyableArg() {}
};

/**
 * The command line interface may be used in a shell-like environment
 * or to execute a stand alone command.
 */
CliParams *CliParams::m_params = NULL;
int main(int argc, char* argv[]) 
{
    CliParams *params = CliParams::getInstance();

    /* Parse the arguments */
    params->parseArgs(argc, argv);
    
    /* 
     * Force the log level to be set to 0, special case for a command.
     */
    SetLogLevel *setLogLevelCommand = new SetLogLevel;
    vector<string> logLevelArgVec;
    logLevelArgVec.push_back("0");
    setLogLevelCommand->setArgVec(vector<string>(logLevelArgVec));
    setLogLevelCommand->action();
                                  
    /* Register the commands after connecting */
    params->registerCommand(setLogLevelCommand);
    params->registerCommand(new RemoveNotifyable(params->getClient()));
    params->registerCommand(new GetChildren(params->getClient()));
    params->registerCommand(new GetAttributes(params->getClient()));
    params->registerCommand(new AddApplication(params->getClient()));
    params->registerCommand(new AddGroup(params->getClient()));
    params->registerCommand(new AddDataDistribution(params->getClient()));
    params->registerCommand(new AddNode(params->getClient()));
    params->registerCommand(new AddPropertyList(params->getClient()));
    params->registerCommand(new Help(params));
    params->registerCommand(new Quit(params));
    
    /* Register the arguments */
    params->registerCommand(new BoolArg());
    params->registerCommand(new IntegerArg());
    params->registerCommand(new StringArg());
    params->registerCommand(new NotifyableArg(params->getClient()));

    /* Keep getting commands until done. */
    while (!params->finished()) {
        params->parseAndRunLine();
    }
    
    /* Clean up */
    map<string, CliCommand *>::iterator it;
    for (it = params->getCommandMap()->begin(); 
         it != params->getCommandMap()->end();
         it++) {
        delete it->second;
    }

    return 0;
}
