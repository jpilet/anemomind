/*
 * HashUtils.h
 *
 *  Created on: 3 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_HASHUTILS_H_
#define SERVER_COMMON_HASHUTILS_H_

namespace sail {

inline size_t combineHash(
    const std::size_t seed, const std::size_t& x) {
    return seed ^ (x + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

template <typename T>
size_t computeHash(const T& x) {
  return std::hash<T>()(x);
}

}



#endif /* SERVER_COMMON_HASHUTILS_H_ */
