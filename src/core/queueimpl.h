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

#ifndef	_QUEUEIMPL_H_
#define _QUEUEIMPL_H_

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

    virtual std::string take(const int64_t timeout = 0, 
                             bool *timedOut = NULL);

    virtual std::string front(bool *foundFront = NULL);

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
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_QUEUEIMPL_H_ */
