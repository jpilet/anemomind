/*
 * SpatialMedian.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_SPATIALMEDIAN_H_
#define SERVER_MATH_NONLINEAR_SPATIALMEDIAN_H_

#include <Eigen/Dense>
#include <server/common/math.h>

namespace sail {
namespace SpatialMedian {

struct Settings {
  double threshold = 1.0e-6;
  int iters = 30;
};

template <int N, typename T>
Eigen::Matrix<T, N, 1> computeMean(const Array<Eigen::Matrix<T, N, 1> > &pts) {
  Eigen::Matrix<T, N, 1> sum = Eigen::Matrix<T, N, 1>::Zero();
  for (auto pt: pts) {
    sum += pt;
  }
  return (1.0/pts.size())*sum;
}

template <int N, typename T>
Eigen::Matrix<T, N, 1> iterate(const Eigen::Matrix<T, N, 1> &estimate,
    const Array<Eigen::Matrix<T, N, 1> > &pts, double threshold) {
  Eigen::Matrix<T, N, 1> sum = Eigen::Matrix<T, N, 1>::Zero();
  T wsum = T(0.0);
  for (auto pt: pts) {
    auto diff = thresholdCloseTo0((pt - estimate).norm(), threshold);
    auto w = 0.5/diff;
    sum += w*pt;
    wsum += w;
  }
  return (T(1.0)/wsum)*sum;
}

// See https://en.wikipedia.org/wiki/Median#Spatial_median_.28L1_median.29:
// "It is a robust and highly efficient estimator of a central tendency of a population."
template <int N, typename T = double>
Eigen::Matrix<T, N, 1> compute(const Array<Eigen::Matrix<T, N, 1> > &pts, const Settings &settings) {
  Eigen::Matrix<T, N, 1> estimate = computeMean(pts);
  for (int i = 0; i < settings.iters; i++) {
    estimate = iterate(estimate, pts, settings.threshold);
  }
  return estimate;
}


}
}

#endif /* SERVER_MATH_NONLINEAR_SPATIALMEDIAN_H_ */
