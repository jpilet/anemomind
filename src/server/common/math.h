/*
 * math.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef COMMON_MATH_H_
#define COMMON_MATH_H_

#include <cmath>
#include <cassert>
#include <limits>
#include <server/common/MDArray.h>

namespace sail {

template <typename T>
bool isFinite(const T &x);

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
constexpr T sqr(T x) {
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

template <typename T>
T norm2dif(int dims, const T *a, const T *b) {
  T sum = 0;
  for (int i = 0; i < dims; i++) {
    sum += sqr(a[i] - b[i]);
  }
  return sum;
}

template <typename T, int dims>
T norm2dif(const T *a, const T *b) {
  return norm2dif(dims, a, b);
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

inline double approximateExp(double x, double thresh) {
  if (x < thresh) {
    return 0;
  }
  return exp(x);
}

// Integer division, rounding to upper integer
inline int div1(int a, int b) {
  return (a - 1)/b + 1;
}

template <typename T>
void add(int dims, const T *a, const T *b, T *dst) {
  for (int i = 0; i < dims; i++) {
    dst[i] = a[i] + b[i];
  }
}

template <typename T>
void subtract(int dims, const T *a, const T *b, T *dst) {
  for (int i = 0; i < dims; i++) {
    dst[i] = a[i] - b[i];
  }
}

template <typename T>
void scale(int dims, T s, const T *x, T *dst) {
  for (int i = 0; i < dims; i++) {
    dst[i] = s*x[i];
  }
}

/*
 * Works for column major as well as
 * row major order of the coefficients
 * in a and b.
 */
template <typename T>
bool invert2x2(const T *a, T *b) {
  T denom = a[0]*a[3] - a[1]*a[2];
  T factor = 1.0/denom;
  b[0] = factor*a[3];
  b[1] = -factor*a[1];
  b[2] = -factor*a[2];
  b[3] = factor*a[0];
  return !(-std::numeric_limits<double>::epsilon() < denom &&
      denom < std::numeric_limits<double>::epsilon());
}


template <typename T>
void solveQuadratic(T a, T b, T c, T *x0, T *x1) {
  T twoA = 2*a;
  T q = sqrt(b*b - 4*a*c);
  *x0 = (-b + q)/twoA;
  *x1 = (-b - q)/twoA;
}

// A tri basis is a linear basis where the 2-norm
// between any pair of two different basis vectors is 1, such as
// two connected edges of a regular triangle or tetrahedron.
// For instance,
// If you have a tetrahedron, whose edges all have the same length 1,
// and translate that tetrahedron so that one of its vertices is at
// the origin, then it is possible to rotate the tetrahedron around
// the origin so that the remaining three vertices are the three vectors
// output by this function, respectively, when dim=3.
void makeTriBasisVector(int dims, int index, double *dst);


template <typename T>
T mirror(T x, T width) {
  T doubleWidth = 2*width;
  T local = positiveMod(x, doubleWidth);
  if (local < width) {
    return local;
  } else {
    return doubleWidth - local;
  }
}

template <typename T>
void mirror(int dims, T width, const T *x, T *y) {
  for (int i = 0; i < dims; i++) {
    y[i] = mirror(x[i], width);
  }
}

template <typename T>
T smoothNonNegAbs(T x, T thresh) {
  if (x < T(0)) {
    return smoothNonNegAbs(-x, thresh);
  } else if (x < thresh) {
    T a = 1.0/(2.0*thresh);
    T b = 0.5*thresh;
    return a*x*x + b;
  }
  return x;
}

template <typename T>
T smoothNonNegAbs2(T x, T thresh) {
  return sqrt(thresh + x*x);
}

template <typename T>
struct MatrixElement {
 int i, j;
 T value;
};
typedef MatrixElement<double> MatrixElementd;

inline double thresholdCloseTo0(double x, double lb) {
  if (x < 0) {
    return -thresholdCloseTo0(-x, lb);
  } else if (x < lb) {
    return lb;
  }
  return x;
}

template <typename T> // Should work for AD types too.
bool genericIsNan(T x) {
  return !(x == x);
}

inline bool implies(bool a, bool b) {
  return !a || b;
}

template <typename T, int dims>
bool isFiniteMDArray(const MDArray<T, dims> &X) {
  /*int rows = X.rows();
  int cols = X.cols();
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      T x = X(i, j);
      if (!isFinite(x)) {
        return false;
      }
    }
  }*/
  return true;
}

template <typename T>
bool isFiniteArray(Array<T> X) {
  for (int i = 0; i < X.size(); i++) {
    if (!isFinite(X[i])) {
      return false;
    }
  }
  return true;
}

template <typename T>
T toFinite(T x, T defaultValue) {
  return (std::isfinite(x)? x : defaultValue);
}

template <typename T>
T clamp(T x, T lower, T upper) {
  return std::min(std::max(x, lower), upper);
}

// SFINAE: http://jguegant.github.io/blogs/tech/sfinae-introduction.html
// So that we can have a single IsFinite<T>::evaluate(x) for any type T,
// be it composite template type or a primitive, or anything else.
template <typename T, typename X=void>
struct IsFinite {
 static bool evaluate(const T &x) {
   return false;
 }
};

template <typename T>
struct IsFinite<T, decltype(std::isfinite(std::declval<T>()))> {
 static bool evaluate(const T &x) {
   return std::isfinite(x);
 }
};

template <typename T>
struct IsFinite<T, decltype(isFiniteMDArray(std::declval<T>()))> {
  static bool evaluate(const MDArray<T, 2> &array) {
    return isFiniteMDArray(array);
  }
};

template <typename T>
struct IsFinite<T, decltype(isFiniteArray(std::declval<T>()))> {
  static bool evaluate(const Array<T> &array) {
    return isFiniteArray(array);
  }
};


template <typename T>
bool isFinite(const T &x) {
  return IsFinite<T>::evaluate(x);
}

/*
 * A calculation is sane if, whenever
 * the result of the calculation is non-finite,
 * at least one argument was also non-finite. If all
 * the arguments are finite but not the result, then something
 * is probably wrong.
 */
template <typename T>
bool saneCalculation(T result, Array<T> arguments) {
  if (std::isfinite(result)) {
    return true;
  } else {
    return !isFinite(arguments);
  }
}

Arrayd makeNextRegCoefs(Arrayd coefs);
Arrayd makeRegCoefs(int order);

} /* namespace sail */

#endif /* COMMON_MATH_H_ */
