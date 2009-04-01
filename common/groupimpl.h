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

/**
 * Definition of class GroupImpl.
 */
class GroupImpl
    : public virtual Group, 
      public virtual NotifyableImpl
{
  public:
    virtual Node *getLeader();

    virtual int64_t getLeadershipChangeTime() 
    { 
        return m_leadershipChangeTime; 
    }

    virtual NameList getNodeNames();

    virtual Node *getNode(const std::string &nodeName, bool create = false);

    virtual NameList getGroupNames();

    virtual Group *getGroup(const std::string &groupName, bool create = false);

    virtual NameList getDataDistributionNames();

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
          m_leadershipStringsInitialized(false) {}

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
