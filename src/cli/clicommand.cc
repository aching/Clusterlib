#include "clusterlibinternal.h"
#include "clicommand.h"
#include <iomanip>

using namespace std;
using namespace boost;

namespace clusterlib {

const string CliCommand::NOTIFYABLE_ARG = "ntp";
const string CliCommand::NOTIFYABLE_NAME_ARG = "ntpName";
const string CliCommand::CHILDREN_ARG = "child";
const string CliCommand::KEY_ARG = "key";
const string CliCommand::VALUE_ARG = "value";
const string CliCommand::ZKNODE_ARG = "zknode";
const string CliCommand::FORCE_ARG = "force";

const string CliCommand::UNKNOWN_ARG_HELP_MSG = "Unknown argument";

string
CliCommand::getArgTypeAsString(ArgType argType)
{
    switch (argType) {
        case BoolArg:
            return "Bool";
        case IntegerArg:
            return "Integer";
        case StringArg:
            return "String";
        case NotifyableArg:
            return "Notifyable";
        case JsonArg:
            return "Json";
        case UnknownArg:
            return "Unknown";
        default:
            throw InvalidArgumentsException(
                "getArgTypeAsString: Invalid argType");
    }
}

CliCommand::CliArg::CliArg()
    : m_helpMsg(UNKNOWN_ARG_HELP_MSG),
      m_argType(UnknownArg),
      m_required(false),
      m_default(true)
{
}

CliCommand::CliArg::CliArg(const string &defaultValue, 
                           const string &helpMsg, 
                           ArgType argType, 
                           bool required)
    : m_defaultValue(defaultValue),
      m_helpMsg(helpMsg),
      m_argType(argType),
      m_required(required),
      m_default(true)
{
}

void
CliCommand::CliArg::setValue(const string &value)
{
    if (m_default == false) {
        throw InvalidMethodException("setValue: Already set!");
    }

    m_value = value;
    m_default = false;
}

void
CliCommand::CliArg::promoteDefaultValue()
{
    if (m_default == false) {
        throw InvalidMethodException("promoteDefaultValue: Not default!");
    }

    m_value = m_defaultValue;
}

const string &
CliCommand::CliArg::getNativeArg() const
{
    return m_value;
}

const string &
CliCommand::CliArg::getHelpMsg() const
{
    return m_helpMsg;
}

bool
CliCommand::CliArg::getBoolArg() const
{
    if (m_argType != BoolArg) {
        throw InvalidMethodException(string("getBoolArg: Called on type ") +
                                     getArgTypeAsString(m_argType));
    }

    if (m_value == "true") {
        return true;
    }
    else if (m_value == "false") {
        return false;
    }
    else {
        throw InvalidMethodException(
            string("getBoolArg: Bad value = ") + m_value);
        return false;
    }
}

int64_t
CliCommand::CliArg::getIntegerArg() const
{
    if (m_argType != IntegerArg) {
        throw InvalidMethodException(string("getIntegerArg: Called on type ") +
                                     getArgTypeAsString(m_argType));
    }

    return ::atoi(m_value.c_str());
}

const string &
CliCommand::CliArg::getStringArg() const
{
    if (m_argType != StringArg) {
        throw InvalidMethodException(string("getStringArg: Called on type ") +
                                     getArgTypeAsString(m_argType));
    }

    return m_value;
}

shared_ptr<Notifyable>
CliCommand::CliArg::getNotifyableArg(const shared_ptr<Root> &rootSP) const
{
    if (m_argType != NotifyableArg) {
        throw InvalidMethodException(string("getStringArg: Called on type ") +
                                     getArgTypeAsString(m_argType));
    }

    return rootSP->getNotifyableFromKey(m_value);
}

json::JSONValue
CliCommand::CliArg::getJsonArg() const
{
    if (m_argType != JsonArg) {
        throw InvalidMethodException(string("getJsonArg: Called on type ") +
                                     getArgTypeAsString(m_argType));
    }

    return json::JSONCodec::decode(m_value);
}

CliCommand::ArgType
CliCommand::CliArg::getArgType() const
{
    return m_argType;
}

const string &
CliCommand::CliArg::getDefaultValue() const
{
    return m_defaultValue;
}

bool
CliCommand::CliArg::isRequired() const
{
    return m_required;
}

bool
CliCommand::CliArg::isDefaultValue() const
{
    return m_default;
}

void
CliCommand::CliArg::reset()
{
    m_value.clear();
    m_default = true;
}

void
CliCommand::setArg(const vector<pair<string, string> > &argValueVec)
{
    ostringstream oss;
    map<string, CliCommand::CliArg>::iterator argMapIt;
    vector<pair<string, string> >::const_iterator argValueVecIt;
    for (argValueVecIt = argValueVec.begin();
         argValueVecIt != argValueVec.end();
         ++argValueVecIt) {
        if (!allowUnknownArgs()) {
            argMapIt = m_argMap.find(argValueVecIt->first);
            if (argMapIt == m_argMap.end()) {
                oss.str("");
                oss << "setArg: Couldn't find arg "  << argValueVecIt->first <<
                    " in command " << m_command;
                throw InvalidArgumentsException(oss.str());
            }
        }
        m_argMap[argValueVecIt->first].setValue(argValueVecIt->second);
    }
}

void
CliCommand::setArg(const string &arg, const string &value)
{
    ostringstream oss;
    map<string, CliCommand::CliArg>::iterator argMapIt;
    if (!allowUnknownArgs()) {
        argMapIt = m_argMap.find(arg);
        if (argMapIt == m_argMap.end()) {
            oss.str("");
            oss << "setArg: Couldn't find arg "  << arg <<
                " in command " << m_command;
            throw InvalidArgumentsException(oss.str());
        }
    }
    m_argMap[arg].setValue(value);
}

void
CliCommand::checkArgs()
{
    ostringstream oss;
    map<string, CliArg>::iterator argMapIt;
    for (argMapIt = m_argMap.begin(); argMapIt != m_argMap.end(); ++argMapIt) {
        if (argMapIt->second.isDefaultValue()) {
            if (argMapIt->second.isRequired()) {
                oss << "Missing required argument '" << argMapIt->first 
                    << "'" << endl;
            }
            else {
                /* Promote the default value to the value */
                argMapIt->second.promoteDefaultValue();
            }
        }
    }
    if (!oss.str().empty()) {
        throw InvalidArgumentsException(string("checkArgs: \n") + oss.str());
    }
}

void
CliCommand::resetArgs()
{
    map<string, CliArg>::iterator argMapIt;
    bool removedArg = true;
    /* Get rid of the unknown arguments and reset the rest of them */
    while (removedArg == true) {
        removedArg = false;
        for (argMapIt = m_argMap.begin(); 
             argMapIt != m_argMap.end(); 
             ++argMapIt) {
            if (argMapIt->second.getArgType() == UnknownArg) {
                removedArg = true;
                break;
            }
        }
    }
    for (argMapIt = m_argMap.begin(); argMapIt != m_argMap.end(); ++argMapIt) {
        argMapIt->second.reset();
    }
}

const map<string, CliCommand::CliArg> &
CliCommand::getArgMap() const
{
    return m_argMap;
}

void
CliCommand::addArg(const string &argName,
                   const string &defaultValue, 
                   const string &helpMsg,
                   ArgType argType, 
                   bool required)
{
    map<string, CliArg>::const_iterator argMapIt = m_argMap.find(argName);
    if (argMapIt != m_argMap.end()) {
        throw InvalidMethodException(string("addArg: ") + argName +
                                     " already exists!");
    }

    m_argMap.insert(make_pair<string, CliArg>(argName,
                                              CliArg(defaultValue,
                                                     helpMsg,
                                                     argType,
                                                     required)));
}

const CliCommand::CliArg &
CliCommand::getArg(const string &argName) const
{
    map<string, CliArg>::const_iterator argMapIt = m_argMap.find(argName);
    if (argMapIt == m_argMap.end()) {
        throw InvalidMethodException(string("getArg: ") + argName +
                                     " doesn't exist!");
    }

    return argMapIt->second;
}

string
CliCommand::generateArgUsage()
{
    string res;
    ostringstream oss;

    const int32_t columnWidth = 12;
    oss << left << setw(columnWidth) << "Name"
        << left << setw(columnWidth) << "Type"
        << left << setw(columnWidth) << "Required"
        << left << setw(columnWidth) << "Default"
        << left << setw(columnWidth) << "Usage" << endl;
    res.append(oss.str());

    map<string, CliArg>::const_iterator argMapIt;
    for (argMapIt = m_argMap.begin(); argMapIt != m_argMap.end(); ++argMapIt) {
        oss.str("");
        oss << left << setw(columnWidth) 
            << argMapIt->first
            << left << setw(columnWidth) 
            << getArgTypeAsString(argMapIt->second.getArgType())
            << left << setw(columnWidth) 
            << (argMapIt->second.isRequired() ? "yes" : "optional")
            << left << setw(columnWidth)
            << (!argMapIt->second.isRequired() ? 
                argMapIt->second.getDefaultValue() : "N/A")
            << left << setw(columnWidth)
            << argMapIt->second.getHelpMsg() << endl;
        res.append(oss.str());
    }

    return res;
}

bool
CliCommand::allowUnknownArgs()
{
    return m_allowUnknownArgs;
}

}
