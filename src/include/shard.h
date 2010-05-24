/*
 * shard.h --
 *
 * Definition of class Shard; it represents an element in a data
 * distribution.
 *
 * $Header:$
 * $Revision$
 * $Date$
 */

#ifndef _CL_SHARD_
#define _CL_SHARD_

namespace clusterlib
{

/**
 * Definition and implementation of class Shard.
 */
class Shard {
  public:
    /**
     * Get the start of the range (inclusive)
     *
     * @return the start of the range
     */
    HashRange &getStartRange() const 
    { 
        if (m_startRange == NULL) {
            throw InvalidMethodException("getStartRange: No start range set.");
        }
        return *m_startRange;
    }
    
    /**
     * Get the end of the range (inclusive)
     *
     * @return the end of the range
     */
    HashRange &getEndRange() const 
    { 
        if (m_endRange == NULL) {
            throw InvalidMethodException("getEndRange: No end range set.");
        }
        return *m_endRange;
    }

    /**
     * Get the notifyable pointer
     *
     * @return the Notifyable * asssociated with this Shard (NULL if none)
     */
    Notifyable *getNotifyable() const
    {
        return m_notifyable;
    }

    /**
     * Get the priority
     *
     * @return the current priority of this shard
     */
    int32_t getPriority() const
    {
        return m_priority;
    }

    /**
     * Constructor.
     */
    Shard(const HashRange &startRange, 
          const HashRange &endRange, 
          Notifyable *ntp, 
          int32_t priority)
        : m_startRange(&(startRange.create())),
          m_endRange(&(endRange.create())),
          m_notifyable(ntp),
          m_priority(priority) 
    {
        *m_startRange = startRange;
        *m_endRange = endRange;
    }

    /**
     * Default constructor.
     */
    Shard()
        : m_startRange(NULL),
          m_endRange(NULL),
          m_notifyable(NULL),
          m_priority(-1) {}

    /**
     * Copy constructor.
     */
    Shard(const Shard &other)
        : m_startRange(&(other.getStartRange().create())),
          m_endRange(&(other.getEndRange().create())),
          m_notifyable(other.getNotifyable()),
          m_priority(other.getPriority())
    {
        *m_startRange = other.getStartRange();
        *m_endRange = other.getEndRange();
    }

    /**
     * Destructor.
     */
    ~Shard() 
    {
        if (m_startRange != NULL) {
            delete m_startRange;
        }
        if (m_endRange != NULL) {
            delete m_endRange;
        }
    }

    /**
     * Assignment operator for deep copy.
     */
    Shard & operator= (const Shard &other)
    {
        if (m_startRange == NULL) {
            m_startRange = &(other.getStartRange().create());
            *m_startRange = other.getStartRange();
        }
        if (m_endRange == NULL) {
            m_endRange = &(other.getEndRange().create());
            *m_endRange = other.getEndRange();
        }
        m_notifyable = other.getNotifyable();
        m_priority = other.getPriority();

        return *this;
    }

  private:
    /** Start of the range */
    HashRange *m_startRange;

    /** End of the range */
    HashRange *m_endRange;

    /** Notifyable associated with this shard */
    mutable Notifyable *m_notifyable;

    /** Priority of this shard */
    int32_t m_priority;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_CL_SHARD_ */
