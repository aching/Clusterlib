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

#ifndef _CL_JSONEXEPTIONS_H_
#define _CL_JSONEXEPTIONS_H_

namespace json {

/**
 * Generic exception for the json namespace.
 */
class Exception 
    : public clusterlib::Exception
{
  public:
    /**
     * Constructor.
     */
    explicit Exception(const std::string &msg) throw()
        : clusterlib::Exception(msg) {}
};

/**
 * Defines the exception of a JSON parsing error.
 */
class JSONParseException 
    : public Exception
{
  public:
    /**
     * Creates an instance of JSONParseException with error message.
     *
     * @param msg the error message.
     */
    explicit JSONParseException(const std::string &msg) throw()
        : Exception(msg) {}
};

/**
 * Defines the exception of a JSON value error. For example, the
 * value is not compatible with its type.
 */
class JSONValueException 
    : public Exception 
{
  public:
    explicit JSONValueException(const std::string &msg) throw()
        : Exception(msg) {}
};

}	/* End of 'json' */

#endif	/* _CL_JSONEXEPTIONS_H_ */

