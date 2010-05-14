/*
 * processslotupdater.h --
 * 
 * $Header:$
 * $Revision$
 * $Date$
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
        STOP, ///< Stop the current process
    };

    /**
     * Constructor.
     *
     * @param msecsFrequency How often to run this check
     * @param notifyable The notifyable to set the current state.
     */
    ProcessSlotUpdater(int64_t msecsFrequency, 
                            clusterlib::Notifyable *notifyable);

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
     * @param notifyable The Notifyable to check states.
     * @return A course of action to be taken.
     */
    UpdateAction determineAction(clusterlib::Notifyable &notifyable);
};

}

#endif	/* !_CL_PROCESSSLOTUPDATER_H_ */
