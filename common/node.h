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
 * Define the possible states a node
 * can be in.
 */
enum NodeState {
    NS_ILLEGAL = -1,
    NS_GREEN = 0,
    NS_YELLOW = 1,
    NS_RED = 2,
    NS_DOWN = 3,
    NS_INACTIVE = 4,
    NS_SPARE = 5
};

/*
 * Definition of class Node
 */
class Node
    : public virtual Notifyable
{
  public:
    /*
     * Retrieve the group object for the group that
     * this node is in.
     */
    Group *getGroup() { return mp_group; }

  protected:
    /*
     * Deliver event notifications. This method only updates
     * the cached representation, it is not responsible to
     * deliver the event to registered EventHandler instances.
     */
    void deliverNotification(const Event e);

    /*
     * Friend declaration for Factory so that it will be able
     * to call the constructor.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    Node(Group *group,
         const string &name,
         const string &key,
         FactoryOps *f)
        : Notifyable(f, key, name),
          mp_group(group)
    {
    }

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    Node()
        : Notifyable(NULL, "", "")
    {
        throw ClusterException("Someone called the Node default "
                               "constructor!");
    }

    /*
     * Make the destructor private also.
     */
    ~Node() {}

  private:
    /*
     * The group this node is in.
     */
    Group *mp_group;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NODE_H_ */
