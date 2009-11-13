#include "clusterlibinternal.h"
#include "cliparams.h"
#include <stdlib.h> 
#include <readline/readline.h>
#include <readline/history.h>
#include <curses.h>
#include "cliformat.h"

using namespace std;
using namespace boost;
using namespace clusterlib;

/** 
 * Used by readline.  Try to complete based on previously seen keys or
 * all the different command names.
 */
static char *
commandCompletion(const char *text, int iteration)
{
    CliParams *params = CliParams::getInstance();

    if (text && (text[0] == '/')) {
        static set<string>::iterator it;
        static bool atLeastOneResult = false;
        if (iteration == 0) {
            it = params->getKeySet()->begin();
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
                         chopIt++) {
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
        }

        for (; it != params->getKeySet()->end(); it++) {
            if (it->compare(0, strlen(text), text) == 0) {
                set<string>::iterator returnIt = it;
                it++;
                atLeastOneResult = true;
                return strdup(returnIt->c_str());
            }
        }
    }
    else {
        if (params->getCommandMap()->size() == 0) {
            return NULL;
        }
        
        static map<string, CliCommand *>::iterator it;
        if (iteration == 0) {
            it = params->getCommandMap()->begin();
        }
        
        for (; it != params->getCommandMap()->end(); it++) {
            if (it->first.compare(0, strlen(text), text) == 0) {
                map<string, CliCommand *>::iterator returnIt = it;
                it++;
                return strdup(returnIt->first.c_str());
            }
        }
    }

    return NULL;
}

CliParams::CliParams() 
    : m_factory(NULL),
      m_client(NULL),
      m_finished(false), 
      m_line(NULL),
      m_keySetMaxSize(1024*1024)

{
    /* 
     * Set readline to use commandCompletion instead of the
     * default. 
     */
    rl_completion_entry_function = commandCompletion;
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
    const char *optstring = ":hz:l:c:";

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
            case 'l':
                printCommandNames();
                exit(0);
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
    m_factory = new Factory(getZkServerPortList());
    m_client = m_factory->createClient();

    /* Add the root key to the key set. */
    addToKeySet(m_client->getRoot()->getKey());
}

/** The key separator arguments. */
static const string keySeparator = " ";

void
CliParams::parseAndRunLine()
{        
    cout << endl;
    vector<string> components;
    if (m_command.empty()) {
        m_line = readline("Enter command (Use '?' if help is required):\n");
        /* If the line has text, save it to history. */
        if (m_line && *m_line) {
            add_history(m_line);
        }
        split(components, m_line, is_any_of(keySeparator));
        /* Clean up. */
        if (m_line) {
            free(m_line);
            m_line = NULL;
        }        
    }
    else {
        split(components, m_command, is_any_of(keySeparator));
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

CliCommand *
CliParams::getCommandByName(const string &name)
{
    map<string, CliCommand *>::iterator it = 
        m_commandMap.find(name);
    if (it == m_commandMap.end()) {
        return NULL;
    }
    
    return it->second;
}

