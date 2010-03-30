/*
 * cliparams.h --
 *
 * Definition of class CliParams; it manages the parameters of the cli
 * process.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CLIPARAMS_H_
#define _CL_CLIPARAMS_H_

#include <iostream>
#include <getopt.h>
#include <ctype.h>
#include "clicommand.h"

/**
 * CLI test parameters.  This is a singleton class per the Gamma
 * method.
 */
class CliParams {
  public:
    static CliParams *getInstance() 
    {
        if (m_params == NULL) {
            m_params = new CliParams();
        }
        return m_params;
    }
    
    /**
     * Print the usage
     *
     * @param exec the executable name
     */
    void printUsage(char *exec) const;

    /**
     * Parse the arguments.
     *
     * @param argc the number of arguments
     * @param argv the vector of arguments
     */
    void parseArgs(int argc, char **argv);
    
    /**
     * Get the zookeeper server port list as a comma separated string.
     *
     * @return the string of zookeeper servers (comma delimited)
     */
    const std::string &getZkServerPortList() const 
    { 
        return m_zkServerPortList; 
    }

    /**
     * Register a command.
     *
     * @param the command to register (should be allocated with new)
     */
    void registerCommand(CliCommand *command) 
    {
        std::pair<std::map<std::string, CliCommand *>::iterator, bool> ret = 
            m_commandMap.insert(
                std::pair<std::string, CliCommand *>(
                    command->getCommandName(), command));
        if (ret.second == false) {
            throw clusterlib::InvalidArgumentsException(
                "registerCommand: Command " + command->getCommandName() + 
                " already exists!");
        }
    }

    /**
     * Print the command names.
     */
    void printCommandNames() 
    {
        printf("Commands:\n");
        std::vector<std::string> commandVec;
        std::map<std::string, CliCommand *>::const_iterator it;
        for (it = m_commandMap.begin(); it != m_commandMap.end(); it++) {
            /* 
             * Distinguish command from arg based on capitalized first
             * letter. 
             */
            if (!it->first.empty() && !isupper(*(it->first.c_str()))) {
                printf(" %s\n", it->first.c_str());
            }
        }
    }

    /**
     * Print the command argument names.
     */
    void printArgNames() 
    {
        printf("Arguments:\n");
        std::vector<std::string> commandVec;
        std::map<std::string, CliCommand *>::const_iterator it;
        for (it = m_commandMap.begin(); it != m_commandMap.end(); it++) {
            /* 
             * Distinguish command from arg based on capitalized first
             * letter. 
             */
            if (!it->first.empty() && isupper(*(it->first.c_str()))) {
                printf(" %s\n", it->first.c_str());
            }
        }
    }

    /**
     * Is this program finished?
     *
     * @return true if finished.
     */
    bool finished()
    {
        return m_finished;
    }

    /**
     * Now it is finished.
     */
    void setFinished()
    {
        m_finished = true;
    }

    /**
     * Parse line and execute command.
     */
    void parseAndRunLine();

    /**
     * Get command by name.
     *
     * @param name the name of the command
     * @return the pointer to the command or NULL if cannot be found.
     */
    CliCommand *getCommandByName(const std::string &name);

    /**
     * Get the command map.
     *
     * @return the pointer to the command map.
     */
    std::map<std::string, CliCommand* > *getCommandMap() 
    {
        return &m_commandMap; 
    }

    /**
     * Get the key set.  It has all the possible completions.
     *
     * @return the pointer to the key set.
     */
    std::set<std::string> *getKeySet()
    {
        return &m_keySet;
    }

    /**
     * Safely add the key to the key set if we haven't reached the
     * maximum number of entries.
     *
     * @param key the key to be added
     */
    void addToKeySet(const std::string &key)
    {
        if (m_keySet.size() < m_keySetMaxSize) {
            m_keySet.insert(key);
        }
    }

    /**
     * Remove the key from the key set if possible.
     *
     * @param key the key to be removed
     */
    void removeFromKeySet(const std::string &key)
    {
        m_keySet.erase(key);
    }

    /**
     * Get the clusterlib client.
     * 
     * @return the clusterlib client pointer
     */
    clusterlib::Client *getClient()
    {
        return m_client;
    }

    /**
     * Get the clusterlib factory.
     *
     * @return the clusterlib factory pointer
     */
    clusterlib::Factory *getFactory()
    {
        return m_factory;
    }

    /**
     * Initialize the factory and client.
     */
    void initFactoryAndClient();
    
    /**
     * Destructor.
     */
    ~CliParams() 
    {
        if (m_factory) {
            delete m_factory;
            m_factory = NULL;
        }
    }

    /**
     * Get the initial log level desired.
     */
    int32_t getLogLevel()
    {
        return m_logLevel;
    }

  private:
    /**
     * Constructor
     */
    CliParams();
    
  private:
    /** 
     * The command separated list of ZooKeeper Servers
     * i.e. (wmdev1008:2181,wmdev1007:2181)
     */
    std::string m_zkServerPortList;

    /**
     * Factory pointer
     */
    clusterlib::Factory *m_factory;

    /**
     * Client pointer
     */
    clusterlib::Client *m_client;

    /**
     * Registered commands
     */
    std::map<std::string, CliCommand *> m_commandMap;

    /**
     * Time to exit?
     */
    bool m_finished;

    /**
     * Run this command line command.
     */
    std::string m_command;

    /**
     * Readline character pointer
     */
    char *m_line;

    /**
     * Cache of notifyable keys
     */
    std::set<std::string> m_keySet;

    /**
     * Maximum m_keySet size.
     */
    int64_t m_keySetMaxSize;

    /**
     * The debug level
     */
    int32_t m_logLevel;

    /**
     * Print the command names then exit.
     */
    bool m_listCommands;

    /** Single instance */
    static CliParams *m_params;
};

#endif	/* !_CL_CLIPARAMS_H_ */
