/*
 * IntervalLookup.h
 *
 *  Created on: May 20, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_INTERVALLOOKUP_H_
#define SERVER_COMMON_INTERVALLOOKUP_H_

#include <algorithm>
#include <iterator>

namespace sail {


// Finds a lambda, so that
// x = (1.0 - lambda)*lower + lambda*upper
// Useful when doing linear interpolation
template <typename T>
auto computeLambda(T lower, T upper, T x) -> decltype((x - lower)/(upper - lower)) {
  typedef decltype((x - lower)/(upper - lower)) ResultType;

  return lower < upper?
    ResultType((x - lower)/(upper - lower)) : ResultType(0.5);
}

// Finds the index of the interval to which x belongs.
template <typename Iter>
int findIntervalIndex(Iter from, Iter to,
    typename std::iterator_traits<Iter>::value_type x) {
  if (from == to) {
    return -1;
  }
  return std::upper_bound(from, to, x) - from - 1;
}

}



#endif /* SERVER_COMMON_INTERVALLOOKUP_H_ */
