/*
 * JetUtils.h
 *
 *  Created on: 22 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_JETUTILS_H_
#define SERVER_MATH_JETUTILS_H_

#include <ceres/jet.h>

namespace sail {

template <typename T>
struct MakeConstant {
  static T apply(double x) {
    return T(x);
  };
};

template <typename T, int N>
struct MakeConstant<ceres::Jet<T, N> > {
  static ceres::Jet<T, N> apply(double x) {
    return ceres::Jet<T, N>(MakeConstant<T>::apply(x));
  }
};

}

#endif /* SERVER_MATH_JETUTILS_H_ */
