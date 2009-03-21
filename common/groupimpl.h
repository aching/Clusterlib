/*
 * groupimpl.h --
 *
 * Definition of class GroupImpl; it represents a set of nodes within
 * a specific application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_GROUPIMPL_H_
#define _GROUPIMPL_H_

namespace clusterlib
{

/*
 * Definition of class Group.
 */
class GroupImpl
    : public virtual Group, 
      public virtual NotifyableImpl
{
  public:
    /**
     * Get the leader node.
     *
     * @return the Node * for the leader of the group, or NULL.
     */
    virtual Node *getLeader();

    /**
     * Get the time at which the leadership of this group
     * switched last.
     *
     * @return the time at which the leadership switched, or -1.
     */
    virtual int64_t getLeadershipChangeTime() 
    { 
        return m_leadershipChangeTime; 
    }

    /**
     * Get a copy of the map of all the nodes in this group.
     * 
     * @return a copy of the map of all the nodes in this group. 
     */
    virtual NodeMap getNodes() 
    {
        Locker l(getNodeMapLock());

        m_cachingNodes = true;
        recacheNodes();
        return m_nodes;
    }

    /**
     * Get the named node.
     * 
     * @param create create the node if doesn't exist
     * @return NULL if the named node does not exist and create
     * == false
     */
    virtual Node *getNode(const std::string &nodeName, bool create = false);

    /**
     * Get a copy of the map of all the groups in this group.
     * 
     * @return a copy of the map of all the groups in this group.
     */
    virtual GroupMap getGroups()
    {
        Locker l(getGroupMapLock());

        m_cachingGroups = true;
        recacheGroups();
        return m_groups;
    }

    /**
     * Get the named group.
     * 
     * @param create create the group if doesn't exist.
     * @return NULL if the group does not exist and create
     * == false, else the Group *.
     */
    virtual Group *getGroup(const std::string &groupName, bool create = false);

    /**
     * Get a copy of the map of all distributions in this group.
     * 
     * @return a copy of the map of all the distributions in this group.
     */
    virtual DataDistributionMap getDataDistributions()
    {
        Locker l(getDataDistributionMapLock());

        m_cachingDists = true;
        recacheDataDistributions();
        return m_dists;
    }

    /**
     * Get the named distribution.
     * 
     * @param create create the distribution if doesn't exist
     * @return NULL if no distribution exists for this notifyable
     */
    virtual DataDistribution *getDataDistribution(const std::string &distName,
                                                  bool create = false);


    /*
     * Internal functions not used by outside clients
     */
  public:
    /*
     * Constructor used by the factory.
     */
    GroupImpl(const std::string &name,
              const std::string &key,
              FactoryOps *f,
              NotifyableImpl *parent)
        : NotifyableImpl(f, key, name, parent),
          mp_leader(NULL),
          m_leaderIsKnown(false),
          m_leadershipChangeTime(0),
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
     * Destructor.
     */
    virtual ~GroupImpl() {};

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
    std::string getCurrentLeaderNodeName();
    std::string getLeadershipBidsNodeName();
    std::string getLeadershipBidPrefix();

    /*
     * Are we caching all nodes fully?
     */
    bool cachingNodes() { return m_cachingNodes; }
    void recacheNodes();

    /*
     * Are we caching all groups fully?
     */
    bool cachingGroups() { return m_cachingGroups; }
    void recacheGroups();

    /*
     * Are we caching all distributions?
     */
    bool cachingDataDistributions() { return m_cachingDists; }
    void recacheDataDistributions();

    /*
     * Update the time of the latest leadership change.
     */
    void setLeadershipChangeTime(int64_t t) { m_leadershipChangeTime = t; }

    /*
     * Initialize the cached representation of this group.
     */
    virtual void initializeCachedRepresentation();

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    GroupImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw ClusterException("Someone called the GroupImpl default "
                               "constructor!");
    }

    /*
     * Get the address of the lock for the node, group, and
     * distribution maps.
     */
    Mutex *getNodeMapLock() { return &m_nodeMapLock; }
    Mutex *getGroupMapLock() { return &m_groupMapLock; }
    Mutex *getDataDistributionMapLock() { return &m_distMapLock; }

  private:
    /*
     * The leader node, if already cached (should be, if
     * there's a leader).
     */
    Node *mp_leader;
    bool m_leaderIsKnown;
    int64_t m_leadershipChangeTime;
    Mutex m_leadershipLock;

    /*
     * Strings that help in the leadership protocol.
     */
    std::string m_currentLeaderNodeName;
    std::string m_leadershipBidsNodeName;
    std::string m_leadershipBidPrefix;
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
    Mutex m_groupMapLock;

    /*
     * Map of all data distributions within this object.
     */
    DataDistributionMap m_dists;
    Mutex m_distMapLock;

    /*
     * Are we caching nodes, groups, or distributions fully?
     */
    bool m_cachingNodes;
    bool m_cachingGroups;
    bool m_cachingDists;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_GROUPIMPL_H_ */
