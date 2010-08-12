/*
 * safenotifyablemap.h --
 *
 * Implementation of SafeNotifyableMap class.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_SAFENOTIFYABLEMAP_H_
#define	_CL_SAFENOTIFYABLEMAP_H_

namespace clusterlib {

 /**
  * Special caching structure for clusterlib notifyables.
  */
class SafeNotifyableMap
{
  public:
    /**
     * Constructor.
     */
    SafeNotifyableMap() {};

    /**
     * Try to find the notifyable (thread-safe if holding the mutex).
     *
     * @param notifyableKey the key of the notifyable
     * @return a pointer to the notifyable or NULL if not found
     */
    boost::shared_ptr<NotifyableImpl> getNotifyable(
        const std::string &notifyableKey);
    
    /**
     * Insert the notifyable into the map if it is unique (thread-safe
     * if holding the mutex).  The map key is the notifyable's key.
     * At this point, the memory of the notifyable is owned by
     * SafeNotifyableMap and will be removed during destruction.
     *
     * @param notifyableSP Pointer to the notifyable to insert
     */
    void uniqueInsert(const boost::shared_ptr<NotifyableImpl> &notifyableSP);

    /**
     * Remove the notifyable from the map (thread-safe if holding the
     * mutex).
     */
    void erase(const boost::shared_ptr<NotifyableImpl> &notifyableSP);

    /**
     * Get the lock that protects this object.
     *
     * @return a reference to the mutex
     */
    const Mutex &getLock() const;

    /**
     * Destructor.  Frees all memory for every NotifyableImpl * in the map.
     */
    ~SafeNotifyableMap();

  private:
    /**
     * No copy constructor.
     */
    SafeNotifyableMap(const SafeNotifyableMap &other);

    /**
     * No assignment.
     */
    SafeNotifyableMap & operator=(const SafeNotifyableMap &other);

  private:
    /** 
     * The map containing the pointers to the allocated Notifyable
     * objects.
     */
    std::map<std::string, boost::shared_ptr<NotifyableImpl> > m_ntpMap;
    
    /**
     * Lock that protects m_ntpMap.
     */
    Mutex m_ntpMapLock;
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_SAFENOTIFYABLEMAP_H_ */

