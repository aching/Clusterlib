/*
 * nodeimpl.h --
 *
 * Definition of class NodeImpl; it represents a node in a group in an
 * application of clusterlib.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_NODEIMPL_H_
#define _CL_NODEIMPL_H_

namespace clusterlib
{

/**
 * Definition of class NodeImpl.
 */
class NodeImpl
    : public virtual Node, 
      public virtual NotifyableImpl
{
  public:
    virtual CachedProcessSlotInfo &cachedProcessSlotInfo();

    virtual NameList getProcessSlotNames();

    virtual ProcessSlot *getProcessSlot(
        const std::string &processSlotName, 
        AccessType accessType = LOAD_FROM_REPOSITORY);

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /**
     * Constructor.
     */
    NodeImpl(FactoryOps *fp,
             const std::string &key,
             const std::string &name,
             GroupImpl *group)
        : NotifyableImpl(fp, key, name, group),
          mp_group(group),
          m_cachedProcessSlotInfo(this) {}

    /**
     * Virtual destructor.
     */
    virtual ~NodeImpl();

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

    /**
     * Create the process slot info JSONObject key
     *
     * @param nodeKey the node key
     * @return the generated process slot info JSONObject key
     */
    static std::string createProcessSlotInfoJSONObjectKey(
        const std::string &nodeKey);

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    NodeImpl()
        : NotifyableImpl(NULL, "", "", NULL),
          m_cachedProcessSlotInfo(this)
    {
        throw InvalidMethodException("Someone called the Node default "
                                       "constructor!");
    }

  private:
    /**
     * The group this node is in.
     */
    GroupImpl *mp_group;

    /**
     * The cached process slot information
     */
    CachedProcessSlotInfoImpl m_cachedProcessSlotInfo;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NODEIMPL_H_ */
