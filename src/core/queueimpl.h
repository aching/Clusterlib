/*
 * queueimpl.h --
 *
 * Definition of class QueueImpl; it represents a queue of strings in
 * clusterlib.  The strings can use JSON encoding to represent more
 * complex datatypes.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_QUEUEIMPL_H_
#define _CL_QUEUEIMPL_H_

namespace clusterlib
{

/**
 * Definition of class QueueImpl.
 */
class QueueImpl
    : public virtual Queue, 
      public virtual NotifyableImpl
{
  public:
    virtual int64_t put(const std::string &element);

    virtual void take(std::string &element);

    virtual bool takeWaitMsecs(int64_t msecTimeout, 
                               std::string &element);

    virtual bool front(std::string &element);

    virtual int64_t size();

    virtual bool empty();

    virtual void clear();

    virtual bool removeElement(int64_t id);

    virtual std::map<int64_t, std::string> getAllElements();

  public:
    /**
     * Constructor used by Factory.
     */
    QueueImpl(FactoryOps *fp,
              const std::string &key,
              const std::string &name,
              NotifyableImpl *parent);

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~QueueImpl();

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries();

    /**
     * Establish the watch and handler for queue changes.
     */
    void establishQueueWatch();

  private:
    /**
     * Make the default constructor private so it cannot be called.
     */
    QueueImpl()
        : NotifyableImpl(NULL, "", "", NULL)
    {
        throw InvalidMethodException(
            "Someone called the QueueImpl "
            "default constructor!");
    }

    /**
     * Get the queue parent key
     *
     * @return a const reference to the queue parent key
     */
    const std::string &getQueueParentKey() { return m_queueParentKey; }

    /**
     * Key for the queue parent
     */
    std::string m_queueParentKey;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_QUEUEIMPL_H_ */
