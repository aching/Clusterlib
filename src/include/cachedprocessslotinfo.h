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

#ifndef	_CL_CACHEDPROCESSSLOTINFO_H_
#define _CL_CACHEDPROCESSSLOTINFO_H_

namespace clusterlib {

/**
 *  Represents cached data of a node's process slot information.
 */
class CachedProcessSlotInfo
    : public virtual CachedData
{
  public:
    /**
     * Are the process slots enabled?
     * 
     * @return true Return true if the process slots are enabled, 
     *         false otherwise.
     */
    virtual bool getEnable() = 0;
        
    /**
     * Set whether the procecess slots are enabled.
     *
     * @param enable If true, enable the process slots.  Otherwise, disable 
     *        the process slots.
     */
    virtual void setEnable(bool enable) = 0;

    /**
     * How many process slots are allocated?
     *
     * @return Returns the number of maximum process slots .
     */
    virtual int32_t getMaxProcessSlots() = 0;
    
    /**
     *  Set the number of maximum process slots.
     *
     * @param maxProcessSlots The number of maximum process slots allowed for
     *        this notifyable.
     */
    virtual void setMaxProcessSlots(int32_t maxProcessSlots) = 0;

    /**
     * Destructor.
     */
    virtual ~CachedProcessSlotInfo() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CACHEDPROCESSSLOTINFO_H_ */
