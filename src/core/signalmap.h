/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#ifndef __SIGNAL_MAP_H__
#define __SIGNAL_MAP_H__

namespace clusterlib {

/**
 * Manages multiple PredMutexCond pointers that are keyed by a string.
 */
class SignalMap {
  public:
    /** 
     * Add a new pointer to the map if it does not exist.  The memory
     * allocated by the pointer is owned by the user.  Reference
     * counts are added to the same PredMutexCond if this function is
     * called more than once on the same key.
     */
    void addRefPredMutexCond(const std::string &key, 
                             PredMutexCond *predMutexCond);

    /**
     * Removes a reference for this key.  If the number of references
     * falls to zero, the PredMutexCond pointer is removed (object
     * allocated memory is not cleaned up).
     */
    void removeRefPredMutexCond(const std::string &key);
  
    /**
     * Signals threads waiting on waitPredMutexCond().  Must be called
     * on a valid key.
     */
    void signalPredMutexCond(const std::string &key);
  
    /**
     * Wait on the PredMutexCond that is being tracked from
     * addRefPredMutexCond().
     */
    void waitPredMutexCond(const std::string &key);

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

};	/* End of 'namespace clusterlib' */
        
#endif /* __SIGNAL_MAP_H__ */

