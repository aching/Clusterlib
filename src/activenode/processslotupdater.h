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

#ifndef	_CL_PROCESSSLOTUPDATER_H_
#define _CL_PROCESSSLOTUPDATER_H_

namespace activenode {

/**
 * Makes sure that the process slot desired state is keeping up with
 * the current state.
 */
class ProcessSlotUpdater 
    : public clusterlib::Periodic 
{
  public:
    /**
     * Decide on an action to take.
     */
    enum UpdateAction {
        NONE = 0, ///< Do nothing
        START, ///< Start the new process
        KILL, ///< Kill the current process
    };

    /**
     * Constructor.
     *
     * @param msecsFrequency How often to run this check
     * @param processSlotSP ProcessSlot to set the current state.
     */
    ProcessSlotUpdater(
        int64_t msecsFrequency, 
        const boost::shared_ptr<clusterlib::ProcessSlot> &processSlotSP);

    /**
     * Virtual destructor.
     */
    virtual ~ProcessSlotUpdater();

    /**
     * Periodic function to run.
     */
    virtual void run();
    
    /**
     * Helper function for run to figure out what to do.
     *
     * @param processSlotSP ProcessSlot to set the current state.
     * @return A course of action to be taken.
     */
    UpdateAction determineAction(
        const boost::shared_ptr<clusterlib::ProcessSlot> &processSlotSP);
};

}

#endif	/* !_CL_PROCESSSLOTUPDATER_H_ */
