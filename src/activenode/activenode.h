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

#ifndef	_CL_ACTIVENODE_H_
#define _CL_ACTIVENODE_H_

namespace activenode {

/**
 * Creates an "active node" that can be managed.  It is designed to be
 * extended to handle user-defined RPC methods and manage various
 * processes.  Additionally it checks its own health.
 */
class ActiveNode 
{
  public:
    /**
     * Constructor.
     *
     * @param params the parsed parameters from the command line.
     * @param factory the clusterlib factory reference
     */
    ActiveNode(const ActiveNodeParams &params, 
               clusterlib::Factory *factory);

    /**
     * Destructor.
     */
    ~ActiveNode();

    /**
     * Get the active node pointer.
     *
     * @return the pointer to the active node
     */
    const boost::shared_ptr<clusterlib::Node> &getActiveNode();

    /**
     * Get the root pointer.
     *
     * @return the pointer to the root
     */
    const boost::shared_ptr<clusterlib::Root> &getRoot();

    /**
     * Start the loop of running the active node with the number of
     * threads equal to the size of the rpcManagerVec.
     *
     * @param rpcManagerVec the vector of rpcManagers (one will be
     *        used for each thread).
     * @return 0 if returned normally, -1 if returned with a problem.
     */
    int32_t run(
        ::std::vector< ::clusterlib::ClusterlibRPCManager *> &rpcManagerVec);

    /**
     * Get the ActiveNodePeriodicCheck pointer.
     *
     * @return Pointer to m_activeNodePeriodicCheck.
     */
    clusterlib::Periodic *getActiveNodePeriodicCheck();

  private:
    /**
     * Parsed parameters.
     */
    ActiveNodeParams m_params;

    /**
     * The clusterlib factory
     */
    clusterlib::Factory *m_factory;

    /**
     * The clusterlib client context used
     */
    clusterlib::Client *m_client;

    /**
     * The clusterlib root
     */
    boost::shared_ptr<clusterlib::Root> m_rootSP;

    /**
     * The group that has the active node
     */
    boost::shared_ptr<clusterlib::Group> m_activeNodeGroupSP;

    /**
     * The clusterlib node that represents this physical node
     */
    boost::shared_ptr<clusterlib::Node> m_activeNodeSP;

    /**
     * The vector of all the handler pointers
     */
    std::vector<clusterlib::UserEventHandler *> m_handlerVec;

    /**
     * The vector of all the Periodic functions.
     */
    std::vector<clusterlib::Periodic *> m_periodicVec;

    /**
     * Pointer to the ActiveNodePeriodicCheck
     */
    clusterlib::Periodic *m_activeNodePeriodicCheck;

    /**
     * When the signal is sent, the shutdown will begin.
     */
    clusterlib::PredMutexCond m_predMutexCond;
};

}

#endif	/* !_CL_ACTIVENODE_H_ */
