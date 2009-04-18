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
        throwIfRemoved();

        return m_leadershipChangeTime; 
    }

    /**
     * Get a list of node names in this group.
     * 
     * @return a list of names of nodes in this group.
     */
    virtual NameList getNodeNames();

    /**
     * Get the named node.
     * 
     * @param create create the node if doesn't exist
     * @return NULL if the named node does not exist and create
     * == false
     */
    virtual Node *getNode(const std::string &nodeName, bool create = false);

    /**
     * Get a list of group names in this group.
     * 
     * @return a list of names of groups in this group.
     */
    virtual NameList getGroupNames();

    /**
     * Get the named group.
     * 
     * @param create create the group if doesn't exist.
     * @return NULL if the group does not exist and create
     * == false, else the Group *.
     */
    virtual Group *getGroup(const std::string &groupName, bool create = false);

    /**
     * Get a list of names of data distributions in this group.
     * 
     * @return a list of names of data distributions in this group.
     */
    virtual NameList getDataDistributionNames();

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
    GroupImpl(FactoryOps *f,
              const std::string &key,
              const std::string &name,
              NotifyableImpl *parent)
        : NotifyableImpl(f, key, name, parent),
          mp_leader(NULL),
          m_leaderIsKnown(false),
          m_leadershipChangeTime(0),
          m_leadershipStringsInitialized(false)
    {
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
     * Update the time of the latest leadership change.
     */
    void setLeadershipChangeTime(int64_t t) { m_leadershipChangeTime = t; }

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries();

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    GroupImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException("Someone called the GroupImpl default "
                                       "constructor!");
    }

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
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_GROUPIMPL_H_ */
