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

namespace ceres {
template <typename T, int N>
struct Jet;
}

namespace sail {

template <typename T, typename=bool>
struct IsFinite {
 static bool evaluate(const T &x) {
   return std::isfinite(x);
 }
};

template <typename T, typename=bool>
struct IsNaN {
 static bool evaluate(const T &x) {
   return std::isnan(x);
 }
};

#define SPECIALIZE_NUMERIC_TEMPLATE(ClassName, FunctionCall) \
  template <typename T> \
  struct ClassName<T, decltype(FunctionCall(std::declval<T>()))> { \
    static bool evaluate(const T &x) {return FunctionCall(x);} \
  };

template <typename T>
bool isFinite(const T &x) {
  return IsFinite<T, bool>::evaluate(x);
}

template <typename T>
bool isNaN(const T &x) {
  return IsNaN<T, bool>::evaluate(x);
}


template <typename T, int N>
inline bool isNaNCeresJet(const ceres::Jet<T, N> &x) {return std::isnan(x.a);}

template <typename T, int N>
inline bool isFiniteCeresJet(const ceres::Jet<T, N> &x) {return std::isfinite(x.a);}

SPECIALIZE_NUMERIC_TEMPLATE(IsFinite, isFiniteCeresJet)
SPECIALIZE_NUMERIC_TEMPLATE(IsNaN, isNaNCeresJet)


}

#endif /* SERVER_COMMON_NUMERIC_H_ */
