/* 
 * ============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ============================================================================
 */

#include "log.h" 
#include "hash.h"

DEFINE_LOGGER( LOG, "cluster.hash" )

namespace clusterlib {

HashType getHash(const HashArgType& str)
{
    // adapted from jenkin's one-at-a-time hash
    uint32_t hash = 0;
    size_t i;
    
    for (i = 0; i < str.length(); i++) {
        hash += str.at(i);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

}       /* end of 'namespace clusterlib' */
