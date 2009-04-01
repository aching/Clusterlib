/*
 * group.h --
 *
 * Interface class Group; it represents a set of nodes within a
 * specific application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_GROUP_H_
#define _GROUP_H_

namespace clusterlib
{

/**
 * Definition of class Group.
 */
class Group
    : public virtual Notifyable
{
  public:
    /**
     * Get the leader node.
     *
     * @return the Node * for the leader of the group, or NULL.
     */
    virtual Node *getLeader() = 0;

    /**
     * Get the time at which the leadership of this group
     * switched last.
     *
     * @return the time at which the leadership switched, or -1.
     */
    virtual int64_t getLeadershipChangeTime() = 0;

    /**
     * Get a list of names of all nodes.
     * 
     * @return a copy of the list of all nodes
     */
    virtual NameList getNodeNames() = 0;

    /**
     * Get the named node.
     * 
     * @param create create the node if doesn't exist
     * @return NULL if the named node does not exist and create
     * == false
     * @throw ClusterException only if tried to create and couldn't create
     */
    virtual Node *getNode(const std::string &nodeName, 
                          bool create = false) = 0;

    /**
     * Get a list of names of all groups.
     * 
     * @return a copy of the list of all groups
     */
    virtual NameList getGroupNames() = 0;

    /**
     * Get the named group.
     * 
     * @param create create the group if doesn't exist.
     * @return NULL if the group does not exist and create
     * == false, else the Group *.
     * @throw ClusterException only if tried to create and couldn't create
     */
    virtual Group *getGroup(const std::string &groupName, 
                            bool create = false) = 0;


    /**
     * Get a list of names of all data distributions.
     * 
     * @return a copy of the list of all data distributions
     */
    virtual NameList getDataDistributionNames() = 0;

    /**
     * Get the named distribution.
     * 
     * @param create create the distribution if doesn't exist
     * @return NULL if no distribution exists for this notifyable
     * @throw ClusterException only if tried to create and couldn't create
     */
    virtual DataDistribution *getDataDistribution(const std::string &distName,
                                                  bool create = false) = 0;

    /*
     * Destructor.
     */
    virtual ~Group() {};
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_GROUP_H_ */
