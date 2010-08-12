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

#ifndef _CL_SHARD_H_
#define _CL_SHARD_H_

namespace clusterlib {

/**
 * Definition and implementation of class Shard.
 */
class Shard
{
  public:
    /**
     * Get the Root pointer.
     */
    boost::shared_ptr<Root> getRoot() const;

    /**
     * Get the start of the range (inclusive)
     *
     * @return the start of the range
     */
    HashRange &getStartRange() const;
    
    /**
     * Get the end of the range (inclusive)
     *
     * @return the end of the range
     */
    HashRange &getEndRange() const;

    /**
     * Get the notifyable pointer
     *
     * @return the Notifyable * asssociated with this Shard (NULL if none)
     */
    boost::shared_ptr<Notifyable> getNotifyable() const;

    /**
     * Get the Notifyable key
     *
     * @return the Notifyable key associated with this shard.
     */
    std::string getNotifyableKey() const;

    /**
     * Get the priority
     *
     * @return the current priority of this shard
     */
    int32_t getPriority() const;

    /**
     * Constructor.
     */
    Shard(const boost::shared_ptr<Root> &rootSP,
          const HashRange &startRange, 
          const HashRange &endRange, 
          std::string notifyableKey, 
          int32_t priority);

    /**
     * Default constructor.
     */
    Shard();

    /**
     * Copy constructor.
     */
    Shard(const Shard &other);

    /**
     * Destructor.
     */
    ~Shard();

    /**
     * Assignment operator for deep copy.
     */
    Shard & operator= (const Shard &other);

  private:
    /** Clusterlib root (used to get the Notifyable at runtime) */
    mutable boost::shared_ptr<Root> m_rootSP;

    /** Start of the range */
    HashRange *m_startRange;

    /** End of the range */
    HashRange *m_endRange;

    /** Notifyable key associated with this shard */
    std::string m_notifyableKey;

    /** Priority of this shard */
    int32_t m_priority;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_SHARD_H_ */
