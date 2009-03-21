/*
 * node.h --
 *
 * Definition of class Node; it represents a node in a group in an
 * application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_NODE_H_
#define _NODE_H_

namespace clusterlib
{

/*
 * Definition of class Node
 */
class Node
    : public virtual Notifyable
{
  public:
    /**
     * Is this node the leader of its group?
     *
     * @return true if I am the leader.
     */
    virtual bool isLeader() = 0;

    /**
     * Get the client-state of this node.
     *
     * @return a string representing the client state for this
     * node.
     */
    virtual std::string getClientState() = 0;
    
    /**
     * Get the master-set state of this node.
     *
     * @return an int32 value representing the state set by the
     * master for this node.
     */
    virtual int32_t getMasterSetState() = 0;

    /**
     * Is this node connected?
     *
     * @return true if this node is connected.
     */
    virtual bool isConnected() = 0;

    /*
     * Return the time at which various events happened.
     */
    virtual int64_t getClientStateTime() = 0;
    virtual int64_t getMasterSetStateTime() = 0;
    virtual int64_t getConnectionTime() = 0;

    /**
     * Is this node healthy?
     * 
     * @return true if healthy, false if not
     */
    virtual bool isHealthy() = 0;

    /*
     * Destructor.
     */
    virtual ~Node() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NODE_H_ */
