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

#ifndef	_CL_ROOTIMPL_H_
#define _CL_ROOTIMPL_H_

namespace clusterlib {

/**
 * Implementation of class Root.
 */
class RootImpl
    : public virtual Root, 
      public virtual NotifyableImpl
{
  public:
    virtual NameList getApplicationNames();

    virtual bool getApplicationWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Application> *pApplicationSP);

    virtual boost::shared_ptr<Application> getApplication(
        const std::string &name,
        AccessType accessType);

    virtual boost::shared_ptr<Notifyable> getMyParent() const
    {
        throw InvalidMethodException("RootImpl does not have a parent");
    }
    
    virtual boost::shared_ptr<Application> getMyApplication(
        int64_t msecTimeout)
    {
        throw InvalidMethodException(
            "RootImpl not is part of an application");
    }

    virtual boost::shared_ptr<Group> getMyGroup(int64_t msecTimeout)
    {
        throw InvalidMethodException("RootImpl is not part of a group");
    }

    virtual bool getPropertyListWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<PropertyList> *pPropertyListSP)
    {
        throw InvalidMethodException("RootImpl cannot have PropertyList");
    }

    virtual void remove(bool removeChildren = false) 
    {
        throw InvalidMethodException("RootImpl cannot be removed");
    }

    virtual void releaseRef()
    {
        throw InvalidMethodException("RootImpl cannot be released");
    }

    /*
     * Internal functions not used by outside clients
     */
  public:
    /*
     * Constructor used by Factory.
     */
    RootImpl(FactoryOps *fp,
             const std::string &key,
             const std::string &name)
        : NotifyableImpl(fp, key, name, boost::shared_ptr<NotifyableImpl>()) {}

    virtual ~RootImpl() {};

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

    virtual void removeRepositoryEntries() 
    {
        throw InvalidMethodException("RootImpl cannot remove its"
                                     " repository entries");
    }

  private:
    /*
     * Do not call the default constructor.
     */
    RootImpl();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_ROOTIMPL_H_ */
