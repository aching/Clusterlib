/* 
 * ===========================================================================
 * $Header$
 * $Revision$
 * $Date$
 * ===========================================================================
 */

/**
 * This file provides hashing related types and functions that are used 
 * by the cluster's data distribution(s).
 * 
 * TODO this is the first step to seperate hash function from the routing proxy; 
 * see BZ#1689313 for more details.
 */
  
#ifndef __HASH_H__
#define __HASH_H__

#include <string>

using namespace std;

namespace clusterlib {

/**
 * \brief The data type of the argument of a hash function.
 */
typedef string HashArgType;
        
/**
 * \brief The data type returned by a hash function.
 */
typedef uint64_t HashType;
    
/**
 * \brief Calculates a hash value for the given string. 
 * It uses a modified version of a Bob Jenkin's hash function.
 * 
 * @param str the value to be hashed
 * @return the hash value
 */
HashType getHash(const HashArgType& str); 

};  /* End of 'namespace clusterlib' */

#endif  /* __HASH_H__ */
