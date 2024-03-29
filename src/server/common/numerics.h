/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *
 *  In order to write generic code, we need polymorphic functions
 *  that work on different types of numbers, such as double,
 *  int, PhysicalQuantity, ceres::Jet, adouble, etc. This header
 *  files declare common math functions that can be used with those
 *  types, and uses SFINAE to figure out exactly what should be done.
 */

#ifndef SERVER_COMMON_NUMERIC_H_
#define SERVER_COMMON_NUMERIC_H_

#include <cmath>
#include <type_traits>
#include <utility>

namespace ceres {
template <typename T, int N>
struct Jet;
}

namespace sail {

template <typename T>
// If T is an integral type (char, int long, etc), the following type resolves
// as bool and is used as return type. If the return type can't be deduced from
// T, this function will not be used for this type (SFINAE)
typename std::enable_if<std::is_integral<T>::value, bool>::type isNaN(T x) {
  return false;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type isFinite(T x) {
  return true;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type isNaN(T x) {
  return std::isnan(x);
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type isFinite(T x) {
  return std::isfinite(x);
}

template <typename T, int N>
bool isNaN(const ceres::Jet<T, N> &x) {return std::isnan(x.a);}

template <typename T, int N>
bool isFinite(const ceres::Jet<T, N> &x) {return std::isfinite(x.a);}

}

#endif /* SERVER_COMMON_NUMERIC_H_ */
