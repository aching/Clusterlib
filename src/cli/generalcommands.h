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

#ifndef	_CL_GENERALCOMMANDS_H_
#define _CL_GENERALCOMMANDS_H_

/*
 * All the basic commands that can be used to access/modify clusterlib
 * objects.  These commands may be used by other application clis as
 * desired.
 */

namespace clusterlib  {

/**
 * Set the logging level for the clusterlib library.
 */
class SetLogLevel : public CliCommand
{
  public:
    /** Name of the level argument */
    static const std::string LEVEL_ARG;

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
    /** Name of the application argument */
    static const std::string COMMAND_NAME_ARG;

    Help(CliParams *cliParams);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~Help();

  private:
    /** Pointer to params to get help. */
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
    /** Name of the request argument */
    static const std::string REQUEST_ARG;
    /** Name of the params array argument */
    static const std::string PARAM_ARRAY_ARG;

    JSONRPCCommand(Client *client, 
                   const boost::shared_ptr<Queue> &respQueueSP);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~JSONRPCCommand();

  private:
    /** The queue to put the response on. */
    boost::shared_ptr<Queue> m_respQueueSP;
};

/**
 * Manage a process slot.
 */
class ManageProcessSlot : public CliCommand
{
  public:
    /** Name of the desired state argument */
    static const std::string DESIRED_STATE_ARG;

    ManageProcessSlot(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~ManageProcessSlot();
};

/**
 * Manage an ActiveNode
 */
class ManageActiveNode : public CliCommand
{
  public:
    /** Name of the start argument */
    static const std::string START_ARG;

    ManageActiveNode(Client *client);
    virtual void action();
    virtual std::string helpMessage();
    virtual ~ManageActiveNode();
};

/**
 * AggZookeeperState
 */
class AggZookeeperState : public CliCommand
{
  public:
    /** Name of the zookeeper server list argument */
    static const std::string ZKSERVER_LIST_ARG;

    AggZookeeperState(); 
    virtual void action();
    virtual std::string helpMessage();
    virtual ~AggZookeeperState();
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

/**
 * JsonArg
 */
class JsonArg : public CliCommand
{
  public:
    JsonArg();
    virtual void action();
    virtual std::string helpMessage();
    virtual ~JsonArg();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_GENERALCOMMANDS_H_ */
