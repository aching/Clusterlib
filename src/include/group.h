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

#ifndef	_CL_GROUP_H_
#define _CL_GROUP_H_

namespace clusterlib {

/**
 * Definition of class Group.
 */
class Group
    : public virtual Notifyable
{
  public:
    /**
     * Get a list of names of all nodes.
     * 
     * @return a copy of the list of all nodes
     */
    virtual NameList getNodeNames() = 0;

    /**
     * Get the name Node.
     *
     * @param name Name of the Node to create
     * @param accessType Mode of access
     * @param msecTimeout Amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pNodeSP Pointer to that pNodeSP if it exists, 
     *        therwise NULL.
     * @return True if the operation finished before the timeout
     */
    virtual bool getNodeWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Node> *pNodeSP) = 0;

    /**
     * Get the named Node (no timeout).
     * 
     * @param name Name of the Node to get
     * @param accessType Mode of access
     * @return NULL if the named Node does not exist
     * @throw Exception only if tried to create and couldn't create
     */
    virtual boost::shared_ptr<Node> getNode(
        const std::string &name,
        AccessType accessType) = 0;

    /**
     * Get a list of names of all groups.
     * 
     * @return a copy of the list of all groups
     */
    virtual NameList getGroupNames() = 0;

    /**
     * Get the name Group.
     *
     * @param name Name of the Group to create
     * @param accessType Mode of access
     * @param msecTimeout Amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pGroupSP Pointer to that pGroupSP if it exists, 
     *        otherwise NULL.
     * @return True if the operation finished before the timeout
     */
    virtual bool getGroupWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Group> *pGroupSP) = 0;

    /**
     * Get the named Group (no timeout).
     * 
     * @param name Name of the Group to get
     * @param accessType Mode of access
     * @return NULL if the named Group does not exist
     * @throw Exception only if tried to create and couldn't create
     */
    virtual boost::shared_ptr<Group> getGroup(
        const std::string &name,
        AccessType accessType) = 0;

    /**
     * Get a list of names of all data distributions.
     * 
     * @return a copy of the list of all data distributions
     */
    virtual NameList getDataDistributionNames() = 0;

    /**
     * Get the name DataDistribution.
     *
     * @param name Name of the DataDistribution to create
     * @param accessType Mode of access
     * @param msecTimeout Amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pDataDistributionSP Pointer to that pDataDistributionSP if it
     *        exists, otherwise NULL.
     * @return True if the operation finished before the timeout
     */
    virtual bool getDataDistributionWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<DataDistribution> *pDataDistributionSP) = 0;

    /**
     * Get the named DataDistribution (no timeout).
     * 
     * @param name Name of the DataDistribution to get
     * @param accessType Mode of access
     * @return NULL if the named DataDistribution does not exist
     * @throw Exception only if tried to create and couldn't create
     */
    virtual boost::shared_ptr<DataDistribution> getDataDistribution(
        const std::string &name,
        AccessType accessType) = 0;

    /*
     * Destructor.
     */
    virtual ~Group() {};
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_GROUP_H_ */
