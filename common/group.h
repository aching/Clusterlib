/*
 * group.h --
 *
 * Definition of class Group; it represents a set of nodes within a specific
 * application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_GROUP_H_
#define _GROUP_H_

namespace clusterlib
{

/*
 * Definition of class Group.
 */
class Group
    : public virtual Notifyable
{
  public:
    /*
     * Retrieve the application object for the application
     * that this group is part of.
     */
    Application *getApplication() { return mp_app; }

    /*
     * Retrieve a node with a given name in this group.
     */
    Node *getNode(const string &nodeName, 
		  bool create = false);

    /*
     * Retrieve the map of nodes in this group.
     */
    NodeMap *getNodes() { return &m_nodes; }

    /*
     * Has the node map already been filled?
     */
    bool filledNodeMap() { return m_filledNodeMap; }

  protected:
    /*
     * Friend declaration so that Factory can call the constructor.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    Group(Application *app,
          const string &name,
          const string &key,
          FactoryOps *f)
        : Notifyable(f, key, name),
          mp_app(app),
          m_filledNodeMap(false)
    {
        m_nodes.clear();

        updateCachedRepresentation();
    }

    /*
     * Set the filled flag for the node map.
     */
    void setFilledNodeMap(bool v) { m_filledNodeMap = v; }

    /*
     * Update the cached representation of this group.
     */
    virtual void updateCachedRepresentation();

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    Group()
        : Notifyable(NULL, "", "")
    {
        throw ClusterException("Someone called the Group default "
                               "constructor!");
    }

    /*
     * Make the destructor private also.
     */
    virtual ~Group() {};

    /*
     * Get the address of the lock for the node map.
     */
    Mutex *getNodeMapLock() { return &m_nodeMapLock; }

  private:
    /*
     * The application object that contains this group.
     */
    Application *mp_app;

    /*
     * Map of all nodes within this group.
     */
    NodeMap m_nodes;
    Mutex m_nodeMapLock;

    /*
     * Denote whether we've filled the node map.
     */
    bool m_filledNodeMap;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_GROUP_H_ */
