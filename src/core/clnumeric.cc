/*
 * clusterlibints.cc --
 *
 * Implementation of CLNumeric.
 *
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "clusterlibinternal.h"

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

const int32_t CLNumeric::INITIAL_ZK_VERSION = -3;
const int32_t CLNumeric::DELETED_ZK_VERSION = -2;

/* 
 * All indices use for parsing ZK node names
 */
const int32_t CLNumericInternal::CLUSTERLIB_INDEX = 1;
const int32_t CLNumericInternal::VERSION_NAME_INDEX = 2;
const int32_t CLNumericInternal::ROOT_INDEX = 3;
const int32_t CLNumericInternal::APP_INDEX = 4;
const int32_t CLNumericInternal::APP_NAME_INDEX = 5;

/*
 * Number of components in an Root key
 */
const int32_t CLNumericInternal::ROOT_COMPONENTS_COUNT = 4;

/*
 * Number of components in an Application key
 */
const int32_t CLNumericInternal::APP_COMPONENTS_COUNT = 6;

/*
 * Minimum components necessary to represent each respective key
 */
const int32_t CLNumericInternal::DIST_COMPONENTS_MIN_COUNT = 6;
const int32_t CLNumericInternal::PROP_COMPONENTS_MIN_COUNT = 6;
const int32_t CLNumericInternal::QUEUE_COMPONENTS_MIN_COUNT = 6;
const int32_t CLNumericInternal::GROUP_COMPONENTS_MIN_COUNT = 6;
const int32_t CLNumericInternal::NODE_COMPONENTS_MIN_COUNT = 6;
const int32_t CLNumericInternal::PROCESSSLOT_COMPONENTS_MIN_COUNT = 8;

const size_t CLNumericInternal::SEQUENCE_NUMBER_SIZE = 10;

}	/* End of 'namespace clusterlib' */
