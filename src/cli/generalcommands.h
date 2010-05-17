/*
 * generalcommands.h --
 *
 * All the basic commands that can be used to access/modify clusterlib
 * objects.  These commands may be used by other application clis as
 * desired.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_GENERALCOMMANDS_H_
#define _CL_GENERALCOMMANDS_H_

namespace clusterlib {

/**
 * Set the logging level for the clusterlib library.
 */
class SetLogLevel : public CliCommand
{
  public:
    SetLogLevel();
    virtual void action();
    virtual std::string helpMessage();
    virtual ~SetLogLevel();
};

/**
 * Remove a notifyable
 */
class RemoveNotifyable : public CliCommand
{
  public:
    RemoveNotifyable(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~RemoveNotifyable();
};

/**
 * Get all the lock bids on this notifyable and possibly its children.
 */
class GetLockBids : public CliCommand
{
  public:
    GetLockBids(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~GetLockBids();
};

/**
 * Get the children of a notifyable
 */
class GetChildren : public CliCommand
{
  public:
    GetChildren(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~GetChildren();
};

/**
 * Get the attributes of a notifyable
 */
class GetAttributes : public CliCommand
{
  public:
    GetAttributes(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~GetAttributes();
};

/**
 * Set a current state key-value of a notifyable
 */
class SetCurrentState : public CliCommand
{
  public:
    SetCurrentState(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~SetCurrentState();
};

/**
 * Set a desired state key-value of a notifyable
 */
class SetDesiredState : public CliCommand
{
  public:
    SetDesiredState(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~SetDesiredState();
};

/**
 * Add an application to the clusterlib hierarchy
 */
class AddApplication : public CliCommand
{
  public:
    AddApplication(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AddApplication();
};

/**
 * Add a group to the clusterlib hierarchy
 */
class AddGroup : public CliCommand
{
  public:
    AddGroup(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AddGroup();
};

/**
 * Add a data distribution to the clusterlib hierarchy
 */
class AddDataDistribution : public CliCommand
{
  public:
    AddDataDistribution(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AddDataDistribution();
};

/**
 * Add a node to the clusterlib hierarchy
 */
class AddNode : public CliCommand
{
  public:
    AddNode(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AddNode();
};

/**
 * Add a property list to the clusterlib hierarchy
 */
class AddPropertyList : public CliCommand
{
  public:
    AddPropertyList(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AddPropertyList();
};

/**
 * Add a queue to the clusterlib hierarchy
 */
class AddQueue : public CliCommand
{
  public:
    AddQueue(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AddQueue();
};

/**
 * Get raw zookeeper node data
 */
class GetZnode : public CliCommand
{
  public:
    GetZnode(Factory *factory, Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~GetZnode();

  private:
    Factory *m_factory;
};

/**
 * Get raw zookeeper node children
 */
class GetZnodeChildren : public CliCommand
{
  public:
    GetZnodeChildren(Factory *factory, Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~GetZnodeChildren();

  private:
    Factory *m_factory;
};

/**
 * Get commands and arguments.
 */
class Help : public CliCommand
{
  public:
    Help(CliParams *cliParams);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~Help();

  private:
    CliParams *m_params;
};

/**
 * Add CLI aliases
 */
class AddAlias : public CliCommand
{
  public:
    AddAlias(CliParams *cliParams);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AddAlias();

  private:
    CliParams *m_params;
};

/**
 * Remove CLI aliases
 */
class RemoveAlias : public CliCommand
{
  public:
    RemoveAlias(CliParams *cliParams);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~RemoveAlias();

  private:
    CliParams *m_params;
};

/**
 * Get a CLI alias's replacement
 */
class GetAliasReplacement : public CliCommand
{
  public:
    GetAliasReplacement(CliParams *cliParams);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~GetAliasReplacement();

  private:
    CliParams *m_params;
};

/**
 * Issue JSON-RPC request
 */
class JSONRPCCommand : public CliCommand
{
  public:
    JSONRPCCommand(Client *client, Queue *respQueue);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~JSONRPCCommand();

  private:
    Queue *m_respQueue;
};

/**
 * Start a running process.
 */
class StartProcessSlot : public CliCommand
{
  public:
    StartProcessSlot(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~StartProcessSlot();
};

/**
 * Stop a running process.
 */
class StopProcessSlot : public CliCommand
{
  public:
    StopProcessSlot(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~StopProcessSlot();
};

/**
 * Shutdown an ActiveNode
 */
class StopActiveNode : public CliCommand
{
  public:
    StopActiveNode(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~StopActiveNode();
};

/**
 * Quit
 */
class Quit : public CliCommand
{
  public:
    Quit(CliParams *cliParams); 
    virtual void action();
    virtual std::string helpMessage();
    virtual ~Quit();

  private:
    CliParams *m_params;
};

/**
 * BoolArg
 */
class BoolArg : public CliCommand
{
  public:
    BoolArg();
    virtual void action();
    virtual std::string helpMessage();
    virtual ~BoolArg();
};

/**
 * IntegerArg
 */
class IntegerArg : public CliCommand
{
  public:
    IntegerArg();
    virtual void action();
    virtual std::string helpMessage();
    virtual ~IntegerArg();
};

/**
 * StringArg
 */
class StringArg : public CliCommand
{
  public:
    StringArg();
    virtual void action();
    virtual std::string helpMessage();
    virtual ~StringArg();
};

/**
 * NotifyableArg
 */
class NotifyableArg : public CliCommand
{
  public:
    NotifyableArg(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~NotifyableArg();
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_GENERALCOMMANDS_H_ */
