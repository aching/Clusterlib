/*
 * Copyright (c) 2010 Yahoo! Inc. All rights reserved. Licensed under
 * the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License. See accompanying
 * LICENSE file.
 * 
 * $Id$
 */

#ifndef	_CL_QUEUE_H_
#define _CL_QUEUE_H_

namespace clusterlib {

/**
 * Represents a queue of string in clusterlib.  The strings can use
 * JSON encoding to represent more complex datatypes.
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
     * queue.  This will wait unconditionally.
     *
     * @return Element retrieved from the queue if successful
     */
    virtual std::string take() = 0;

    /**
     * Retrieves and removes the head of this queue, waiting if the
     * lock is held by another client or if there are no elements in the
     * queue.  This will wait up to msecTimeout milliseconds.
     *
     * @param msecTimeout the amount of usecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param element the element retrieved from the queue if successful
     * @return true if an element was retrieved, false otherwise
     */
    virtual bool takeWaitMsecs(int64_t msecTimeout, 
                               std::string &element) = 0;

    /**
     * Return the front string in the queue if there is any (does not
     * remove it).  The queue is locked during this operation and
     * unlocked after this operation.
     *
     * @param element the element retrieved from the queue if successful
     * @return true if an element was retrieved, false otherwise
     */
    virtual bool front(std::string &element) = 0;

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

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_QUEUE_H_ */
