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

/*
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
     * Get a copy of the map of all the nodes in this group.
     * 
     * @return a copy of the map of all the nodes in this group. 
     */
    virtual NodeMap getNodes() = 0;

    /**
     * Get the named node.
     * 
     * @param create create the node if doesn't exist
     * @return NULL if the named node does not exist and create
     * == false
     */
    virtual Node *getNode(const std::string &nodeName, 
                          bool create = false) = 0;
    
    /**
     * Get a copy of the map of all the groups in this group.
     * 
     * @return a copy of the map of all the groups in this group.
     */
    virtual GroupMap getGroups() = 0;

    /**
     * Get the named group.
     * 
     * @param create create the group if doesn't exist.
     * @return NULL if the group does not exist and create
     * == false, else the Group *.
     */
    virtual Group *getGroup(const std::string &groupName, 
                            bool create = false) = 0;

    /**
     * Get a copy of the map of all distributions in this group.
     * 
     * @return a copy of the map of all the distributions in this group.
     */
    virtual DataDistributionMap getDataDistributions() = 0;

    /**
     * Get the named distribution.
     * 
     * @param create create the distribution if doesn't exist
     * @return NULL if no distribution exists for this notifyable
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
