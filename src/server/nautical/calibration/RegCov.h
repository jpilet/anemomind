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
#include <iostream>
#include <server/common/string.h>

namespace sail {

inline int getDataCount(int dim) {
  return dim/2;
}

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
Mapped<T> computeRegs(Array<T> src, int step) {
  auto acc = accumulateTrajectory(src);
  int n = acc.size()/2;
  CHECK(2*n == acc.size());
  int regCount = std::max(0, n - 2*step);
  return map(Spani(0, regCount), [=](int i) {
    return calcReg(acc, i, step);
  });
}

int computeDifCount(int dataSize, int step) {
  return std::max(0, dataSize - 2*step);
}

template <typename T>
Mapped<T> computeDifs(const Mapped<T> &src) {
  return map(Spani(0, std::max(0, src.size()-1)), [=](int i) {
    return src[i+1] - src[i];
  });
}

template <typename T>
Mapped<T> computeRegDifs(Array<T> src, int step) {
  return computeDifs(computeRegs(src, step));
}

template <typename T>
T computeMean(Mapped<T> x) {
  return (1.0/x.size())*reduce(x, [](double x, double y) {
    return x + y;
  });
}

template <typename T>
Mapped<T> subtractMean(Mapped<T> X) {
  auto mean = computeMean(X);
  return map(X, [=](double x) {return x - mean;});
}

template <typename T>
T computeCovariance(Arrayd gpsDifs, Array<T> flowDifs, Arrayi split) {
  CHECK(gpsDifs.size() == flowDifs.size());
  return reduce(map(subtractMean(subsetByIndex(gpsDifs, split)),
                    subtractMean(subsetByIndex(flowDifs, split)),
                    [](double x, double y) {
                      return x*y;
                    }),
            [=](double x, double y) {return x + y;});
}


}

#endif /* SERVER_NAUTICAL_CALIBRATION_REGCOV_H_ */
