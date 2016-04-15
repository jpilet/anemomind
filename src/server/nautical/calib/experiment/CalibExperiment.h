/*
 * Experimental.h
 *
 *  Created on: Apr 7, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_EXPERIMENTAL_EXPERIMENTAL_H_
#define SERVER_NAUTICAL_EXPERIMENTAL_EXPERIMENTAL_H_

#include <Eigen/Dense>
#include <server/math/Integral1d.h>
#include <server/nautical/NavCompatibility.h>
#include <server/common/Functional.h>

namespace sail {
namespace Experimental {

void plotFlowsXY(Array<Eigen::Vector2d> flows);
void plotFlowsTime(Array<Eigen::Vector2d> flows);
void plotFlowsItg(Array<Eigen::Vector2d> flows);

struct FlowMats {
  static constexpr int cols = 4;

  Eigen::Matrix<double, 2, cols> A;
  Eigen::Matrix<double, 2, 1> B;

  template <typename T>
  Eigen::Matrix<T, 2, 1> eval(const T *x) const {
    Eigen::Matrix<T, 2, 1> dst;
    dst(0) = T(B(0));
    dst(1) = T(B(1));
    for (int i = 0; i < cols; i++) {
      dst(0) += A(0, i)*x[i];
      dst(1) += A(1, i)*x[i];
    }
    return dst;
  }
};

FlowMats makeCurrentMatrix(const Nav &x);
Eigen::Vector4d getDefaultParams();

Array<FlowMats> makeCurrentMats(const Array<Nav> &navs);

template <typename T>
Array<Eigen::Matrix<T, 2, 1> > computeFlows(const Array<FlowMats> &mats, const T *x) {
  return sail::map(mats, [&](const FlowMats &m) {
    return m.eval(x);
  });
}

template <typename T>
class Gaussian {
public:
  typedef Eigen::Matrix<T, 2, 1> Vec;

  Gaussian() : _n(0), _pt(T(0), T(0)), _sqSum(T(0)) {}
  Gaussian(const Vec &v) : _n(1), _pt(v), _sqSum(v.squaredNorm()) {}
  Gaussian(int n, const Vec &v, T s) : _n(n), _pt(v), _sqSum(s) {}

  Gaussian operator+(const Gaussian &other) const {
    return Gaussian(_n + other._n, _pt + other._pt, _sqSum + other._sqSum);
  }

  Gaussian operator-(const Gaussian &other) const {
    return Gaussian(_n - other._n, _pt - other._pt, _sqSum - other._sqSum);
  }

  Vec computeMean() const {
    return T(1.0/_n)*_pt;
  }

  T computeVariance(const Vec &mean) const {
    return mean.squaredNorm() - (2.0/_n)*mean.dot(_pt) + (1.0/_n)*_sqSum;
  }

  T computeVariance() const {
    return computeVariance(computeMean());
  }

  T computeStandardDeviation(const Vec &mean) const {
    return sqrt(1.0e-12 + computeVariance(mean));
  }
private:
  int _n;
  Vec _pt;
  T _sqSum;
};

template <typename T>
Array<T> computeRelativeErrors(const Array<Eigen::Matrix<T, 2, 1> > &vecs, int size) {
  Integral1d<Gaussian<T> > itg(sail::map(vecs, [&](const Eigen::Matrix<T, 2, 1> &v) {
    return Gaussian<T>(v);
  }), Gaussian<T>());
  int n = vecs.size() - size + 1;
  auto half = size/2;
  Array<T> result(n);
  for (int i = 0; i < n; i++) {
    int from = i;
    int to = i + size;
    int middle = from + half;
    auto e0 = 0.5*(itg.integrate(from, middle).computeVariance() +
        itg.integrate(middle, to).computeVariance());
    auto e1 = itg.integrate(from, to).computeVariance();
    result[i] = e0/e1;
  }
  return result;
}


}
}

#endif /* SERVER_NAUTICAL_EXPERIMENTAL_EXPERIMENTAL_H_ */
