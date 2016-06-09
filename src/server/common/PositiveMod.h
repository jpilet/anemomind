/*
 * PositiveMod.h
 *
 *  Created on: Jun 9, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_POSITIVEMOD_H_
#define SERVER_COMMON_POSITIVEMOD_H_

namespace sail {

template <typename T>
T makePositiveCyclic(T a, T b0, T zero = T(0)) {
  T b = b0;
  T c = a + b;
  while (c < zero) {
    b = b + b;
    c = a + b;
  }
  return c;
}

template <typename T>
T reduceCyclically(T a, T b) {
  T c = b;
  while (!(a < c + c)) {
    c = c + c;
  }
  return a - c;
}


/*
 * A modulo function that always returns non-negative numbers.
 * Works with all types T that satisfy these conditions:
 *
 *   - Can be compared by <
 *   - Can be added
 *   - Can be subtracted
 *
 */
template <typename T>
T positiveMod(T a, T b, T zero = T(0)) {
  if (a < zero) {
    return positiveMod(makePositiveCyclic(a, b, zero), b, zero);
  }
  T c = a;
  while (!(c < b)) {
    c = reduceCyclically(c, b);
  }
  return c;
}


}



#endif /* SERVER_COMMON_POSITIVEMOD_H_ */
