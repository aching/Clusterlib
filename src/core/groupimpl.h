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

#ifndef	_CL_GROUPIMPL_H_
#define _CL_GROUPIMPL_H_

namespace clusterlib {

/*
 * Definition of class Group.
 */
class GroupImpl
    : public virtual Group, 
      public virtual NotifyableImpl
{
  public:
    virtual NameList getNodeNames();

    virtual bool getNodeWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Node> *pNodeSP);

    virtual boost::shared_ptr<Node> getNode(
        const std::string &name,
        AccessType accessType);

    virtual NameList getGroupNames();

    virtual bool getGroupWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Group> *pGroupSP);

    virtual boost::shared_ptr<Group> getGroup(
        const std::string &name,
        AccessType accessType);

    virtual NameList getDataDistributionNames();

    virtual bool getDataDistributionWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<DataDistribution> *pDataDistributionSP);

    virtual boost::shared_ptr<DataDistribution> getDataDistribution(
        const std::string &name,
        AccessType accessType);

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
              boost::shared_ptr<NotifyableImpl> parent)
        : NotifyableImpl(f, key, name, parent) {}

    /*
     * Destructor.
     */
    virtual ~GroupImpl() {};

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    GroupImpl();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_GROUPIMPL_H_ */
