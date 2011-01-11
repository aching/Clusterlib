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
 
#ifndef _CL_BLOCKINGQUEUE_H_
#define _CL_BLOCKINGQUEUE_H_

namespace clusterlib  {
 
/**
 * \brief An unbounded blocking queue of elements of type E.
 * 
 * This class is thread safe.
 */
template <class E>
class BlockingQueue
{
    public:
        
        /**
         * \brief Adds the specified element to this queue, waiting if
         * necessary for space to become available.
         * 
         * @param element the element to be added
         */
        void put(E element);
        
        /**
         * \brief Retrieves and removes the head of this queue,
         * waiting forever if no elements are present in this
         * queue.
         * 
         * @return Element from the queue
         */
        E take();
        
        /**
         * \brief Retrieves and removes the head of this queue,
         * waiting if no elements are present in this queue.
         * 
         * @param msecTimeout the amount of msecs to wait until giving up, 
         *        -1 means wait forever, 0 means return immediately
         * @param element the element filled in if returned true
         * @return true if an element was retrieved, false otherwise
         */
        bool takeWaitMsecs(int64_t msecTimeout, E &element);
        
        /**
         * Returns the current size of this blocking queue.
         * 
         * @return the number of elements in this queue
         */
        int32_t size() const;
        
        /**
         * \brief Returns whether this queue is empty or not.
         * 
         * @return true if this queue has no elements; false otherwise
         */
        bool empty() const;

        /**
         * \brief Erases the queue.
         */
        void erase();

    private:        
        /**
         * The queue of elements. Deque is used to provide O(1) time 
         * for head elements removal.
         */
        std::deque<E> m_queue;
        
        /**
         * The mutex used for queue synchronization.
         */
        mutable Mutex m_mutex;
        
        /**
         * The conditionial variable associated with the mutex above.
         */
        mutable Cond m_cond;
};

template<class E>
int BlockingQueue<E>::size() const
{
    int32_t size;
    m_mutex.acquire();
    size = m_queue.size();
    m_mutex.release();
    return size;
}

template<class E>
bool BlockingQueue<E>::empty() const
{
    bool isEmpty;
    m_mutex.acquire();
    isEmpty = m_queue.empty();
    m_mutex.release();
    return isEmpty;
}

template<class E> 
void BlockingQueue<E>::put(E element)
{
    m_mutex.acquire();
    m_queue.push_back(element);
    m_cond.signal();
    m_mutex.release();
}

template<class E> 
E BlockingQueue<E>::take()
{
    m_mutex.acquire();
    while (m_queue.empty()) {
        m_cond.wait(m_mutex);
    }
    E element = m_queue.front();
    m_queue.pop_front();
    m_mutex.release();
    return element;
}

template<class E> 
bool BlockingQueue<E>::takeWaitMsecs(int64_t msecTimeout, E &element)
{
    if (msecTimeout < -1) {
        std::stringstream ss;
        ss << "takeWaitMsecs: Cannot have msecTimeout < -1 (" 
           << msecTimeout << ")";
        throw InvalidArgumentsException(ss.str());
    }

    /* Adjust the curUsecTimeout for msecTimeout */
    int64_t curUsecTimeout = 0;
    int64_t maxUsecs = 0;
    if (msecTimeout != -1) {
        maxUsecs = TimerService::getCurrentTimeUsecs() + (msecTimeout * 1000);
    }
    else {
        curUsecTimeout = -1;
    }

    m_mutex.acquire();
    bool hasResult = true;
    while (m_queue.empty()) {
        if (curUsecTimeout != -1) {
            /* Don't let curUsecTimeout go negative if not already -1. */
            curUsecTimeout = std::max(
                maxUsecs - TimerService::getCurrentTimeUsecs(), 
                static_cast<int64_t>(0));
        }        
        if (!m_cond.waitUsecs(m_mutex, curUsecTimeout)) {
            hasResult = false;
            break;
        }

        /* Ran out of time */
        if ((curUsecTimeout != -1 ) &&
            (TimerService::compareTimeUsecs(maxUsecs) >= 0)) {
            hasResult = false;
            break;
        }
    }
    if (hasResult) {
        element = m_queue.front();
        m_queue.pop_front();
        m_mutex.release();
        return true;
    } 
    else {
        m_mutex.release();
        return false;
    }
}

template<class E>
void BlockingQueue<E>::erase()
{
    m_mutex.acquire();
    m_queue.resize(0);
    m_mutex.release();
 }

}	/* End of 'namespace clusterlib' */

#endif  /* _CL_BLOCKINGQUEUE_H_ */

