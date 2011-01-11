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

#ifndef	_CL_UNKNOWNHASHRANGE_H_
#define _CL_UNKNOWNHASHRANGE_H_

namespace clusterlib {

/**
 * Represents a HashRange subclass that has no known type.
 */
class UnknownHashRange 
    : public HashRange
{
  public:
    static std::string name();

    virtual std::string getName() const;

    virtual bool isBegin();

    virtual bool isEnd();
    
    virtual bool operator< (const HashRange &other) const;
    
    virtual bool operator== (const HashRange &other) const;

    virtual HashRange & operator= (const HashRange &other);

    virtual json::JSONValue toJSONValue() const;

    virtual void set(const json::JSONValue &jsonValue);

    virtual HashRange &create() const;

    virtual UnknownHashRange & operator++ ();

    /**
     * Virtual destructor.
     */ 
    virtual ~UnknownHashRange() {}

    /**
     * Constructor.
     */
    explicit UnknownHashRange(const json::JSONValue &jsonValue)
        : m_jsonValue(jsonValue)
    {
    }

    /**
     * No argument constructor.
     */
    UnknownHashRange() {}

  private:
    /**
     * The underlying JSONValue.
     */
    json::JSONValue m_jsonValue;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_UNKNOWNHASHRANGE_H_ */
