/*
 * clusterlib.h --
 *
 * The main include file for users of clusterlib.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_FACTORY_H_
#define	_FACTORY_H_

namespace clusterlib
{

/*
 * A client visible class to begin accessing clusterlib objects
 */
 class Factory 
{
  public:
    /*
     * Create a factory instance, connect it to
     * the specified cluster registry.
     */
    Factory(const std::string &registry);

    /*
     * Destructor.
     */
    ~Factory();

    /**
     * Create a cluster client object.
     *
     * @return a Client pointer
     */
    Client *createClient();

    /**
     * Create a cluster server object. Also
     * create the needed registration if createReg
     * is set to true.
     *
     * @param group The group this server is in
     * @param name Name of the node in the group
     * @param checker Health checker
     * @param flags The flags used in creating the server
     * @return pointer to the Server object
     */
    Server *createServer(Group *group,
                         const std::string &nodeName,
                         HealthChecker *checker,
                         ServerFlags flags);

    /**
     * Is the factory connected to ZooKeeper?
     * 
     * @return true if connected, false otherwise
     */
    bool isConnected();

    /**
     * Ensure that all operations at this point have been pushed to
     * the underlying data store.
     */
    void synchronize();

    /**
     * For use by unit tests only: get the zkadapter so that the test can
     * synthesize ZK events and examine the results.
     * 
     * @return the ZooKeeperAdapter * from Factory Ops
     */
    zk::ZooKeeperAdapter *getRepository();

  private:
    /**
     * Private access to the m_ops
     */
    FactoryOps *getOps() { return m_ops; }
    
  private:
    /**
     * Does all the factory operations
     */
    FactoryOps *m_ops;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_FACTORY_H_ */
