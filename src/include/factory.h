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

/**
 * A client visible class to begin accessing clusterlib objects
 */
class Factory 
{
  public:
    /**
     * Create a factory instance, connect it to
     * the specified cluster registry.
     *
     * @param registry the Zookeeper comma separated list of
     *        server:port (i.e. localhost:2221,localhost2:2222).
     */
    Factory(const std::string &registry);

    /*
     * Destructor.
     */
    ~Factory();

    /**
     * Create a cluster client object.  This object is a gateway to
     * the clusterlib objects and a context for user-level events.
     *
     * @return a Client pointer
     */
    Client *createClient();

    /**
     * Create a client for handling JSON-RPC responses.
     *
     * Register a special handler to allow this client to support
     * JSON-RPC response handling.  After JSON-RPC requests are sent,
     * this handler watches for responses.  This handler must be
     * registered prior to any JSON-RPC requests if a response is
     * desired.
     *
     * @param responseQueue the queue this client specifies for the
     *        response to its JSON-RPC requests
     * @param completedQueue the queue this client specifies for the
     *        problems with elements in the response queue
     * @return a Client pointer
     */
    Client *createJSONRPCResponseClient(Queue *responseQueue,
                                        Queue *completedQueue);

    /**
     * Create a client for handling JSON-RPC methods.
     *
     * Register a special handler to allow this client to support
     * JSON-RPC according to a JSONRPCManager.  Any encoded JSON-RPC
     * messages in the recvQueue are processed according to the
     * rpcManager and placed in the sender's response queue if
     * provided or the completedQueue.
     *
     * @param recvQueue the queue where this client receives JSON-RPC requests
     * @param completedQueue the queue where this client places responses or 
     *        errors for JSON-RPC requests if no destination is specified.
     * @param rpcManager actually invokes the methods to process JSON-RPC
     *        requests
     * @return a Client pointer
     */
    Client *createJSONRPCMethodClient(Queue *recvQueue,
                                      Queue *completedQueue,
                                      ::json::rpc::JSONRPCManager *rpcManager);

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
