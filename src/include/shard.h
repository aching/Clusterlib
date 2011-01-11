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

#ifndef _CL_SHARD_H_
#define _CL_SHARD_H_

namespace clusterlib {

/**
 * Bounded by HashRanges.
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
