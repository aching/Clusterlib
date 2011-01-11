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

#ifndef	_CL_APPLICATIONIMPL_H_
#define _CL_APPLICATIONIMPL_H_

namespace clusterlib {

/**
 * Implementation of class Application.
 */
class ApplicationImpl
    : public virtual Application, 
      public virtual GroupImpl
{
  public:
    virtual bool getMyGroupWaitMsecs(
        int64_t msecTimeout,
        boost::shared_ptr<Group> *pGroupSP)
    {
        throw InvalidMethodException(
            "Application cannot be a part of a Group!");
    }

    /*
     * Internal functions not used by outside clients
     */
  public:
    /**
     * Constructor used by Factory.
     */
    ApplicationImpl(FactoryOps *fp,
                    const std::string &key, 
                    const std::string &name, 
                    boost::shared_ptr<NotifyableImpl> root)
        : NotifyableImpl(fp, key, name, root),
          GroupImpl(fp, key, name, root) {}

    /**
     * Destructor.
     */
    virtual ~ApplicationImpl() {};

    virtual void initializeCachedRepresentation();

  private:
    /**
     * Do not call the default constructor
     */
    ApplicationImpl();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_APPLICATIONIMPL_H_ */
