/*
 * math.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef COMMON_MATH_H_
#define COMMON_MATH_H_

#include <cmath>

namespace sail {

//template <typename T, int E>
//class Expt
//{
//public:
//	inline static T eval(T x) {return x*Expt<T, E-1>(x);}
//};
//
//template <typename T>
//class Expt<T, 0>
//{
//public:
//	inline static T eval(T x) {return 1;}
//};

template <int a, int b>
class StaticPower {
 public:
  static const int result = a*StaticPower<a, b-1>::result;
};

template <int a>
class StaticPower<a, 0> {
 public:
  static const int result = 1;
};

template <typename T>
T sqr(T x) {
  return x*x;
}

template <typename T, int dims>
void sub(T *a, T *b, T *aMinusB) {
  for (int i = 0; i < dims; i++) {
    aMinusB[i] = a[i] - b[i];
  }
}

template <typename T, int dims>
T norm2(T *x) {
  T result = 0;
  for (int i = 0; i < dims; i++) {
    result += sqr(x[i]);
  }
  return result;
}

template <typename T>
T norm2(int dims, T *x) {
  T result = 0;
  for (int i = 0; i < dims; i++) {
    result += sqr(x[i]);
  }
  return result;
}

template <typename T>
T norm(int dims, T *x) {
  return sqrt(norm2<T>(dims, x));
}

template <typename T>
void normalizeInPlace(int dims, T *x) {
  T f = 1.0/norm<T>(dims, x);
  for (int i = 0; i < dims; i++) {
    x[i] *= f;
  }
}



template <typename T, int dims>
T norm2dif(T *a, T *b) {
  T res[dims];
  sub<T, dims>(a, b, res);
  return norm2<T, dims>(res);
}

template <typename T, int dims>
T normdif(T *a, T *b) {
  return sqrt(norm2dif<T, dims>(a, b));
}

// returns (a has the same value as b) , even if that value is nan or inf.
// Otherwise, nan == nan will evaluate to false.
template <typename T>
bool strictEquality(T a, T b) {
  if (std::isnan(a)) {
    return std::isnan(b);
  } else if (std::isinf(a)) {
    if (std::isinf(b)) {
      return (a > 0) == (b > 0);
    }
    return false;
  } else {
    return a == b;
  }
}

/*
 * Please see PhysicalQuantity.h
 * These functions may soon be deprecated.
 */
#define MAKE_UNIT2OTHERUNIT_CONVERTER(fwdName, invName, factor) template <typename T> T fwdName(T x) {return (factor)*x;} template <typename T> T invName(T x) {return (1.0/(factor))*x;}
MAKE_UNIT2OTHERUNIT_CONVERTER(deg2rad, rad2deg, M_PI/180.0);
MAKE_UNIT2OTHERUNIT_CONVERTER(nm2m, m2nm, 1852.0);
MAKE_UNIT2OTHERUNIT_CONVERTER(knots2MPS, MPS2knots, 1852.0/3600.0);
#undef MAKE_UNIT2OTHERUNIT_CONVERTER

// Always returns a number in [0, b[
template <typename T>
T positiveMod(T a, T b) {
  assert(b > T(0));
  T aOverB = std::floor(a/b);
  if (a >= T(0)) {
    return a - aOverB*b;
  } else {
    T a2 = a - (aOverB - 1)*b;
    assert(a2 >= T(0));
    return a2 - std::floor(a2/b)*b;
  }
}

template <typename T>
T cyclicDif(T a, T b, T cycle) {
  T dif = positiveMod(a - b, cycle);
  T dif2 = cycle - dif;
  return (dif < dif2? dif : dif2);
}

template <typename T>
T normalizeAngleBetweenMinusPiAndPi(T a) {
  // Unfortunately, positiveMod can't be instanciated with a ceres::Jet.
  // Let's fake positiveMod with comparisons and additions.
  T result(a);
  while (result < T(-M_PI)) {
    result += T(2.0 * M_PI);
  }
  while (result > T(M_PI)) {
    result -= T(2.0 *M_PI);
  }
  return result;
}

template <typename T>
bool near(T a, T b, T marg) {
  T dif = std::abs(a - b);
  return dif <= marg;
}

template <typename T>
bool nearWithNan(T a, T b, T marg) {
  if (std::isnan(a)) {
    return std::isnan(b);
  }
  return near(a, b, marg);
}

/*
 * The sigmoid function (see http://en.wikipedia.org/wiki/Sigmoid_function)
 * As x -> -infty, evalSigmoid -> 0
 * As x ->  infty, evalSigmoid -> 1
 */
template <typename T>
T evalSigmoid(T x) {
  return 1.0/(1.0 + exp(-x));
}

/*
 * This class is useful in optimization problems where we need to
 * bound some value to an open interval ]_minv, _maxv[.
 *
 * Also, its derivative is always nonzero, making it less likely to get stuck.
 *
 * It is always true that 0 maps to the middle of the interval, making it
 * reasonable to initialize optimization parameters to 0.
 */
template <typename T>
class Sigmoid {
 public:
  Sigmoid() : _k(1), _m(0.0) {}
  Sigmoid(T minv, T maxv) : _k(maxv - minv), _m(minv) {}

  T eval(T x) {
    return _k*evalSigmoid(x) + _m;
  }
 private:
  T _k, _m;
};

} /* namespace sail */

#endif /* COMMON_MATH_H_ */
