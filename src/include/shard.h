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

#ifndef _SHARD_H__
#define _SHARD_H__

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
    HashRange getStartRange() const 
    { 
        return m_startRange;
    }
    
    /**
     * Get the end of the range (inclusive)
     *
     * @return the end of the range
     */
    HashRange getEndRange() const 
    { 
        return m_endRange;
    }

    /**
     * Get the node pointer
     *
     * @return the Node * asssociated with this Shard (NULL if none)
     */
    const Node *getNode() const
    {
        return m_node;
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
    Shard(HashRange startRange, 
          HashRange endRange, 
          const Node *node, 
          int32_t priority)
        : m_startRange(startRange),
          m_endRange(endRange),
          m_node(node),
          m_priority(priority) {}
    
  private:
    /** Start of the range */
    HashRange m_startRange;

    /** End of the range */
    HashRange m_endRange;

    /** Node associated with this shard */
    const Node *m_node;

    /** Priority of this shard */
    int32_t m_priority;
};

};	/* End of 'namespace clusterlib' */

#endif	/* !_SHARD_H__ */
