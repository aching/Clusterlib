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
     *
     * @param key the key in the signal map
     * @param predMutexCond the pointer to the PredMutexCond that will be 
     *        stored in the map
     * @return true if actually added, false if just incremented
     */
    bool addRefPredMutexCond(const std::string &key, 
                             PredMutexCond *predMutexCond);

    /**
     * Removes a reference for this key.  If the number of references
     * falls to zero, the PredMutexCond pointer is removed (object
     * allocated memory (original predMutexCond) is not cleaned up).
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
     * @param timeout how long to wait until an signal becomes available, 
     *        in milliseconds; if <code>0</code> then wait forever;
     * @return false if the function timed out, true if predicate changed
     *         (always true if it returns and the timeout == 0)
     */
    bool waitPredMutexCond(const std::string &key, const uint64_t timeout = 0);

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

