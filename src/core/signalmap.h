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
 * Manages multiple PredMutexCond objects that are keyed by a string.
 */
class SignalMap {
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
     * @param timeout how long to wait until an signal becomes available, 
     *        in milliseconds; if <code>0</code> then wait forever;
     * @return false if the function timed out, true if predicate changed
     *         (always true if it returns and the timeout == 0)
     */
    bool waitPredMutexCond(const std::string &key, 
                           const uint64_t timeout = 0);

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

