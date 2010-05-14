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
    virtual NameList getNodeNames();

    virtual Node *getNode(
        const std::string &name, AccessType accessType = LOAD_FROM_REPOSITORY);

    virtual NameList getGroupNames();

    virtual Group *getGroup(const std::string &name, 
                            AccessType accessType = LOAD_FROM_REPOSITORY);

    virtual NameList getDataDistributionNames();

    virtual DataDistribution *getDataDistribution(
        const std::string &name,
        AccessType accessType = LOAD_FROM_REPOSITORY);

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
    GroupImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException("Someone called the GroupImpl default "
                                       "constructor!");
    }
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_GROUPIMPL_H_ */
