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

#ifndef	_CL_GROUPIMPL_H_
#define _CL_GROUPIMPL_H_

namespace clusterlib {

/*
 * Implements class Group.
 */
class GroupImpl
    : public virtual Group, 
      public virtual NotifyableImpl
{
  public:
    virtual NameList getNodeNames();

    virtual bool getNodeWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Node> *pNodeSP);

    virtual boost::shared_ptr<Node> getNode(
        const std::string &name,
        AccessType accessType);

    virtual NameList getGroupNames();

    virtual bool getGroupWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<Group> *pGroupSP);

    virtual boost::shared_ptr<Group> getGroup(
        const std::string &name,
        AccessType accessType);

    virtual NameList getDataDistributionNames();

    virtual bool getDataDistributionWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<DataDistribution> *pDataDistributionSP);

    virtual boost::shared_ptr<DataDistribution> getDataDistribution(
        const std::string &name,
        AccessType accessType);

    /*
     * Internal functions not used by outside clients
     */
  public:
    /*
     * Constructor used by the factory.
     */
    GroupImpl(FactoryOps *f,
              const std::string &key,
              const std::string &name,
              boost::shared_ptr<NotifyableImpl> parent)
        : NotifyableImpl(f, key, name, parent) {}

    /*
     * Destructor.
     */
    virtual ~GroupImpl() {};

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

  private:
    /*
     * Make the default constructor private so it cannot be called.
     */
    GroupImpl();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_GROUPIMPL_H_ */
