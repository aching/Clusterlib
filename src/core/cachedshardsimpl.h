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

#ifndef	_CL_CACHEDSHARDSIMPL_H_
#define _CL_CACHEDSHARDSIMPL_H_

namespace clusterlib {

/**
 * The object stored in the interval tree with each notifyable.
 */
class ShardTreeData 
{
  public:
    ShardTreeData()
        : m_priority(-1) {}

    ShardTreeData(int32_t priority, const std::string &ntpKey) 
        : m_priority(priority),
          m_notifyableKey(ntpKey) {}

    int32_t getPriority() const { return m_priority; }

    const std::string &getNotifyableKey() const { return m_notifyableKey; }

    bool operator< (const ShardTreeData &rhs) const
    {
        if (getPriority() < rhs.getPriority()) {
            return true;
        }
        else if (getPriority() == rhs.getPriority()) {
            if (getNotifyableKey() < rhs.getNotifyableKey()) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }

    bool operator==(const ShardTreeData &rhs) const
    {
        if ((getPriority() == rhs.getPriority()) &&
            (getNotifyableKey() == rhs.getNotifyableKey())) {
            return true;
        }
        else {
            return false;
        }
    }

    bool operator> (const ShardTreeData &rhs) const
    {
        if (!(*this < rhs) && (!(*this == rhs))) {
            return true;
        }
        else {
            return false;
        }
    }

    /**
     * Print out useful data from <<.
     */
    friend std::ostream & operator<< (std::ostream &stream, 
                                      const ShardTreeData &other)
    {
        stream << "ShardTreeData(priority=" << other.getPriority()
               << ",key=" << other.getNotifyableKey() << ")";

        return stream;
    }

  private:
    /** The priority of this shard */
    int32_t m_priority;

    /** The Notifyable key */
    std::string m_notifyableKey;
};

/**
 * Implements class CachedShards.
 */
class CachedShardsImpl
    : public virtual CachedDataImpl,
      public virtual CachedShards
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    virtual std::string getHashRangeName();

    virtual NotifyableList getNotifyables(const HashRange &hashRange);

    virtual uint32_t getCount();

    virtual bool isCovered();

    virtual void insert(const HashRange &start,
                        const HashRange &end,
                        const boost::shared_ptr<Notifyable> &notifyableSP,
                        int32_t priority = 0);
    
    virtual std::vector<Shard> getAllShards(
        const boost::shared_ptr<Notifyable> &notifyableSP = 
        boost::shared_ptr<Notifyable>(),
        int32_t priority = -1);

    virtual bool remove(Shard &shard);

    virtual void clear();

    /**
     * Constructor.
     */
    explicit CachedShardsImpl(NotifyableImpl *notifyable);

    /**
     * Destructor.
     */
    virtual ~CachedShardsImpl();

  private:
    /**
     * Marshal the shards into a JSONValue for publishing.
     *
     * @return An JSON array of the shards.
     */
    json::JSONValue::JSONArray marshalShards();

    /**
     * Unmarshal a stringified sequence of shards into this
     * object. The shards are stored as a JSONArray of JSONArray
     * objects (begin, end, notifyablekey, priority), with an initial
     * JSONString at the front of the JSONArray to denote the
     * HashRange.
     *
     * @param encodedJsonArr The encoded JSON array of shards (each shard 
     *        is a JSON array as well)
     */
    void unmarshalShards(const std::string &encodedJsonArr);

    /**
     * Throw an InvalidMethodException() is the HashRange is
     * UnknownHashRange.
     */
    void throwIfUnknownHashRange();
    
  private:
    /**
     * Stores all the Shard objects for HashRange types that are not
     * UnknownHashRange.  Must be a pointer since the HashRange may
     * change.
     */
    IntervalTree<HashRange &, ShardTreeData> *m_shardTree;

    /**
     * The number of shards in the tree.
     */
    int32_t m_shardTreeCount;

    /**
     * Unsorted storage for UnknownHashRange Shard objects.
     */
    std::vector<Shard> m_unknownShardArr;

    /**
     * Registered HashRange (set implicitly when read, or used).  Can
     * only be set when Shard objects are loaded or during an insert()
     * when there are no Shard objects.
     */
    HashRange *m_hashRange;
};

}

#endif	/* !_CL_CACHEDSHARDSIMPL_H_ */
