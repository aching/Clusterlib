/*
 * clicommand.h --
 *
 * Definition of class CliCommand; represents a cli command that can
 * be run.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CLICOMMAND_H_
#define _CLICOMMAND_H_

#include <vector>
#include <getopt.h>
#include <sstream>

/**
 * A cli command.
 */
class CliCommand {
  public:
    /** 
     * The type of the the arguments (how they will be converted).
     */
    enum ArgType {
        BoolArg = 1,
        IntegerArg,
        StringArg,
        NotifyableArg
    };

    /**
     * Get an ArgType as a string.
     *
     * @param argType the type of CLI argument
     * @return the CLI argument as a string
     */
    static std::string getArgTypeAsString(ArgType argType) 
    {
        switch (argType) {
            case BoolArg:
                return "BoolArg";
            case IntegerArg:
                return "IntegerArg";
            case StringArg:
                return "StringArg";
            case NotifyableArg:
                return "NotifyableArg";
            default:
                return "Unknown";
        }
    }

    /**
     * Get the arguments are a put together string.
     *
     * @return the arguments of a command (space separated)
     */
    std::string getArgString()
    {
        std::string ret;
        std::vector<ArgType>::const_iterator it;
        for (it = m_argTypeVec.begin(); it != m_argTypeVec.end(); it++) {
            ret.append(getArgTypeAsString(*it));
            ret.append(" ");
        }
        if (m_minArgCount != -1) {
            std::stringstream ss;
            ss << "- " << m_minArgCount << " of " 
               << m_argTypeVec.size() << " args required";
            ret.append(ss.str());
        }
        
        return ret;
    }

    /**
     * Set the argument vector for this command.  Check to make sure
     * its a valid size.
     *
     * @param argVec
     */
    void setArgVec(const std::vector<std::string> &argVec)
    {
        m_argVec = argVec;
        checkArgCount();
    }

    /**
     * Get the argument in its native 'string' format.
     *
     * @param argIndex the index of the argument in the vector that
     *        was passed in.
     * @return the 'native' string of the argument
     */
    std::string getNativeArg(int32_t argIndex)
    {
        return m_argVec[argIndex];
    }

    /**
     * Get the argument as a booleen.
     *
     * @param argIndex the index of the argument to convert
     * @return the argument as a boolean
     */
    bool getBoolArg(int32_t argIndex) 
    {
        if (m_argTypeVec[argIndex] != BoolArg) {
            std::stringstream ss;
            ss << argIndex;
            throw clusterlib::InvalidArgumentsException(
                "getBoolArg: Index invalid " + ss.str());
        }
        if (m_argVec[argIndex].compare("true") == 0) {
            return true;
        }

        return false;
    }

    /**
     * Get the argument as a int32_t.
     *
     * @param argIndex the index of the argument to convert
     * @return the argument as a int32_t
     */
    int32_t getIntArg(int32_t argIndex) 
    {
        if (m_argTypeVec[argIndex] != IntegerArg) {
            std::stringstream ss;
            ss << argIndex;
            throw clusterlib::InvalidArgumentsException(
                "getIntArg: Index invalid " + ss.str());
        }
        return ::atoi(m_argVec[argIndex].c_str());
    }

    /**
     * Get the argument as a string.
     *
     * @param argIndex the index of the argument to convert
     * @return the argument as a string
     */
    std::string getStringArg(int32_t argIndex) 
    {
        if (m_argTypeVec[argIndex] != StringArg) {
            std::stringstream ss;
            ss << argIndex;
            throw clusterlib::InvalidArgumentsException(
                "getStringArg: Index invalid " + ss.str());
        }

        return m_argVec[argIndex];
    }

    /**
     * Get the argument as a Notifyable.
     *
     * @param argIndex the index of the argument to convert
     * @return the argument as a Notifyable * or NULL if cannot be found.
     */
    clusterlib::Notifyable *getNotifyableArg(int32_t argIndex) 
    {
        if (m_argTypeVec[argIndex] != NotifyableArg) {
            std::stringstream ss;
            ss << argIndex;
            throw clusterlib::InvalidArgumentsException(
                "getNotifyableArg: Index invalid " + ss.str());
        }

        return getClient()->getRoot()->getNotifyableFromKey(
            m_argVec[argIndex]);
    }

    /**
     * Set up the argument type vector.
     * 
     * @param argTypeVec the vector of argument types
     */
    void setArgTypeVec(std::vector<ArgType> argTypeVec)
    {
        m_argTypeVec = argTypeVec;
    }

    /**
     * Constructor
     */
    CliCommand(std::string command, 
               clusterlib::Client *client, 
               int32_t minArgCount = -1) 
        : m_command(command),
          m_client(client),
          m_minArgCount(minArgCount) {}

    /**
     * Get the command name
     *
     * @return the command name
     */
    std::string getCommandName() const { return m_command; }

    /**
     * Get the clusterlib client
     * 
     * @return the clusterlib client pointer
     */
    clusterlib::Client *getClient() { return m_client; }

    /**
     * Subclasses commands must define the action to take.
     */
    virtual void action() = 0;

    /**
     * Derived command can define a help message.
     *
     * @return the message to print when asked about usage.
     */
    virtual std::string helpMessage()
    {
        return std::string();
    }

    /**
     * Virtual destructor.
     */
    virtual ~CliCommand() {}

    /**
     * Get the argument count
     *
     * @return the size of the argument vector
     */
    size_t getArgCount() 
    {
        return m_argVec.size();
    }
    
  private:
    /**
     * Make sure the argument count is valid.
     */
    void checkArgCount() 
    {
        if (m_minArgCount == -1) {
            if (m_argVec.size() != m_argTypeVec.size()) {
                std::stringstream ss;
                ss << "checkArgCount: Expected size = ";
                ss << m_argTypeVec.size();
                ss << " and actual size = ";
                ss << m_argVec.size();
                throw clusterlib::InconsistentInternalStateException(ss.str());
            }
        }
        else {
            if ((m_argVec.size() > m_argTypeVec.size()) ||
                (static_cast<int32_t>(m_argVec.size()) 
                 < m_minArgCount)) {
                std::stringstream ss;
                ss << "checkArgCount: Expected size >= ";
                ss << m_minArgCount;
                ss << " and <= ";
                ss << m_argTypeVec.size();
                ss << " and actual size = ";
                ss << m_argVec.size();
                throw clusterlib::InconsistentInternalStateException(ss.str());
            }
        }
    }

  private:
    /**
     * Name of the command
     */
    std::string m_command;

    /** 
     * The client of the clusterlib instance 
     */
    clusterlib::Client *m_client;

    /**
     * Vector of arguments types
     */
    std::vector<ArgType> m_argTypeVec;

    /**
     * Vector of actual arguments
     */
    std::vector<std::string> m_argVec;

    /**
     * Minimum number of arguments allowed
     */
    int32_t m_minArgCount;
};

#endif	/* !_CLIPARAMS_H__H_ */
