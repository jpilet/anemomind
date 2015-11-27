/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_REGCOV_H_
#define SERVER_NAUTICAL_CALIBRATION_REGCOV_H_

#include <server/common/Array.h>
#include <server/common/logging.h>
#include <server/common/Functional.h>
#include <server/common/Span.h>
#include <server/common/math.h>

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
T calcReg(Array<T> trajectory, int index, int step, double reg = 1.0e-9) {
  int dim = 2;
  int dimStep = dim*step;
  int a = dim*index;
  int b = a + dimStep;
  int c = b + dimStep;
  T sum(0.0);
  for (int i = 0; i < 2; i++) {
    sum += sqr(trajectory[a + i] - 2.0*trajectory[b + i] + trajectory[c + i]);
  }
  return sqrt(sum + reg);
}

template <typename T>
Array<T> computeRegDerivatives(Array<T> src, int step) {
  auto acc = accumulateTrajectory(src);
  int n = acc.size()/2;
  CHECK(2*n == acc.size());
  int regCount = n - 2*step;
  auto regs = map(Spani(0, regCount), [=](int i) {
    return calcReg(acc, i, step);
  });
  return map(Spani(0, regCount-1), [=](int i) {
    return regs[i+1] - regs[i];
  });
}

}

#endif /* SERVER_NAUTICAL_CALIBRATION_REGCOV_H_ */
