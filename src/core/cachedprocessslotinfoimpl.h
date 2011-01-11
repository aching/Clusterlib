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

#ifndef	_CL_CACHEDPROCESSSLOTINFOIMPL_H_
#define _CL_CACHEDPROCESSSLOTINFOIMPl_H_

namespace clusterlib {

/**
 * Implementation of class CachedProcessSlotInfo
 */
class CachedProcessSlotInfoImpl
    : public virtual CachedDataImpl,
      public virtual CachedProcessSlotInfo
{
  public:
    virtual int32_t publish(bool unconditional = false);

    virtual void loadDataFromRepository(bool setWatchesOnly);

  public:
    enum ProcessSlotInfoArrEnum {
        ENABLE = 0,
        MAX_PROCESSES,
        MAX_SIZE
    };

    virtual bool getEnable();
        
    virtual void setEnable(bool enable);

    virtual int32_t getMaxProcessSlots();
    
    virtual void setMaxProcessSlots(int32_t maxProcessSlots);

    /**
     * Constructor.
     */
    explicit CachedProcessSlotInfoImpl(NotifyableImpl *notifyable);

    /**
     * Destructor.
     */
    virtual ~CachedProcessSlotInfoImpl() {}
    
  private:
    /**
     * The cached process slot information
     * format is [bool enable, integer maxProcesses].
     */
    ::json::JSONValue::JSONArray m_processSlotInfoArr;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSSLOTINFOIMPL_H_ */
