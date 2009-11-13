/*
 * timerservice.h --
 *
 * Definition and implementation of TimerService class.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_TIMERSERVICE_H_
#define	_TIMERSERVICE_H_

namespace clusterlib
{

/**
 * This class provides a static timer.
 */
class TimerService {
  public:
    /**
     * Convenience function -- return the current time in ms
     * from the unix epoch.
     *
     * @return current time in milliseconds
     */
    static int64_t getCurrentTimeMillis()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_sec * 1000LL + now.tv_usec / 1000;
    }

};

};	/* End of 'namespace clusterlib' */

#endif	/* !_TIMERSERVICE_H_ */
