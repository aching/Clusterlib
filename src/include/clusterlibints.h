/*
 * clusterlibints.h --
 *
 * Definition of ClusterlibInts.
 *
 * $Header$
 * $Revision$
 * $Date$
 */

#ifndef	_CL_CLUSTERLIBINTS_H_
#define	_CL_CLUSTERLIBINTS_H_

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
class ClusterlibInts
{
  public:
    /* 
     * All indices use for parsing ZK node names
     */
    static const int32_t CLUSTERLIB_INDEX;
    static const int32_t VERSION_NAME_INDEX;
    static const int32_t ROOT_INDEX;
    static const int32_t APP_INDEX;
    static const int32_t APP_NAME_INDEX;

    /*
     * Number of components in a Root key
     */
    static const int32_t ROOT_COMPONENTS_COUNT;

    /*
     * Number of components in an Application key
     */
    static const int32_t APP_COMPONENTS_COUNT;

    /*
     * Minimum components necessary to represent each respective key
     */
    static const int32_t DIST_COMPONENTS_MIN_COUNT;
    static const int32_t PROP_COMPONENTS_MIN_COUNT;
    static const int32_t QUEUE_COMPONENTS_MIN_COUNT;
    static const int32_t GROUP_COMPONENTS_MIN_COUNT;
    static const int32_t NODE_COMPONENTS_MIN_COUNT;
    static const int32_t PROCESSSLOT_COMPONENTS_MIN_COUNT;

    /**
     * This is known to be 10 in Zookeeper 3.1.1.  After JIRA issue
     * ZOOKEEPER-616 is fixed, it will no longer be needed.
     */
    static const size_t SEQUENCE_NUMBER_SIZE;

    /**
     * This is the initial Zookeeper version for objects that cache data.
     */
    static const int32_t INITIAL_ZK_VERSION;

    /**
     * This is the Zookeeper version for objects that were deleted.
     */
    static const int32_t DELETED_ZK_VERSION;

    /**
     * When the msecs has not been set
     */
    static const int64_t MSECS_NOT_AVAILABLE;

  private:
    /**
     * No constructing.
     */
    ClusterlibInts();
};

}	/* End of 'namespace clusterlib' */

#endif	/* !_CL_CLUSTERLIBINTS_H_ */
