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

#ifndef	_CL_HASHRANGE_H_
#define _CL_HASHRANGE_H_

namespace clusterlib {

/**
 * Subclass to create a hash range.  A Shard in a DataDistribution
 * uses HashRanges to specify the bounadaries of a Shard.
 */
class HashRange 
{
  public:
    /**
     * Get the name of this hash range type.
     *
     * @return the name
     */
    virtual std::string getName() const = 0;

    /**
     * Is this point the beginning of the range?
     *
     * @return True if beginning of the range.
     */
    virtual bool isBegin() = 0;

    /**
     * Is this point the end of the range?
     *
     * @return True if end of the range.
     */
    virtual bool isEnd() = 0;

    /**
     * Subclasses need to implement this operator.
     *
     * @param other Another HashRange to compare.
     * @return True if expression is satisfied.
     */
    virtual bool operator< (const HashRange &other) const = 0;

    /**
     * Subclasses need to implement this operator.
     *
     * @param other Another HashRange to compare.
     * @return True if expression is satisfied.
     */
    virtual bool operator== (const HashRange &other) const = 0;

    /**
     * Default implementation, can be overriden by subclasses if desired.
     *
     * @param other Another HashRange to compare.
     * @return True if expression is satisfied.
     */
    virtual bool operator!= (const HashRange &other) const 
    {
        return !(*this == other);
    }

    /**
     * Default implementation, can be overriden by subclasses if desired.
     *
     * @param other Another HashRange to compare.
     * @return True if expression is satisfied.
     */
    virtual bool operator<= (const HashRange &other) const 
    {
        return ((*this < other) || (*this == other));
    }

    /**
     * Default implementation, can be overriden by subclasses if desired.
     *
     * @param other Another HashRange to compare.
     * @return True if expression is satisfied.
     */
    virtual bool operator> (const HashRange &other) const
    {
        return (other < *this);
    }

    /**
     * Default implementation, can be overriden by subclasses if desired.
     *
     * @param other Another HashRange to compare.
     * @return True if expression is satisfied.
     */
    virtual bool operator>= (const HashRange &other) const
    {
        return ((other < *this) || (other == *this));
    }

    /**
     * Useful for sending data to streams.
     *
     * @param stream The output stream.
     * @param hashRange The HashRange to send to the stream.
     * @return The stream.
     */
    friend std::ostream & operator<< (std::ostream &stream, 
                                      HashRange &hashRange)
    {
        stream << json::JSONCodec::encode(hashRange.toJSONValue());

        return stream;
    }
    
    /**
     * Subclasses need to implement this operator.
     *
     * @param other Another HashRange to compare.
     * @return True if expression is satisfied.
     */
    virtual HashRange & operator= (const HashRange &other) = 0;

    /**
     * Convert this HashRange to a JSONValue.
     *
     * @return This HashRange as a JSONValue.
     */
    virtual json::JSONValue toJSONValue() const = 0;

    /**
     * Import data into this object from a JSONValue.
     *
     * @param jsonValue The input data to convert to this type.
     */
    virtual void set(const json::JSONValue &jsonValue) = 0;

    /**
     * Create an heap-allocated object of this type.  The caller will
     * be responsible for freeing it.
     *
     * @return A new object of this type.
     */
    virtual HashRange &create() const = 0;

    /**
     * Subclasses should implement the prefix ++ operator.  This is
     * useful for isCovered().
     *
     * @return This object.
     */
    virtual HashRange & operator++ () = 0;

    /**
     * Virtual destructor.
     */ 
    virtual ~HashRange() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_HASHRANGE_H_ */
