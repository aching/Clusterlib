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

#ifndef	_CL_ACTIVENODEPERIODICCHECK_H_
#define _CL_ACTIVENODEPERIODICCHECK_H_

namespace activenode {

/**
 * Updates the current state with node healthiness.
 */
class ActiveNodePeriodicCheck 
    : public clusterlib::Periodic 
{
  public:
    /**
     * Constructor.
     *
     * @param msecsFrequency How often to run this check
     * @param nodeSP Node to set the current state.
     * @param predMutexCond Synchronize on this object.
     */
    ActiveNodePeriodicCheck(
        int64_t msecsFrequency, 
        const boost::shared_ptr<clusterlib::Node> &nodeSP,
        clusterlib::PredMutexCond &predMutexCond);

    /**
     * Virtual destructor.
     */
    virtual ~ActiveNodePeriodicCheck();

    /**
     * Periodic function to run.
     */
    virtual void run();

  private:
    /**
     * Reference to the shutdown PredMutexCond.
     */
    clusterlib::PredMutexCond &m_predMutexCond;
};

}

#endif	/* !_CL_ACTIVENODEPERIODICCHECK_H_ */
