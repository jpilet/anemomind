/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_REGCOV_H_
#define SERVER_NAUTICAL_CALIBRATION_REGCOV_H_

#include <server/common/Array.h>
#include <server/common/logging.h>

namespace sail {

template <typename T>
Array<T> accumulateTrajectory(Array<T> src) {
  const int dim = 2;
  int n = src.size()/dim;
  CHECK(n*dim == src.size());
  auto dst = Array<T>::fill((n + 1)*dim, T(0.0));
  for (int i = 0; i < n; i++) {
    int srcOffset = dim*i;
    int dstOffset = srcOffset + dim;
    for (int j = 0; j < dim; j++) {
      dst[dstOffset + j] = dst[dstOffset + j - dim] + src[srcOffset + j];
    }
  }
  return dst;
}

template <typename T>
Array<T> computeRegDerivatives(Array<T> src) {
  auto acc = accumulateTrajectory(src);
}

}

#endif /* SERVER_NAUTICAL_CALIBRATION_REGCOV_H_ */
