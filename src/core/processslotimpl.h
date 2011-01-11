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

#ifndef	_CL_PROCESSSLOTIMPL_H_
#define _CL_PROCESSSLOTIMPL_H_

namespace clusterlib {

/**
 * Implements class ProcessSlot.
 */
class ProcessSlotImpl
    : public virtual ProcessSlot, 
      public virtual NotifyableImpl
{
  public:
    virtual CachedProcessInfo &cachedProcessInfo();

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /*
     * Constructor used by the factory.
     */
    ProcessSlotImpl(FactoryOps *fp,
                    const std::string &key,
                    const std::string &name,
                    boost::shared_ptr<NotifyableImpl> node)
        : NotifyableImpl(fp, key, name, node),
          m_cachedProcessInfo(this) {}

    /**
     * Create the key-value JSONArray key
     *
     * @param processSlotKey the ProcessSlot key
     * @return the generated process info JSONArray key
     */
    static std::string createProcessInfoJsonArrKey(
        const std::string &processSlotKey);

    /**
     * Helper function to get the exeutable arguments from a cancelled
     * state.
     */
    void getExecArgs(CachedState &cachedState,
                     std::vector<std::string> &addEnv, 
                     std::string &path, 
                     std::string &command);

    /**
     * Start up the process with given arguments this node.
     *
     * @param addEnv The added environment for the new process.
     * @param path The path to execute the command.
     * @param command The command to start.
     * @return the pid of the new process
     */
    pid_t startLocal(const std::vector<std::string> &addEnv, 
                     const std::string &path, 
                     const std::string &command);
    
    /**
     * Stop the process with the user-defined args on this server
     *
     * @param pid The pid to send the signal to.
     * @param signal The kill signal.
     */
    void stopLocal(pid_t pid, int32_t signal);

    /**
     * Create a string with the default executable arguments
     */
    static std::string createDefaultExecArgs();
    
    /*
     * Destructor.
     */
    virtual ~ProcessSlotImpl();

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

  private:
    /*
     * Do not call the default constructor
     */
    ProcessSlotImpl();

  private:
    /**
     * The cached process info
     */
    CachedProcessInfoImpl m_cachedProcessInfo;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_PROCESSSLOTIMPL_H_ */
