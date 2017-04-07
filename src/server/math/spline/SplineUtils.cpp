/*
 * SplineUtils.cpp
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#include <server/math/spline/SplineUtils.h>
#include <server/math/band/BandWrappers.h>
#include <server/common/ArrayIO.h>
#include <server/common/LineKM.h>
#include <server/common/indexed.h>
#include <server/common/TimedValue.h>
#include <server/nautical/common.h>

namespace sail {

Array<double> makePowers(int n, double f) {
  Array<double> dst = Array<double>::fill(n, 1.0);
  dst[0] = 1.0;
  for (int i = 1; i < n; i++) {
    dst[i] = f*dst[i-1];
  }
  return dst;
}



} /* namespace sail */
