#include "clusterlibinternal.h"
#include "cliparams.h"
#include <stdlib.h> 
#include "cliformat.h"

#ifndef NO_TAB_COMPLETION
#include <readline/readline.h>
#include <readline/history.h>
#include <curses.h>
#endif

using namespace std;
using namespace clusterlib;

#ifndef NO_TAB_COMPLETION
/** 
 * Used by readline.  Try to complete based on previously seen
 * Notifyable keys, children of Notifyable keys, all command names,
 * and all alias replacements.
 */
static char *
commandCompletion(const char *text, int iteration)
{
    CliParams *params = CliParams::getInstance();

    static set<string>::iterator keySetIt;
    static bool atLeastOneResult = false;

    static map<string, map<string, CliCommand *> >::iterator groupIt;
    static map<string, CliCommand *>::iterator commandIt;

    static map<string, string>::iterator aliasReplacementMapIt;

    /* Initialize the static members in the first iteration */
    if (iteration == 0) {
        keySetIt = params->getKeySet()->begin();
        
        groupIt = params->getGroupCommandMap()->begin();
        commandIt = groupIt->second.begin();

        aliasReplacementMapIt = params->getAliasReplacementMap()->begin();
    }

    /* Look for a Notifyable key */
    if (text && (text[0] == '/')) {
        if (atLeastOneResult == false) {
            /* No results last time, let's find some */
            string chopText(text);
            Root *root = params->getClient()->getRoot();
            string rootKey(root->getKey());
            /*
             * Algorithm: 
             * 
             * Cut off a letter each time until an
             * entry is found in the key set.  If a key is found,
             * get the children and continue.  If no notifyable
             * can be retrieved, remove that entry from the set
             * and keep going.
             */
            set<string>::iterator chopIt;
            while (chopText.size() >= rootKey.size()) {
                for (chopIt = params->getKeySet()->begin();
                     chopIt != params->getKeySet()->end();
                     ++chopIt) {
                    if (chopIt->compare(chopText) == 0) {
                        Notifyable *ntp = 
                            root->getNotifyableFromKey(chopText);
                        if (ntp == NULL) {
                            params->removeFromKeySet(chopText);
                        }
                        else {
                            NotifyableList nl = ntp->getMyChildren();
                            NotifyableList::const_iterator nlIt;
                            for (nlIt = nl.begin(); 
                                 nlIt != nl.end(); 
                                 nlIt++) {
                                params->addToKeySet((*nlIt)->getKey());
                            }
                        }
                    }
                }
                chopText.resize(chopText.size() - 1);
            }
        }
        atLeastOneResult = false;

        while (keySetIt != params->getKeySet()->end()) {
            if (keySetIt->compare(0, strlen(text), text) == 0) {
                set<string>::iterator returnIt = keySetIt;
                ++keySetIt;
                atLeastOneResult = true;
                return strdup(returnIt->c_str());
            }
            ++keySetIt;
        }
    }

    /* Look for a matching command */
    while (groupIt != params->getGroupCommandMap()->end()) {
        while (commandIt != groupIt->second.end()) {
            if (commandIt->first.compare(0, strlen(text), text) == 0) {
                map<string, CliCommand *>::iterator returnIt = commandIt;
                ++commandIt;
                if (commandIt == groupIt->second.end()) {
                    ++groupIt;
                    commandIt = groupIt->second.begin();
                }
                return strdup(returnIt->first.c_str());
            }
            ++commandIt;
        }
        ++groupIt;
        commandIt = groupIt->second.begin();
    }
    
    /* Look for a matching aliases */
    while (aliasReplacementMapIt != params->getAliasReplacementMap()->end()) {
        if (aliasReplacementMapIt->first.compare(0, strlen(text), text) == 0) {
            map<string, string>::iterator returnIt = aliasReplacementMapIt;
            ++aliasReplacementMapIt;
            return strdup(returnIt->first.c_str());
        }
        ++aliasReplacementMapIt;
    }

    return NULL;
}
#endif

CliParams::CliParams() 
    : m_factory(NULL),
      m_client(NULL),
      m_finished(false), 
      m_line(NULL),
      m_keySetMaxSize(1024*1024),
      m_logLevel(0),
      m_listCommands(false)
{

#ifndef NO_TAB_COMPLETION
    /* 
     * Set readline to use commandCompletion instead of the
     * default. 
     */
    rl_completion_entry_function = commandCompletion;
#endif
}

void 
CliParams::printUsage(char *exec) const
{
    cout <<
"Usage: " << exec <<
" [OPTION]... [VAR=VALUE]...\n\n"
" -h  --help            Display this help and exit.\n"
" -l  --list_cmds       List all available commands.\n"
" -z  --zk_server_port  Zookeeper server port list \n"
" -d  --debug_level     Set the debug level 0-5 (default 0)\n"
" -c  --command         Run a command.  Spaces delimit arguments\n";
}

void 
CliParams::parseArgs(int argc, char **argv)
{
    static struct option longopts[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"zk_server_port_list", required_argument, NULL, 'z'},
        {"list_cmds", no_argument, NULL, 'l'},
        {"command", required_argument, NULL, 'c'},
        {0,0,0,0}
    };

    /* Index of current long option into opt_lng array */
    int32_t option_index = 0;
    int32_t err = -1;
    const char *optstring = ":hz:d:lc:";

    /* Parse all standard command line arguments */
    while (1) {
        err = getopt_long(argc, argv, optstring, longopts, &option_index);
        if (err == -1) {
            break;
        }
        switch(err) {
            case 'h':
                printUsage(argv[0]);
                exit(-1);
            case 'z':
                m_zkServerPortList = optarg;
                break;
            case 'd':
                m_logLevel = atoi(optarg);
                break;
            case 'l':
                m_listCommands = true;
                break;
            case 'c':
                m_command = optarg;
                break;
            default:
                cout << "Option -" 
                     << static_cast<char>(optopt) 
                     << " is invalid" << endl;
                exit(-1);
        }
    }

    if (m_zkServerPortList.empty()) {
        cout << "Option -z needs to be set" << endl;;
        printUsage(argv[0]);
        ::exit(-1);
    }
}

/** Split the line into tokens and respect 'TOKEN' tokens */
static vector<string> getTokensFromString(string input)
{
    vector<string> resVec;

    size_t index = 0;
    bool quoteToken = false;
    size_t startTokenIndex = string::npos;
    while (index != input.size()) {
        if (input[index] == '\'') {
            if (quoteToken == false) {
                quoteToken = true;
                startTokenIndex = index + 1;
            }
            else {
                resVec.push_back(input.substr(startTokenIndex, 
                                              index - startTokenIndex));
                quoteToken = false;
                startTokenIndex = string::npos;
            }
        }
        else if (input[index] == ' ') {
            if (quoteToken == false) {
                if (startTokenIndex != string::npos) {
                    resVec.push_back(input.substr(startTokenIndex, 
                                                  index - startTokenIndex));
                    startTokenIndex = string::npos;
                }
            }
        }
        else {
            if (startTokenIndex == string::npos) {
                startTokenIndex = index;
            }
        }
        ++index;
    }
    if (quoteToken != false) {
        ostringstream oss;
        oss << "getTokensFromString: Missing ' terminator in input: " << input;
        throw InvalidArgumentsException(oss.str());
    }
    if (startTokenIndex != string::npos) {
        resVec.push_back(input.substr(startTokenIndex, 
                                      index - startTokenIndex));
    }

    return resVec;
}

void
CliParams::parseAndRunLine()
{        
    cout << endl;
    vector<string> components;

    if (m_listCommands) {
        printCommandNamesByGroup();
        setFinished();
        return;
    }

    if (m_command.empty()) {
#ifndef NO_TAB_COMPLETION
        m_line = readline(generateWelcomeMessage().c_str());
        /* If the line has text, save it to history. */
        if (m_line && *m_line) {
            add_history(m_line);
        }
        components = getTokensFromString(m_line);
        /* Clean up. */
        if (m_line) {
            free(m_line);
            m_line = NULL;
        }        
#else
        char lineString[4096];
        cout << generateWelcomeMessage();
        cin.getline(lineString, 4096);
        components = getTokensFromString(lineString);
#endif
    }
    else {

        components = getTokensFromString(m_command);
        setFinished();
    }

    /* Get rid of any empty arguments. */
    bool foundEmpty = true;
    vector<string>::iterator it;
    while (foundEmpty == true) {
        foundEmpty = false;
        for (it = components.begin(); it != components.end(); it++) {
            if (it->empty()) {
                components.erase(it);
                foundEmpty = true;
                break;
            }
        }
    }

    if (components.size() == 0) {
        cout << "Invalid empty line: " << endl;
    }
    else {
        CliCommand *command = getCommandByName(components[0]);
        if (command != NULL) {
            components.erase(components.begin());
            cout << endl;
            
            /* Change all tokens to aliases if not removeTokenAlias command */
            if (command->getCommandName().compare("removeTokenAlias")) {
                map<string, string>::const_iterator aliasReplacementMapIt;
                for (it = components.begin(); it != components.end(); ++it) {
                    aliasReplacementMapIt = m_aliasReplacementMap.find(*it);
                    if (aliasReplacementMapIt != m_aliasReplacementMap.end()) {
                        cout << "Note: Replacing token '"
                             << *it << "' with alias '" 
                             << aliasReplacementMapIt->second << "'" << endl;
                        *it = aliasReplacementMapIt->second;
                    }
                }
            }

            try {
                command->setArgVec(components);
                command->action();
            }
            catch (const clusterlib::Exception &ex) {
                cout << "Command '" << command->getCommandName() << "' failed "
                     << "with error: " << ex.what() << endl;
            }
        }
        else {
            cout << "Command '" << components[0] << "' not found" 
                      << endl;
                    
        }
    }
}

void
CliParams::registerCommandByGroup(CliCommand *command, const string &groupName)
{
    /*
     * Do not allow commands with the same name to be registered even
     * if they are in different groups.
     */
    map<string, map<string, CliCommand *> >::const_iterator groupCommandMapIt;
    map<string, CliCommand *>::const_iterator commandMapIt;
    for (groupCommandMapIt = m_groupCommandMap.begin();
         groupCommandMapIt != m_groupCommandMap.end();
         ++groupCommandMapIt) {
        commandMapIt = groupCommandMapIt->second.find(
            command->getCommandName());
        if (commandMapIt != groupCommandMapIt->second.end()) {
            throw clusterlib::InvalidArgumentsException(
                "registerCommand: Command " + command->getCommandName() + 
                " with group " + groupName + " already exists!");
        }
    }
    
    m_groupCommandMap[groupName][command->getCommandName()] = command;
}

void
CliParams::printCommandNamesByGroup()
{
    map<string, map<string, CliCommand *> >::const_iterator groupCommandMapIt;
    map<string, CliCommand *>::const_iterator commandMapIt;
    for (groupCommandMapIt = m_groupCommandMap.begin();
         groupCommandMapIt != m_groupCommandMap.end();
         ++groupCommandMapIt) {
        cout << groupCommandMapIt->first << ":" << endl;
        for (commandMapIt = groupCommandMapIt->second.begin();
             commandMapIt != groupCommandMapIt->second.end();
             ++commandMapIt) {
            cout << " " << commandMapIt->first << endl;
        }
        cout << endl;
    }
}

CliCommand *
CliParams::getCommandByName(const string &name)
{
    map<string, map<string, CliCommand *> >::const_iterator groupCommandMapIt;
    map<string, CliCommand *>::const_iterator commandMapIt;
    for (groupCommandMapIt = m_groupCommandMap.begin();
         groupCommandMapIt != m_groupCommandMap.end();
         ++groupCommandMapIt) {
        commandMapIt = groupCommandMapIt->second.find(name);
        if (commandMapIt != groupCommandMapIt->second.end()) {
            return commandMapIt->second;
        }
    }
    
    return NULL;
}

void
CliParams::addAlias(const string &token, const string &alias)
{
    pair<map<string, string>::iterator, bool> ret = 
        m_aliasReplacementMap.insert(make_pair<string, string>(token, alias));
    if (ret.second == false) {
        ostringstream oss;
        oss << "addAlias: Alias " << alias << " already exists";
        throw InvalidArgumentsException(oss.str());
    }
}

size_t
CliParams::removeAlias(const string &alias)
{
    return m_aliasReplacementMap.erase(alias);
}

string
CliParams::getAliasReplacement(const string &alias)
{
    map<string, string>::const_iterator aliasReplacementMapIt = 
        m_aliasReplacementMap.find(alias);
    if (aliasReplacementMapIt == m_aliasReplacementMap.end()) {
        ostringstream oss;
        oss << "getAliasReplacement: Alias " << alias << " not found";
        throw InvalidArgumentsException(oss.str());
    }
    
    return aliasReplacementMapIt->second;
}

void
CliParams::initFactoryAndClient()
{
    m_factory = new Factory(getZkServerPortList());
    m_client = m_factory->createClient();

    /* Add the root key to the key set. */
    addToKeySet(m_client->getRoot()->getKey());
}

static size_t MaxAliasSize = 15;

string
CliParams::generateWelcomeMessage()
{
    string res;
    int32_t remainingAliasSize = -1;

    if (!m_aliasReplacementMap.empty()) {
        res.append("Alias listed below:\n");
        map<string, string>::const_iterator aliasReplacementMapIt;
        for (aliasReplacementMapIt = m_aliasReplacementMap.begin();
             aliasReplacementMapIt != m_aliasReplacementMap.end();
             ++aliasReplacementMapIt) {
            res.append("'");
            res.append(aliasReplacementMapIt->first);
            res.append("' ");
            remainingAliasSize = 
                MaxAliasSize - aliasReplacementMapIt->first.size();
            if (remainingAliasSize > 0) {
                res.append(remainingAliasSize, '-');
            }
            res.append("> '");
            res.append(aliasReplacementMapIt->second);
            res.append("'");
            res.append("\n");
        }
    }
    res.append("Enter command (Use '?' if help is required):\n");

    return res;
}
