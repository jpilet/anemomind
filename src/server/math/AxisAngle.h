/*
 * AxisAngle.h
 *
 *  Created on: 24 Sep 2016
 *      Author: jonas
 *
 * Note: Eigen also has code for axis-angle representation,
 * but it is not written with differentiability in mind.
 */

#ifndef SERVER_MATH_AXISANGLE_H_
#define SERVER_MATH_AXISANGLE_H_

#include <server/math/JetUtils.h>

namespace sail {

template <typename T>
Eigen::Matrix<T, 3, 3> crossProductMatrix(
    const Eigen::Matrix<T, 3, 1> &a) {
  Eigen::Matrix<T, 3, 3> dst;
  auto z = MakeConstant<T>::apply(0.0);
  dst << z, -a(2), a(1),
         a(2), z, -a(0),
         -a(1), a(0), z;
  return dst;
}

template <typename T>
Eigen::Matrix<T, 3, 3> computeRotationFromOmega(
  Eigen::Matrix<T, 3, 1> &omega) {
    T theta = omega.norm();

    // Add a small number to the denominator
    // to keep things differentiable.
    auto reg = MakeConstant<T>::apply(1.0e-12);

    Eigen::Matrix<T, 3, 1> axis =
        (MakeConstant<T>::apply(1.0)/(theta + reg))*omega;

    auto one = MakeConstant<T>::apply(1.0);
    auto K = crossProductMatrix(axis);
    auto cosTheta = cos(theta);
    return Eigen::Matrix<T, 3, 3>::Identity()
        + sin(theta)*K + (one - cosTheta)*K*K;
  }
}



#endif /* SERVER_MATH_AXISANGLE_H_ */
