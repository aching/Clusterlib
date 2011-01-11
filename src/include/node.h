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

#ifndef	_CL_NODE_H_
#define _CL_NODE_H_

namespace clusterlib {

/**
 * Typically represents a physical node.
 */
class Node
    : public virtual Notifyable
{
  public:
    /**
     * Used to access the health of the current state.
     */
    static const std::string HEALTH_KEY;

    /**
     * Used to denote the health: good (current and desired state)
     */
    static const std::string HEALTH_GOOD_VALUE;

    /**
     * Used to denote the health: bad (current and desired state)
     */
    static const std::string HEALTH_BAD_VALUE;

    /**
     * Used to access the health set time of the current state.
     */
    static const std::string HEALTH_SET_MSECS_KEY;

    /**
     * Used to access the health set time as a date of the current state.
     */
    static const std::string HEALTH_SET_MSECS_AS_DATE_KEY;

    /**
     * Used to shutdown an active node by setting in a desired state.
     */
    static const std::string ACTIVENODE_SHUTDOWN;

    /**
     * Access the process slot info cached object
     *
     * @return A reference to the cached process slot info.
     */
    virtual CachedProcessSlotInfo &cachedProcessSlotInfo() = 0;

    /** 
     * Get a list of names of all process slots
     * 
     * @return a copy of the list of all process slots.
     */
    virtual NameList getProcessSlotNames() = 0;
 
    /**
     * Get the name ProcessSlot.
     *
     * @param name Name of the ProcessSlot to create
     * @param accessType Mode of access
     * @param msecTimeout Amount of msecs to wait until giving up, 
     *        -1 means wait forever, 0 means return immediately
     * @param pProcessSlotSP Pointer to that pProcessSlotSP if it exists, 
     *        otherwise NULL.
     * @return True if the operation finished before the timeout
     */
    virtual bool getProcessSlotWaitMsecs(
        const std::string &name,
        AccessType accessType,
        int64_t msecTimeout,
        boost::shared_ptr<ProcessSlot> *pProcessSlotSP) = 0;

    /**
     * Get the named ProcessSlot (no timeout).
     * 
     * @param name Name of the ProcessSlot to get
     * @param accessType Mode of access
     * @return NULL if the named ProcessSlot does not exist
     * @throw Exception only if tried to create and couldn't create
     */
    virtual boost::shared_ptr<ProcessSlot> getProcessSlot(
        const std::string &name,
        AccessType accessType) = 0;

    /**
     * Destructor.
     */
    virtual ~Node() {}
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_NODE_H_ */
