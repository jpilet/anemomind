/*
 * LineKM.h
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#ifndef LINEKM_H_
#define LINEKM_H_

#include <iostream>
#include <cmath>

namespace sail {


template <typename T>
void calcLineKM(T x0, T x1, T y0, T y1, T &k, T &m) {
  k = (y1 - y0)/(x1 - x0);
  m = y0 - k*x0;
}

// function on the form f(x) = _k*x + _m
// Based on the previous LineKM class that works with double, generalized to any type T.
template <typename T>
class GenericLineKM {
 public:
  GenericLineKM(T x0, T x1, T y0, T y1) {
    calcLineKM(x0, x1, y0, y1, _k, _m);
  }

  GenericLineKM(T k, T m) : _k(k), _m(m) {}
  GenericLineKM() : _k(T(1.0)), _m(T(0.0)) {}

  static GenericLineKM undefined() {
    return GenericLineKM(T(NAN), T(NAN));
  }

  static GenericLineKM identity() {
    return GenericLineKM(T(1.0), T(0.0));
  }

  static GenericLineKM constant(T c) {
    return GenericLineKM(T(0.0), c);
  }
  T operator() (T x) const {
    return _k*x + _m;
  }

  T inv(T x) const {
    return (x - _m)/_k;
  }

  T getK() const {
    return _k;
  }

  T getM() const {
    return _m;
  }

  bool operator==(const GenericLineKM &other) const {
    return _k == other._k && _m == other._m;
  }

  GenericLineKM makeInvFun() const {
    return GenericLineKM(T(1.0)/_k, -_m/_k);
  }

  /*
   * Finds two contiguous integers, I[0] and I[1] with I[0] + 1 = I[1],
   * and two weights W[0] and W[1] with W[0]+W[1] = 1.0,
   *
   * so that x = W[0]*(*this)(I[0]) + W[1]*(*this)(I[1])
   *
   * Useful for piecewise linear interpolation
   */
  void makeInterpolationWeights(T x,
      int *indsOut2,
      T *weightsOut2) const {
    T realIndex = inv(x);
    T indexd = floor(realIndex);
    int index = int(indexd);
    T lambda = realIndex - indexd;
    indsOut2[0] = index;
    indsOut2[1] = index+1;
    weightsOut2[0] = 1.0 - lambda;
    weightsOut2[1] = lambda;
  }

  // Solve k*x + m = y <=> x = (y - m)/k
  T solveWithEquality(T y) {
    return (y - _m)/_k;
  }
 private:
  T _k, _m;
};

typedef GenericLineKM<double> LineKM;

template <typename T>
std::ostream &operator<<(std::ostream &s, GenericLineKM<T> line) {
  s << "Line y = " << line.getK() << "*x + " << line.getM();
  return s;
}




} /* namespace sail */

#endif /* LINEKM_H_ */
