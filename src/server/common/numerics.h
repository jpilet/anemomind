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
#include <utility>
#include <type_traits>

namespace ceres {
template <typename T, int N>
struct Jet;
}

namespace sail {

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type IsNaN(T x) {
  return false;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type IsFinite(T x) {
  return true;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type IsNaN(T x) {
  return std::isnan(x);
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type IsFinite(T x) {
  return std::isfinite(x);
}


// Because IsNaN must always be a template in order for resolution to
// work well.
/*#define WRAP_NUMERIC_FUNCTION_FOR_NON_TEMPLATE_TYPE(NAME, TYPE, FUNCTION_TO_WRAP) \
  template <typename T> \
  typename std::enable_if<std::is_same<T, TYPE>::value, bool>::type NAME(const T &x) { \
    return FUNCTION_TO_WRAP(x); \
  }*/

template <typename T, int N>
bool IsNaN(const ceres::Jet<T, N> &x) {return std::isnan(x.a);}

template <typename T, int N>
bool IsFinite(const ceres::Jet<T, N> &x) {return std::isfinite(x.a);}

// Main user function wrapping the resolution mechanism
template <typename T>
bool isFinite(const T &x) {
  return IsFinite(x);
}

// Main user function wrapping the resolution mechanism
template <typename T>
bool isNaN(const T &x) {
  return IsNaN(x);
}






}

#endif /* SERVER_COMMON_NUMERIC_H_ */
