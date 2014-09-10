/*
 *  Created on: 2014-09-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DENSITYESTIMATOR_H_
#define DENSITYESTIMATOR_H_

#include <server/common/Array.h>
#include <server/common/math.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

/*
 * A common interface for density estimators.
 * This lets us easily replace the density estimation algorithm
 * with a faster one, once we have implemented that.
 */
template <int N>
class DensityEstimator {
 public:
  typedef Vectorize<double, N> Vec;

  virtual double density(const Vec &point) const = 0;

  virtual ~DensityEstimator() {}
};

/*
 * For a start, provide the simplest possible density estimator.
 * Estimating the density in a point is O(n), n being the number of
 * training points. It is not scalable, but better than nothing.
 */
template <int N>
class KernelDensityEstimator : public DensityEstimator<N> {
 public:
  typedef typename DensityEstimator<N>::Vec Vec;

  KernelDensityEstimator() : _squaredBandwidth(NAN) {}

  KernelDensityEstimator(double bandwidth,
        Array<Vec> samples) : _squaredBandwidth(bandwidth*bandwidth),
        _samples(samples) {
        assert(!_samples.empty());
  }

  double density(const Vec &point) const {
    double squaredBandwidth = sqr(_squaredBandwidth);
    double sum = 0;
    for (auto p: _samples) {
      sum += calcDensityTerm(p, point, squaredBandwidth);
    }
    return sum;
  }

  int dims() const {
    return _samples[0].size();
  }
 private:
  double _squaredBandwidth;
  Array<Vec> _samples;

  static double gaussianKernel(double squaredDistance, double squaredBandwidth) {
    return exp(-0.5*squaredDistance/squaredBandwidth);
  }

  static double calcDensityTerm(const Vec &a, const Vec &b, double squaredBandwidth) {
    return gaussianKernel(norm2dif<double, N>(a.data(), b.data()), squaredBandwidth);
  }
};


}

#endif /* DENSITYESTIMATOR_H_ */
