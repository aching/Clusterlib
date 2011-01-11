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

