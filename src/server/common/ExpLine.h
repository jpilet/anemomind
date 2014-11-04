/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef EXPLINE_H_
#define EXPLINE_H_

#include <server/common/ToDouble.h>

namespace sail {

/*
 * This function maps a real number
 * (negative or positive) to a positive number.
 *
 * It has a more suitable behaviour than exp in the
 * sense that it doesn't grow exponentially for large numbers,
 * but linearly. In addition, it is convex.
 */
template <typename T>
T expline(T x) {
  if (ToDouble(x) < 0) {
    return exp(x);
  } else {
    return 1.0 + exp(1.0)*x;
  }
}

// inverse function to the function above.
template <typename T>
T logline(T x) {
  if (ToDouble(x) > 1.0) {
    return (x - 1.0)*exp(-1.0);
  } else {
    return log(x);
  }
}

}



#endif /* EXPLINE_H_ */
