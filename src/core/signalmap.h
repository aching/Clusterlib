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

#ifndef _CL_SIGNAL_MAP_H_
#define _CL_SIGNAL_MAP_H_

namespace clusterlib {

/**
 * Manages multiple PredMutexCond objects that are keyed by a string.
 */
class SignalMap
{
  public:
    /** 
     * Add a new PredMutexCond to the map if it does not exist.  The
     * new PredMutexCond memory is owned by the signal map and not the
     * user.  Reference counts are added to the same PredMutexCond if
     * this function is called more than once on the same key.
     *
     * @param key the key in the signal map
     */
    void addRefPredMutexCond(const std::string &key);

    /**
     * Removes a reference for this key.  The client that calls this
     * agrees to not use the PredMutexCond * associated with this key
     * anymore.  If the number of references falls to zero, the
     * PredMutexCond for the key is freed.
     */
    void removeRefPredMutexCond(const std::string &key);
  
    /**
     * Signals threads waiting on waitPredMutexCond().  Must be called
     * on a valid key.
     *
     * @param key the key to signal the predicate on
     * @return true if the key is found, false otherwise
     */
    bool signalPredMutexCond(const std::string &key);
  
    /**
     * Wait on the PredMutexCond that is being tracked from
     * addRefPredMutexCond().
     *
     * @param key to the PredMutexCond
     * @param usecTimeout the amount of usecs to wait until a signal is 
     *        available, -1 means wait forever, 0 means return immediately
     * @return false if the function timed out, true if predicate changed
     *         (always true if it returns and the timeout == 0)
     */
    bool waitUsecsPredMutexCond(const std::string &key, 
                                int64_t usecTimeout);

  private:
    /**
     * Get the mutex.
     */
    Mutex *getSignalMapLock() { return &m_signalMapLock; }

  private:
    /** The map that actually has the PredMutexCond pointers */
    std::map<std::string, PredMutexCond *> m_signalMap;

    /**
     * Mutex that protects m_signalMap
     */
    Mutex m_signalMapLock;
};

}	/* End of 'namespace clusterlib' */
        
#endif /* _CL_SIGNAL_MAP_H_ */

