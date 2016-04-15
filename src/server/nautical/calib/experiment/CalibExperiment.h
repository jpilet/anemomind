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
#include <server/math/PointQuad.h>

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
Array<T> computeRelativeErrors(const Array<Eigen::Matrix<T, 2, 1> > &vecs, int size) {
  Integral1d<PointQuad<T, 2> > itg(sail::map(vecs, [&](const Eigen::Matrix<T, 2, 1> &v) {
    return PointQuad<T, 2>(v);
  }), PointQuad<T, 2>());
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
