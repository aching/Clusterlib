/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */
 
#ifndef _CL_BLOCKINGQUEUE_H_
#define _CL_BLOCKINGQUEUE_H_
 
#include <deque>
#include <sstream>
#include <algorithm>
#include "mutex.h"

namespace clusterlib {
 
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
         * @param e the element to be added
         */
        void put(E e);
        
        /**
         * \brief Retrieves and removes the head of this queue,
         * waiting forever if no elements are present in this
         * queue.
         * 
         * @param e the element filled in
         */
        void take(E &e);
        
        /**
         * \brief Retrieves and removes the head of this queue,
         * waiting if no elements are present in this queue.
         * 
         * @param msecTimeout the amount of msecs to wait until giving up, 
         *        -1 means wait forever, 0 means return immediately
         * @param e the element filled in if returned true
         * @return true if an element was retrieved, false otherwise
         */
        bool takeWaitMsecs(int64_t msecTimeout, E &e);
        
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
void BlockingQueue<E>::put(E e)
{
    m_mutex.acquire();
    m_queue.push_back(e);
    m_cond.signal();
    m_mutex.release();
}

/**
 * Get an element from the blocking queue without a wait
 *
 * @param e the element returned
 */
template<class E> 
void BlockingQueue<E>::take(E &e)
{
    takeWaitMsecs(-1, e);
}

/**
 * Get an element from the blocking queue with/without a wait
 *
 */
template<class E> 
bool BlockingQueue<E>::takeWaitMsecs(int64_t msecTimeout, E &e)
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
        e = m_queue.front();
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

