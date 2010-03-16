/*
 * activenode.h --
 *
 * Definition of class ActiveNode; it manages the parameters of
 * the active node process.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_ACTIVENODE_H_
#define _CL_ACTIVENODE_H_

#include <iostream>
#include <vector>
#include <getopt.h>

namespace activenode {

/**
 * Active node
 */
class ActiveNode {
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
    clusterlib::Node *getActiveNode();

    /**
     * Get the root pointer.
     *
     * @return the pointer to the root
     */
    clusterlib::Root *getRoot();

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

  private:
    /**
     * Parsed parameters.
     */
    ActiveNodeParams m_params;

    /**
     * Health checker for the active node.
     */
    std::auto_ptr<clusterlib::HealthChecker> m_nodeHealthChecker;
    
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
    clusterlib::Root *m_root;

    /**
     * The group that has the active node
     */
    clusterlib::Group *m_activeNodeGroup;

    /**
     * The clusterlib node that represents this physical node
     */
    clusterlib::Node *m_activeNode;

    /**
     * The vectory of all the handler pointers
     */
    std::vector<clusterlib::UserEventHandler *> m_handlerVec;

    /**
     * When the signal is sent, the shutdown will begin.
     */
    clusterlib::PredMutexCond m_predMutexCond;
};

}

#endif	/* !_CL_ACTIVENODE_H_ */
