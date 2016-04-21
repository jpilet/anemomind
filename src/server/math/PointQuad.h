/*
 * PointQuad.h
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 *
 *  Similar to QuadForm, but for the particular case of computing statistics
 *  related to a point cluster.
 */

#ifndef SERVER_MATH_POINTQUAD_H_
#define SERVER_MATH_POINTQUAD_H_

#include <Eigen/Core>
#include <server/common/logging.h>

template <typename T, int dims>
class PointQuad {
public:
  typedef Eigen::Matrix<T, dims, 1> Vec;

  PointQuad() : _n(0), _pt(T(0), T(0)), _sqSum(T(0)) {}
  PointQuad(const Vec &v) : _n(1), _pt(v), _sqSum(v.squaredNorm()) {}
  PointQuad(int n, const Vec &v, T s) : _n(n), _pt(v), _sqSum(s) {}

  PointQuad operator+(const PointQuad &other) const {
    return PointQuad(_n + other._n, _pt + other._pt, _sqSum + other._sqSum);
  }

  PointQuad operator-(const PointQuad &other) const {
    return PointQuad(_n - other._n, _pt - other._pt, _sqSum - other._sqSum);
  }

  Vec computeMean() const {
    CHECK_LT(0, _n);
    return T(1.0/_n)*_pt;
  }

  struct MeanAndVariance {
    Vec mean;
    T variance;

    T stddev() const {return sqrt(variance);}
  };

  MeanAndVariance computeMeanAndVariance() const {
    auto mu = computeMean();
    return MeanAndVariance{mu, computeVariance(mu)};
  }

  T computeVariance() const {
    return computeMeanAndVariance().variance;
  }

  T computeStandardDeviation() const {
    return computeMeanAndVariance().stddev();
  }

  PointQuad operator+=(const PointQuad &other) {
    _n += other._n;
    _pt += other._pt;
    _sqSum += other._sqSum;
    return *this;
  }

private:
  T computeVariance(const Vec &mean) const {
    return mean.squaredNorm() - (2.0/_n)*mean.dot(_pt) + (1.0/_n)*_sqSum;
  }

  int _n;
  Vec _pt;
  T _sqSum;
};

#endif /* SERVER_MATH_POINTQUAD_H_ */
