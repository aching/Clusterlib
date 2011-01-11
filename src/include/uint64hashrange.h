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

#ifndef	_CL_UINT64HASHRANGE_H_
#define _CL_UINT64HASHRANGE_H_

namespace clusterlib {

/**
 * Represents a HashRange subclass built around uint64_t.
 */
class Uint64HashRange 
    : public HashRange
{
  public:
    virtual std::string getName() const
    {
        return "uint_64";
    }

    virtual bool isBegin()
    {
        if (m_hashPoint == std::numeric_limits<uint64_t>::min()) {
            return true;
        }
        else {
            return false;
        }
    }

    virtual bool isEnd() 
    {
        if (m_hashPoint == std::numeric_limits<uint64_t>::max()) {
            return true;
        }
        else {
            return false;
        }
    }
    
    virtual bool operator< (const HashRange &other) const
    {
        const Uint64HashRange &otherHashRange = 
            dynamic_cast<const Uint64HashRange &>(other);
        return (getHashPoint() < (otherHashRange.getHashPoint()));
    }
    
    virtual bool operator== (const HashRange &other) const
    {
        const Uint64HashRange &otherHashRange = 
            dynamic_cast<const Uint64HashRange &>(other);
        return (getHashPoint() == (otherHashRange.getHashPoint()));
    }

    virtual HashRange & operator= (const HashRange &other) 
    {
        const Uint64HashRange &otherHashRange = 
            dynamic_cast<const Uint64HashRange &>(other);
        m_hashPoint = otherHashRange.getHashPoint();
        return *this;
    }

    virtual json::JSONValue toJSONValue() const
    {
        return json::JSONValue::JSONUInteger(m_hashPoint);
    }

    virtual void set(const json::JSONValue &jsonValue )
    {
        m_hashPoint = jsonValue.get<json::JSONValue::JSONUInteger>();
    }

    virtual HashRange &create() const
    {
        return *(dynamic_cast<HashRange *>(new Uint64HashRange()));
    }

    virtual Uint64HashRange & operator++ ()
    {
        ++m_hashPoint;
        return *this;
    }

    /**
     * Virtual destructor.
     */ 
    virtual ~Uint64HashRange() {}

    /**
     * Constructor.
     */
    explicit Uint64HashRange(uint64_t hashPoint)
        : m_hashPoint(hashPoint)
    {
    }

    Uint64HashRange()
        : m_hashPoint(std::numeric_limits<uint64_t>::min())
    {
    }

  public:
    const uint64_t &getHashPoint() const
    {
        return m_hashPoint;
    }

  private:
    /**
     * The underlying hashing point
     */
    uint64_t m_hashPoint;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_UINT64HASHRANGE_H_ */
