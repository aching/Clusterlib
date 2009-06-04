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
    virtual void becomeLeader();

    virtual void abdicateLeader();

    virtual bool isLeader();

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
    GroupImpl(FactoryOps *f,
              const std::string &key,
              const std::string &name,
              NotifyableImpl *parent)
        : NotifyableImpl(f, key, name, parent) {}

    /*
     * Destructor.
     */
    virtual ~GroupImpl() {};

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
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_GROUPIMPL_H_ */
