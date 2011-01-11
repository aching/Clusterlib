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

#ifndef	_CL_NODEIMPL_H_
#define _CL_NODEIMPL_H_

namespace clusterlib {

/**
 * Implementation of class Node.
 */
class NodeImpl
    : public virtual Node, 
      public virtual NotifyableImpl
{
  public:
    virtual CachedProcessSlotInfo &cachedProcessSlotInfo();

    virtual NameList getProcessSlotNames();

    virtual bool getProcessSlotWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<ProcessSlot> *pProcessSlotSP);

    virtual boost::shared_ptr<ProcessSlot> getProcessSlot(
        const std::string &name,
        AccessType accessType);

    /*
     * Internal functions not used by outside clients
     */    
  public:
    /**
     * Constructor.
     */
    NodeImpl(FactoryOps *fp,
             const std::string &key,
             const std::string &name,
             boost::shared_ptr<NotifyableImpl> group)
        : NotifyableImpl(fp, key, name, group),
          m_cachedProcessSlotInfo(this) {}

    /**
     * Virtual destructor.
     */
    virtual ~NodeImpl();

    virtual NotifyableList getChildrenNotifyables();

    virtual void initializeCachedRepresentation();

    /**
     * Create the process slot info JSONObject key
     *
     * @param nodeKey the node key
     * @return the generated process slot info JSONObject key
     */
    static std::string createProcessSlotInfoJSONObjectKey(
        const std::string &nodeKey);

  private:
    /*
     * Do not call the default constructor
     */
    NodeImpl();

  private:
    /**
     * The cached process slot information
     */
    CachedProcessSlotInfoImpl m_cachedProcessSlotInfo;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NODEIMPL_H_ */
