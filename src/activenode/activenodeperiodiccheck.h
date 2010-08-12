/*
 * activenodeperiodiccheck.h --
 * 
 * $Header:$
 * $Revision$
 * $Date$
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
