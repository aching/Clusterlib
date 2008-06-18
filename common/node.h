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
    /*
     * Retrieve the name of the node.
     */
    const string getName() { return m_name; }

    /*
     * Retrieve the group object for the group that
     * this node is in.
     */
    const Group *getGroup() { return mp_group; }

  protected:
    /*
     * Friend declaration for Factory so that it will be able
     * to call the constructor.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    Node(const Group *group,
         const string &name,
         const string &key,
         FactoryOps *f)
        : Notifyable(f),
          mp_group(group),
          m_name(name),
          m_key(key)
    {
    }

    /*
     * Allow the factory access to my key.
     */
    const string getKey() { return m_key; }

    /*
     * Receive a notification of an event.
     */
    void notify(const Event e);

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    Node()
        : Notifyable(NULL)
    {
        throw ClusterException("Someone called the Node default "
                               "constructor!");
    }

  private:
    /*
     * The group this node is in.
     */
    const Group *mp_group;

    /*
     * The name of this node.
     */
    const string m_name;

    /*
     * The key associated with this node.
     */
    const string m_key;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_NODE_H_ */
