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


namespace clusterlib
{
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

};	/* End of 'namespace clusterlib' */
