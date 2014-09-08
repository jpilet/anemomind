/*
 *  Created on: 2014-09-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/DensityEstimator.h>
#include <server/common/math.h>
#include <cassert>

namespace sail {

namespace {
  double gaussianKernel(double squaredDistance, double squaredBandwidth) {
    return exp(-0.5*squaredDistance/squaredBandwidth);
  }

  double calcDensityTerm(const Arrayd &a, const Arrayd &b, double squaredBandwidth) {
    return gaussianKernel(norm2dif(a.size(), a.ptr(), b.ptr()), squaredBandwidth);
  }
}

NaiveDensityEstimator::NaiveDensityEstimator(double bandwidth,
      Array<Arrayd> samples) : _bandwidth(bandwidth),
      _samples(samples) {
      assert(consistentDims(samples));
      assert(!samples.empty());
}

double NaiveDensityEstimator::density(const Arrayd &point) {
  assert(point.size() == dims());
  double squaredBandwidth = sqr(_bandwidth);
  double sum = 0;
  for (auto p: _samples) {
    sum += calcDensityTerm(p, point, squaredBandwidth);
  }
  return sum;
}

bool NaiveDensityEstimator::consistentDims(Array<Arrayd> samples) {
  if (samples.empty()) {
    return true;
  }

  int n = samples[0].size();
  for (auto s: samples) {
    if (n != s.size()) {
      return false;
    }
  }
  return true;
}

}
