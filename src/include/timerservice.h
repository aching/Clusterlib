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

#ifndef	_CL_TIMERSERVICE_H_
#define	_CL_TIMERSERVICE_H_

namespace clusterlib {

/**
 * Global timing methods.
 */
class TimerService
{
  public:
    /**
     * Get the time in a human-readable format based on the msecs from
     * the epoch.
     *
     * @param msecs the number of msecs from the epoch
     * @return a human-readable string of the time
     */
    static std::string getMsecsTimeString(int64_t msecs)
    {
        time_t secsFromEpoch = msecs / 1000;
        struct tm *timeInfo;
        
        timeInfo = localtime(&secsFromEpoch);
        std::string ret(asctime(timeInfo));
        if (!ret.empty()) {
            ret.resize(ret.size() - 1);
        }
        return ret;
    }

    /**
     * Get the time in a human-readable format at this instant.
     *
     * @return a human-readable string of the current time
     */
    static std::string getMsecsTimeString()
    {
        return getMsecsTimeString(getCurrentTimeMsecs());
    }

    /**
     * Convenience function -- return the current time in msec
     * from the unix epoch.
     *
     * @return current time in milliseconds
     */
    static int64_t getCurrentTimeMsecs()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_sec * 1000LL + now.tv_usec / 1000;
    }

    /**
     * Convenience function -- return the current time in usec
     * from the unix epoch.
     *
     * @return current time in useconds
     */
    static int64_t getCurrentTimeUsecs()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_sec * 1000000LL + now.tv_usec;
    }

    /**
     * Is the current time less than the current time, equal to the
     * current time (given rounding errors) or greater than the
     * current time. 
     *
     * @param usecs the amount of milliseconds to compare
     * @return -1 if current time is less than usecs
     *         0  if current time is equal to usecs
     *         1  if current time is greater than current time
     */
    static int32_t compareTimeUsecs(int64_t usecs)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if ((now.tv_sec * 1000000LL + now.tv_usec) < usecs) {
            return -1;
        }
        if ((now.tv_sec * 1000000LL + now.tv_usec) == usecs) {
            return 0;
        }
        else {
            return 1;
        }
    }
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_TIMERSERVICE_H_ */
