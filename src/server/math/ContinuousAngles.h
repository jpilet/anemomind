/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CONTINUOUSANGLES_H_
#define CONTINUOUSANGLES_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/Array.h>

namespace sail {

template <typename T>
Array<Angle<T> > makeContinuousAngles(const Array<Angle<T> > &src) {
  int count = src.size();
  Array<Angle<T> > dst(count);
  dst[0] = src[0].normalizedAt0();
  for (int i = 1; i < count; i++) {
    dst[i] = dst[i-1] + (src[i] - src[i-1]).normalizedAt0();
  }
  return dst;
}


}

#endif /* CONTINUOUSANGLES_H_ */
