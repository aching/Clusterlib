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

#ifndef	_CL_CLNUMERIC_H_
#define	_CL_CLNUMERIC_H_

/*
 * Defines the numeric constants in Clusterlib.
 */

namespace clusterlib {

/**
 * Desired access to a Notifyable.
 *
 * This determines what type of lock will be acquired.  CACHED_ONLY
 * does not get a distributed lock.  LOAD_FROM_REPOSITORY will get a
 * DIST_LOCK_SHARED on its parent.  CREATE_IF_NOT_FOUND will get a
 * DIST_LOCK_EXCL of its parent.
 */
enum AccessType {
    CACHED_ONLY = 0, ///< Only try to get the data in the cache, not repository
    LOAD_FROM_REPOSITORY, ///< Only check the cache and the repository
    CREATE_IF_NOT_FOUND ///< Create if not found in the cache or repository
};

/**
 * Helper function to get the AccessType as a string.
 *
 * @return AccessType as a string.
 */
std::string accessTypeToString(AccessType accessType);

/**
 * Desired type of distributed lock.
 * 
 * DIST_LOCK_SHARED can share access with multiple readers simultaneously
 * DIST_LOCK_EXCL is an exclusive lock
 * The locking policy is fair, all locks are granted in order of requests 
 * although readers will be granted simultaneous access when possible.
 */
enum DistributedLockType {
    DIST_LOCK_INIT = 0, ///< Initialized value (invalid for actual use)
    DIST_LOCK_SHARED, ///< Shared lock (typically used for reading)
    DIST_LOCK_EXCL ///< Exclusive lock (typically used for reading + writing)
};

/**
 * Helper function to get the DistributedLockType as a string.
 *
 * @return DistributedLockType as a string.
 */
std::string distributedLockTypeToString(
    DistributedLockType distributedLockType);

/**
 * Helper function to get the DistributedLockType from a string
 *
 * @return DistributedLockType from a string.
 */
DistributedLockType distributedLockTypeFromString(
    const std::string &distributedLockTypeString);

/**
 * Class containing static variables for all string constants.
 */
class CLNumeric
{
  public:
    /**
     * This is the initial Zookeeper version for objects that cache data.
     */
    static const int32_t INITIAL_ZK_VERSION;
    /**
     * This is the Zookeeper version for objects that were deleted.
     */
    static const int32_t DELETED_ZK_VERSION;

  private:
    /**
     * No constructing.
     */
    CLNumeric();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CLNUMERIC_H_ */
