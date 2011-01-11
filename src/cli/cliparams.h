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

#ifndef	_CL_CLIPARAMS_H_
#define _CL_CLIPARAMS_H_

namespace clusterlib {

/**
 * CLI test parameters.  This is a singleton class per the Gamma
 * method.
 */
class CliParams
{
  public:
    /**
     * Get the static instance (creating if it doesn't exist).
     */
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
     * @param command Command to register (should be allocated with new)
     * @param groupName Name of the group to register the commmand under
     */
    void registerCommandByGroup(CliCommand *command, 
                                const std::string &groupName);

    /**
     * Print the command names by group.
     */
    void printCommandNamesByGroup();

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
     * Get the group command map.
     *
     * @return the pointer to the group command map.
     */
    std::map<std::string, std::map<std::string, CliCommand* > > *
        getGroupCommandMap() 
    {
        return &m_groupCommandMap; 
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
        if (static_cast<int64_t>(m_keySet.size()) < m_keySetMaxSize) {
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
     * Generate the welcome message.  Can be overriden by subclasses.
     *
     * @return The final welcome message.
     */
    virtual std::string generateWelcomeMessage();

    /**
     * Destructor.
     */
    virtual ~CliParams() 
    {
        if (m_factory) {
            delete m_factory;
            m_factory = NULL;
        }
    }

    /**
     * Add an alias.  The alias must not already exist.
     *
     * @param alias The new alias for the replacement.
     * @param replacement The replacement the alias maps to.
     */ 
    void addAlias(const std::string &alias, const std::string &replacement);

    /**
     * Remove an alias.  The alias may/may not exist.
     *
     * @param alias The new alias for the replacement.
     * @return 1 if the alias was removed, 0 if not found.
     */
    size_t removeAlias(const std::string &alias);

    /**
     * Get the alias's replacement.  If the alias doesn't exist an
     * InvalidArgumentsException will be thrown.
     *
     * @param alias The alias that maps to the replacement
     * @return The replacement that the alias mapped to.
     */
    std::string getAliasReplacement(const std::string &alias);

    /**
     * Get the alias replacement map.
     *
     * @return The pointer to the alias replacement map.
     */
    std::map<std::string, std::string> *getAliasReplacementMap()
    {
        return &m_aliasReplacementMap;
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
     * Registered commands organized by group.
     */
    std::map<std::string, std::map<std::string, CliCommand *> > 
        m_groupCommandMap;

    /**
     * Alias to replacement map (i.e. masterNode ->
     * /_clusterlib/1.0/_root/applications/_gqs/_node/masterNode))
     */
    std::map<std::string, std::string> m_aliasReplacementMap;

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

}

#endif	/* !_CL_CLIPARAMS_H_ */
