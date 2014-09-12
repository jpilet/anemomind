/*
 *  Created on: 2014-09-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DENSITYESTIMATOR_H_
#define DENSITYESTIMATOR_H_

#include <server/common/Array.h>
#include <server/common/math.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <iostream>
#include <server/common/string.h>
#include <adolc/adouble.h>
#include <server/common/ToDouble.h>

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

  KernelDensityEstimator() :
    _squaredBandwidth(NAN), _expThresh(initExpThresh) {}

  KernelDensityEstimator(double bandwidth,
        Array<Vec> samples) : _squaredBandwidth(bandwidth*bandwidth),
        _samples(samples),
        _expThresh(initExpThresh) {
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

  template <typename T>
  T density(const T *pointN) const {
    double squaredBandwidth = sqr(_squaredBandwidth);
    T sum = 0;
    for (auto p: _samples) {
      T dt = calcDensityTerm(p, pointN,
                squaredBandwidth);
      assert(0 <= dt);
      assert(dt <= 1.0);
      sum += dt;
    }
    return sum;
  }

  int dims() const {
    return _samples[0].size();
  }

  bool empty() const {
    return _samples.empty();
  }

  int count() const {
    return _samples.size();
  }

  double maxDensity() const {
    return count();
  }
 private:
  static constexpr double initExpThresh = log(1.0e-2);
  double _squaredBandwidth;
  Array<Vec> _samples;
  double _expThresh;

  template <typename T>
  T gaussianKernel(T squaredDistance, double squaredBandwidth) const {
    if (!(squaredDistance >= 0)) {
      std::cout << EXPR_AND_VAL_AS_STRING(ToDouble<adouble>(squaredDistance)) << std::endl;
    }
    assert(squaredDistance >= 0);
    assert(squaredBandwidth > 0);
    T x = -0.5*squaredDistance/squaredBandwidth;
    constexpr double tol = 1.0e-9;
    assert(x <= tol);
    return exp(x);
  }

  double calcDensityTerm(const Vec &a, const Vec &b, double squaredBandwidth) const {
    assert(false);
    return gaussianKernel(norm2dif<double, N>(a.data(), b.data()), squaredBandwidth);
  }

  template <typename T>
  T calcDensityTerm(const Vec &refpt, const T *x, double squaredBandwidth) const {
    T dist = 0;
    for (int i = 0; i < N; i++) {
      dist += sqr(refpt[i] - x[i]);
    }
    if (0 > dist || std::isnan(dist.getValue())) {
      std::cout << EXPR_AND_VAL_AS_STRING(x[0].getValue()) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(x[1].getValue()) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(x[2].getValue()) << std::endl;
    }
    return gaussianKernel(dist, squaredBandwidth);
  }
};


}

#endif /* DENSITYESTIMATOR_H_ */
