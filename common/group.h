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
     * Get the leader node.
     */
    Node *getLeader();

    /**
     * Get the nodes for this object (if it is allowed). If
     * subclasses do not want to allow getNodes(), override it
     * and throw a clusterlib exception.
     * 
     * @return a map of all the different nodes 
     */
    virtual NodeMap getNodes();

    /**
     * Get the node for this object (if it is allowed). If
     * subclasses do not want to allow getNode(), override it
     * and throw a clusterlib exception.
     * 
     * @param create create the node if doesn't exist?
     * @return NULL if no node exists for this notifyable
     */
    virtual Node *getNode(const string &nodeName, 
                          bool create = false);

    /**
     * Get the groups for this object (if it is allowed). If
     * subclasses do not want to allow getGroups(), override it
     * and throw a clusterlib exception.
     * 
     * @return a map of all the different groups 
     */
    virtual GroupMap getGroups();

    /**
     * Get the group for this object (if it is allowed). If
     * subclasses do not want to allow getGroup(), override it
     * and throw a clusterlib exception.
     * 
     * @param create create the group if doesn't exist?
     * @return NULL if no group exists for this notifyable
     */
    virtual Group *getGroup(const string &groupName,
                            bool create = false);

    /**
     * Get the distributions for this object (if it is allowed). If
     * subclasses do not want to allow getDataDistributions(), override it
     * and throw a clusterlib exception.
     * 
     * @return a map of all the different distributions 
     */
    virtual DataDistributionMap getDataDistributions();

    /**
     * Get the distribution for this object (if it is allowed). If
     * subclasses do not want to allow getDataDistribution(), override it
     * and throw a clusterlib exception.
     * 
     * @param create create the distribution if doesn't exist?
     * @return NULL if no distribution exists for this notifyable
     */
    virtual DataDistribution *getDataDistribution(const string &distName,
                                                  bool create = false);


  protected:
    /*
     * Friend declaration so that Factory can call the constructor.
     */
    friend class Factory;

    /*
     * Constructor used by the factory.
     */
    Group(const string &name,
          const string &key,
          FactoryOps *f,
          Notifyable *parent)
        : Notifyable(f, key, name, parent),
          mp_leader(NULL),
          m_leaderIsKnown(false),
          m_leadershipStringsInitialized(false),
          m_cachingNodes(false),
          m_cachingGroups(false),
          m_cachingDists(false)
    {
        m_nodes.clear();
        m_groups.clear();
        m_dists.clear();
    }

    /*
     * Update the known leader.
     */
    void updateLeader(Node *np);

    /*
     * Is the leader known?
     */
    bool isLeaderKnown() { return m_leaderIsKnown; }

    /*
     * Get the leadership lock for this group.
     */
    Mutex *getLeadershipLock() { return &m_leadershipLock; }

    /*
     * Get strings for leadership protocol.
     */
    void initializeStringsForLeadershipProtocol();
    string getCurrentLeaderNodeName();
    string getLeadershipBidsNodeName();
    string getLeadershipBidPrefix();

    /*
     * Are we caching all nodes, groups, and distributions fully?
     */
    bool cachingNodes() { return m_cachingNodes; }
    void recacheNodes();

    bool cachingGroups() { return m_cachingGroups; }
    void recacheGroups();

    bool cachingDists() { return m_cachingDists; }
    void recacheDists();



    /*
     * Update the cached representation of this group.
     */
    virtual void updateCachedRepresentation();

  protected:
    /*
     * Make the default constructor private so it cannot be called.
     */
    Group()
        : Notifyable(NULL, "", "", NULL)
    {
        throw ClusterException("Someone called the Group default "
                               "constructor!");
    }

    /*
     * Make the destructor private also.
     */
    virtual ~Group() {};

    /*
     * Get the addresses of the locks for the node, group, and
     * distriubution maps.
     */
    Mutex *getNodeMapLock() { return &m_nodeMapLock; }
    Mutex *getGroupMapLock() { return &m_grpLock; }
    Mutex *getDataDistributionMapLock() { return &m_distLock; }

  private:
    /*
     * The leader node, if already cached (should be, if
     * there's a leader).
     */
    Node *mp_leader;
    bool m_leaderIsKnown;
    Mutex m_leadershipLock;

    /*
     * Strings that help in the leadership protocol.
     */
    string m_currentLeaderNodeName;
    string m_leadershipBidsNodeName;
    string m_leadershipBidPrefix;
    bool m_leadershipStringsInitialized;

    /*
     * Map of all nodes within this group.
     */
    NodeMap m_nodes;
    Mutex m_nodeMapLock;

    /*
     * Map of all groups within this object.
     */
    GroupMap m_groups;
    Mutex m_grpLock;

    /*
     * Map of all data distributions within this object.
     */
    DataDistributionMap m_dists;
    Mutex m_distLock;

    /*
     * Variables to remember whether we're caching nodes, groups,
     * and distributions fully.
     */
    bool m_cachingNodes;
    bool m_cachingGroups;
    bool m_cachingDists;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_GROUP_H_ */
