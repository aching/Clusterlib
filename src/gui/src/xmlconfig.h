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

#ifndef _INCLUDED_XMLCONFIG_H_
#define _INCLUDED_XMLCONFIG_H_

namespace configurator {

typedef std::map<std::string, std::string> Configuration;

class ConfigurationException 
    : public virtual std::exception 
{
  public:

    /**
     * Creates an instance of ConfigurationException with error message.
     * @param message the error message.
     */
    explicit ConfigurationException(const std::string &message);

    /**
     * Gets the error message of this exception.
     * @return the error message.
     */
    virtual const char *what() const throw();
    /**
     * Destroys the instance of ConfigurationException.
     */
    virtual ~ConfigurationException() throw();

  private:
    /**
     * Represents the error message.
     */
    std::string message;
};

class XmlConfig
{
  public:
    static void Parse(const std::string &xmlFile, Configuration *config);
  private:
    XmlConfig();
};

}

#endif
