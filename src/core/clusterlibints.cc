/*
 * clusterlibints.cc --
 *
 * Implementation of ClusterlibInts.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlib.h"

using namespace std;

namespace clusterlib {

string
accessTypeToString(AccessType accessType)
{
    switch (accessType) {
        case CACHED_ONLY:
            return "CACHED_ONLY";
        case LOAD_FROM_REPOSITORY:
            return "LOAD_FROM_REPOSITORY";
        case CREATE_IF_NOT_FOUND:
            return "CREATE_IF_NOT_FOUND";
        default:
            return "unknown access type";
    }
}

string
distributedLockTypeToString(DistributedLockType distributedLockType)
{
    switch (distributedLockType) {
        case DIST_LOCK_INIT:
            return "DIST_LOCK_INIT";
        case DIST_LOCK_SHARED:
            return "DIST_LOCK_SHARED";
        case DIST_LOCK_EXCL:
            return "DIST_LOCK_EXCL";
        default:
            return "unknown distributed lock type";
    }
}

DistributedLockType
distributedLockTypeFromString(const string &distributedLockTypeString)
{
    if (distributedLockTypeString.compare("DIST_LOCK_INIT") == 0) {
        return DIST_LOCK_INIT;
    }
    else if (distributedLockTypeString.compare("DIST_LOCK_SHARED") == 0) {
        return DIST_LOCK_SHARED;
    }
    else if (distributedLockTypeString.compare("DIST_LOCK_EXCL") == 0) {
        return DIST_LOCK_EXCL;
    }
    else {
        throw InvalidArgumentsException(
            string("distributedLockTypeString: No valid conversion for ") + 
            distributedLockTypeString);
    }
}

/* 
 * All indices use for parsing ZK node names
 */
const int32_t ClusterlibInts::CLUSTERLIB_INDEX = 1;
const int32_t ClusterlibInts::VERSION_NAME_INDEX = 2;
const int32_t ClusterlibInts::ROOT_INDEX = 3;
const int32_t ClusterlibInts::APP_INDEX = 4;
const int32_t ClusterlibInts::APP_NAME_INDEX = 5;

/*
 * Number of components in an Root key
 */
const int32_t ClusterlibInts::ROOT_COMPONENTS_COUNT = 4;

/*
 * Number of components in an Application key
 */
const int32_t ClusterlibInts::APP_COMPONENTS_COUNT = 6;

/*
 * Minimum components necessary to represent each respective key
 */
const int32_t ClusterlibInts::DIST_COMPONENTS_MIN_COUNT = 6;
const int32_t ClusterlibInts::PROP_COMPONENTS_MIN_COUNT = 6;
const int32_t ClusterlibInts::QUEUE_COMPONENTS_MIN_COUNT = 6;
const int32_t ClusterlibInts::GROUP_COMPONENTS_MIN_COUNT = 6;
const int32_t ClusterlibInts::NODE_COMPONENTS_MIN_COUNT = 6;
const int32_t ClusterlibInts::PROCESSSLOT_COMPONENTS_MIN_COUNT = 8;

const size_t ClusterlibInts::SEQUENCE_NUMBER_SIZE = 10;

const int32_t ClusterlibInts::INITIAL_ZK_VERSION = -3;
const int32_t ClusterlibInts::DELETED_ZK_VERSION = -2;
const int64_t ClusterlibInts::MSECS_NOT_AVAILABLE = -1;

}	/* End of 'namespace clusterlib' */
