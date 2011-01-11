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

#ifndef	_CL_CLIFORMAT_H_
#define _CL_CLIFORMAT_H_

/**
 * Manages output to the console.
 */
class CliFormat
{
  public:
    /** 
     * Print out a bool data attribute. 
     *
     * @param attribute the attribute name
     * @param data the attribute data
     */
    static void attributeOut(std::string attribute, bool data) 
    {
        std::cout << " " << attribute << " : " 
                  << ((data == true) ? "true" : "false") << std::endl;
    }

    /** 
     * Print out a numeric data attribute. 
     *
     * @param attribute the attribute name
     * @param data the attribute data
     */
    static void attributeOut(std::string attribute, int32_t data) 
    {
        std::cout << " " << attribute << " : " << data << std::endl;
    }

    /** 
     * Print out a const char * data attribute. 
     *
     * @param attribute the attribute name
     * @param data the attribute data
     */
    static void attributeOut(std::string attribute, const char *data) 
    {
        std::cout << " " << attribute << " : " << data << std::endl;
    }

    /** 
     * Print out a string data attribute. 
     *
     * @param attribute the attribute name
     * @param data the attribute data
     */
    static void attributeOut(std::string attribute, std::string data) 
    {
        std::cout << " " << attribute << " : " << data << std::endl;
    }

    /** 
     * Print out a vector of string for an attribute
     *
     * @param attribute the attribute name
     * @param dataVec the attribute data in a vector
     */
    static void attributeOut(std::string attribute, 
                             std::vector<std::string> dataVec) 
    {
        std::cout << " " << attribute << " : ";
        for (size_t i = 0; i < dataVec.size(); i++)
            std::cout << dataVec[i] <<  " ";
        std::cout << std::endl;
    }

  private:
    /**
     * Constructor.  Shouldn't be instantiated.
     */
    CliFormat();
};

#endif	/* !_CL_CLIFORMAT_H_ */
