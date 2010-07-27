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

#ifndef	_CL_CLICOMMAND_H_
#define _CL_CLICOMMAND_H_

#include <vector>
#include <getopt.h>
#include <sstream>

namespace clusterlib
{

/**
 * A cli command.
 */
class CliCommand 
{
  public:
    /** Name of the notifyable argument */
    static const std::string NOTIFYABLE_ARG;

    /** Name of the notifyable name argument */
    static const std::string NOTIFYABLE_NAME_ARG;

    /** Name of the children argument */
    static const std::string CHILDREN_ARG;

    /** Name of the key argument */
    static const std::string KEY_ARG;

    /** Name of the value argument */
    static const std::string VALUE_ARG;

    /** Name of the zookeeper node argument */
    static const std::string ZKNODE_ARG;

    /** Name of the force argument */
    static const std::string FORCE_ARG;

    /** 
     * The type of the the arguments (how they will be converted).
     */
    enum ArgType {
        BoolArg = 1,
        IntegerArg,
        StringArg,
        NotifyableArg,
        JsonArg,
        UnknownArg
    };

    /** Unknown argument help message */
    static const std::string UNKNOWN_ARG_HELP_MSG;

    /**
     * Get an ArgType as a string.
     *
     * @param argType the type of CLI argument
     * @return the CLI argument as a string
     */
    static std::string getArgTypeAsString(ArgType argType);

    /**
     * All the metadata associated with an argument from a command.
     */
    class CliArg {
      public:
        /**
         * Constructor for unnamed arguments.
         */
        CliArg();

        /**
         * Constructor.
         *
         * @param defaultValue The default value of the argument.
         * @param helpMsg How to use this argument.
         * @param argType The argument type
         * @param required If true, this object is required.
         */
        CliArg(const std::string &defaultValue, 
               const std::string &helpMsg,
               ArgType argType, 
               bool required);

        /**
         * Set the value.
         *
         * @param value The new value.
         */
        void setValue(const std::string &value);

        /**
         * Copy the default value to the set value.
         */
        void promoteDefaultValue();

        /**
         * Get the argument as a native string (no type checking).
         *
         * @return The native argument.
         */
        const std::string &getNativeArg() const; 

        /**
         * Get the help message for this argument.
         *
         * @return The help message.
         */
        const std::string &getHelpMsg() const;

        /**
         * Get the argument as a booleen.
         *
         * @return the argument as a boolean
         */
        bool getBoolArg() const; 

        /**
         * Get the argument as a int64_t.
         *
         * @return the argument as a int64_t
         */
        int64_t getIntegerArg() const;

        /**
         * Get the argument as a string.
         *
         * @return the argument as a string
         */
        const std::string &getStringArg() const;

        /**
         * Get the argument as a Notifyable.
         *
         * @param root The Clusterlib root.
         * @return the argument as a Notifyable * or NULL if cannot be found.
         */
        Notifyable *getNotifyableArg(Root *root) const;

        /**
         * Get the argument as a JSONValue.
         *
         * @param argIndex the index of the argument to convert
         * @return the argument as a JSONValue
         */
        json::JSONValue getJsonArg() const;

        /**
         * Get the ArgType of this object.
         *
         * @return The ArgType of this object.
         */
        ArgType getArgType() const;

        /**
         * Get the default value.
         *
         * @return Default value as a string.
         */
        const std::string &getDefaultValue() const;
               
        /**
         * Get whether the argument is required.
         *
         * @return True if the argument is required, false otherwise.
         */
        bool isRequired() const;

        /**
         * Get whether the argument is still the default value.
         *
         * @return True if the argument is still the default value, false
         *         otherwise.
         */        
        bool isDefaultValue() const;

        /**
         * Reset to the default settings.
         */
        void reset();

      private:
        /** The default value as a string. */
        std::string m_defaultValue;

        /** The set value as a string */
        std::string m_value;

        /** Help message on how to use this argument */
        std::string m_helpMsg;

        /** The type of this argument. */
        ArgType m_argType;

        /** Required or optional */
        bool m_required;

        /** Was the value replaced - false (or is it still default) */
        bool m_default;
    };

    /**
     * Set the arguments for this command.
     *
     * @param argValueVec The pairs of name, value for each argument.
     */
    void setArg(
        const std::vector<std::pair<std::string, std::string> > &argValueVec);

    /**
     * Set one argument for this command.
     *
     * @param arg The argument
     * @param value The value
     */
    void setArg(
        const std::string &arg,
        const std::string &value);

    /**
     * Check to make sure that all the required arguments are set.
     */
    void checkArgs();

    /**
     * Reset the arguments to the default state for this commmand.
     */
    void resetArgs();

    /**
     * Get the argument map (for readline).
     */
    const std::map<std::string, CliArg> &getArgMap() const;

    /**
     * Constructor
     */
    CliCommand(std::string command, 
               Client *client,
               bool allowUnknownArgs = false)
        : m_command(command),
          m_client(client),
          m_allowUnknownArgs(allowUnknownArgs) {}

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
    Client *getClient() { return m_client; }

    /**
     * Subclasses commands must define the action to take.  If there
     * was a failure, they should throw the appropriate exception.
     */
    virtual void action() = 0;

    /**
     * Derived command can define a help message.
     *
     * @return the message to print when asked about usage.
     */
    virtual std::string helpMessage() = 0;

    /**
     * Virtual destructor.
     */
    virtual ~CliCommand() {}

    /**
     * Add a new parameter to the command.
     *
     * @param argName The name of the argument.
     * @param defaultValue The default value of the argument.
     * @param helpMsg How to use this argument.
     * @param argType The argument type
     * @param required If true, this object is required.
     */
    void addArg(const std::string &argName,
                const std::string &defaultValue,
                const std::string &helpMsg,
                ArgType argType,
                bool required);

    /**
     * Get the argument.
     *
     * @param argName The name of the argument to get.
     */
    const CliArg &getArg(const std::string &argName) const;

    /**
     *  Generate a string that contains the help messages for all the
     *  arguments.
     *
     * @return The generated string.
     */
    std::string generateArgUsage();

    /**
     * Allow unknown arguments?
     *
     * @return True if unknown arguments are allowed.
     */
    bool allowUnknownArgs();

  private:
    /**
     * Name of the command
     */
    std::string m_command;

    /** 
     * The client of the clusterlib instance 
     */
    Client *m_client;

    /**
     * The map of arguments for this object.
     */
    std::map<std::string, CliArg> m_argMap;

    /**
     * Allow unknown arguments to be added on the command line?
     */
    bool m_allowUnknownArgs;
};

}

#endif	/* !_CL_CLICOMMAND_H_ */
