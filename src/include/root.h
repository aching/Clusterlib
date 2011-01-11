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

#ifndef	_CL_ROOT_H_
#define _CL_ROOT_H_

namespace clusterlib {

/**
 * Base Clusterlib object.  It manages Applications.
 */
class Root
    : public virtual Notifyable
{
  public:
    /**
     * Get a list of names of all applications.
     * 
     * @return a copy of the list of all applications
     */
    virtual NameList getApplicationNames() = 0;

    /**
     * Get the name Application.
     *
     * @param name Name of the Application to create
     * @param accessType Mode of access
     * @param msecTimeout Amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pApplicationSP Pointer to that pApplicationSP if it exists, 
     *                  otherwise NULL.
     * @return True if the operation finished before the timeout
     * @throw Exception if Notifyable is the root or application
     */
    virtual bool getApplicationWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Application> *pApplicationSP) = 0;

    /**
     * Get the named Application (no timeout).
     * 
     * @param name Name of the Application to get
     * @param accessType Mode of access
     * @return NULL if the named Application does not exist
     * @throw Exception only if tried to create and couldn't create
     */
    virtual boost::shared_ptr<Application> getApplication(
        const std::string &name,
        AccessType accessType) = 0;

    /*
     * Destructor.
     */
    virtual ~Root() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_ROOT_H_ */
