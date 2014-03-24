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

/*
 * Please see PhysicalQuantity.h
 * These functions may soon be deprecated.
 */
#define MAKE_UNIT2OTHERUNIT_CONVERTER(fwdName, invName, factor) template <typename T> T fwdName(T x) {return (factor)*x;} template <typename T> T invName(T x) {return (1.0/(factor))*x;}
MAKE_UNIT2OTHERUNIT_CONVERTER(deg2rad, rad2deg, M_PI/180.0);
MAKE_UNIT2OTHERUNIT_CONVERTER(nm2m, m2nm, 1852.0);
MAKE_UNIT2OTHERUNIT_CONVERTER(knots2MPS, MPS2knots, 1852.0/3600.0);
#undef MAKE_UNIT2OTHERUNIT_CONVERTER

// Returns an integer in 0..(b-1): this is not the case with the built in modulo.
inline int posMod(int a, int b) {
  return (a + b - (a/b)*b) % b;
}

// Returns a number in [0, b[: this is not the case with fmod.
double floatMod(double a, double b);

// Adds/subtracts 2*pi to an angle a until it is in [b-pi, b+pi[
double angleAtRadians(double a, double b);

} /* namespace sail */

#endif /* COMMON_MATH_H_ */
