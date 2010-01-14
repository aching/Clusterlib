/*
 * queue.h --
 *
 * Interface of class Queue; it represents a queue of string in
 * clusterlib.  The strings can use JSON encoding to represent more
 * complex datatypes.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef	_QUEUE_H_
#define _QUEUE_H_

namespace clusterlib
{

/**
 * Definition of class Queue.
 */
class Queue
    : public virtual Notifyable
{
  public:
    /**
     * Add a new string to the queue (Must be less than 1 MB).
     *
     * @param element the element to be added to the end of the queue
     * @return an identifier for this element in the queue
     */
    virtual int64_t put(const std::string &element) = 0;

    /**
     * Retrieves and removes the head of this queue, waiting if the
     * lock is held by another client or if there are no elements in the
     * queue.
     *
     * @param timeout how long to wait in milliseonds until an element is
     *        available.  If 0, then wait indefinitely.  If <0, return 
     *        immediately
     * @param timedOut if not NULL, then set to true if the function timed out
     *        and did not retrieve an element
     * @return the element from the queue
     */
    virtual std::string take(const int64_t timeout = 0, 
                             bool *timedOut = NULL) = 0;

    /**
     * Return the front string in the queue (does not remove it).  The
     * queue is locked during this operation and unlocked after this
     * operation.
     *
     * @param foundFront if not NULL, will be true if the front was returned,
     *        false if the list was empty
     * @return the front string in the queue.
     */
    virtual std::string front(bool *foundFront = NULL) = 0;

    /**
     * Gets the size of the queue (does not implicitly lock).  Hold
     * the lock to guarantee that the size is maintained after this
     * call.
     *
     * @return the size of the queue
     */
    virtual int64_t size() = 0;

    /**
     * Returns if the queue is empty of not (does not implicitly
     * lock).  Hold the lock to guarantee that the emptyness is
     * maintained after this call.
     * 
     * @return true if empty, false otherwise
     */
    virtual bool empty() = 0;

    /**
     * Erase all elements from the queue.  The queue is locked during
     * this operation.
     */
    virtual void clear() = 0;

    /**
     * Remove an element from the queue given a proper identifier.
     * The queue is locked during this operation.
     * 
     * @param id the identifier from put()
     * @return true if removed successfully, false if not found
     */
    virtual bool removeElement(int64_t id) = 0;

    /**
     * Get a map of all the elements in the queue.
     *
     * @return a map of all the elements in the queue
     */
    virtual std::map<int64_t, std::string> getAllElements() = 0;

    /**
     * Destructor to clean up shards and manual overrides
     */
    virtual ~Queue() {}
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_QUEUE_H_ */