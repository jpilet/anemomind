/*
 * Polynomial.h
 *
 *  Created on: 16 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_POLYNOMIAL_H_
#define SERVER_MATH_POLYNOMIAL_H_

#include <assert.h>
#include <initializer_list>
#include <iostream>

namespace sail {

inline constexpr int cmin(int a, int b) {
  return a < b? a : b;
}

inline constexpr int cmax(int a, int b) {
  return a < b? b : a;
}

template <typename T, int CoefCount>
class Polynomial {
public:
  static const int Degree = CoefCount-1;
  typedef Polynomial<T, CoefCount> ThisType;

  Polynomial(T x) {
    static_assert(1 <= CoefCount, "Too few coefficients");
    _coefs[0] = x;
    for (int i = 1; i < CoefCount; i++) {
      _coefs[i] = 0.0;
    }
  }

  Polynomial(const std::initializer_list<T> &coefs) {
    assert(coefs.size() <= CoefCount);
    int i = 0;
    for (auto c: coefs) {
      _coefs[i] = c;
      i++;
    }
    for (; i < CoefCount; i++) {
      _coefs[i] = 0.0;
    }
    assert(i == CoefCount);
  }

  Polynomial() {}

  static Polynomial<T, CoefCount> zero() {
    Polynomial<T, CoefCount> dst;
    for (int i = 0; i < CoefCount; i++) {
      dst[i] = 0.0;
    }
    return dst;
  }

  typedef Polynomial<T, CoefCount+1> Primitive;
  Primitive primitive() const {
    Primitive dst;
    dst[0] = 0.0;
    for (int i = 0; i < CoefCount; i++) {
      int nextDeg = i+1;
      dst[nextDeg] = _coefs[i]/nextDeg;
    }
    return dst;
  }

  typedef Polynomial<T, cmax(0, CoefCount-1)> Derivative;
  Derivative derivative() const {
    Derivative dst;
    for (int i = 0; i < CoefCount-1; i++) {
      int k = i+1;
      dst[i] = _coefs[k]*k;
    }
    return dst;
  }

  ThisType derivativeSameOrder() const {
    ThisType dst = ThisType::zero();
    for (int i = 0; i < CoefCount-1; i++) {
      int k = i+1;
      dst[i] = _coefs[k]*k;
    }
    return dst;
  }

  T &operator[] (int i) {return _coefs[i];}
  const T &operator[] (int i) const {return _coefs[i];}

  template <int M>
  Polynomial<T, M + CoefCount - 1> operator*(const Polynomial<T, M> &r) const {
    auto dst = Polynomial<T, M + CoefCount - 1>::zero();
    for (int i = 0; i < CoefCount; i++) {
      for (int j = 0; j < M; j++) {
        dst[i + j] += _coefs[i]*r[j];
      }
    }
    return dst;
  }

  Polynomial<T, CoefCount> operator+(const Polynomial<T, CoefCount> &other) const {
    Polynomial<T, CoefCount> dst;
    for (int i = 0; i < CoefCount; i++) {
      dst[i] = _coefs[i] + other[i];
    }
    return dst;
  }

  Polynomial<T, CoefCount> operator-(const Polynomial<T, CoefCount> &other) const {
    Polynomial<T, CoefCount> dst;
    for (int i = 0; i < CoefCount; i++) {
      dst[i] = _coefs[i] - other[i];
    }
    return dst;
  }

  Polynomial<T, CoefCount> scaleBy(T s) const {
    Polynomial<T, CoefCount> dst;
    for (int i = 0; i < CoefCount; i++) {
      dst[i] = s*_coefs[i];
    }
    return dst;
  }

  template <int M>
  Polynomial<T, M> resize() const {
    auto dst = Polynomial<T, M>::zero();
    for (int i = 0; i < cmin(CoefCount, M); i++) {
      dst[i] = _coefs[i];
    }
    return dst;
  }

  bool operator==(const ThisType &other) const {
    for (int i = 0; i < CoefCount; i++) {
      if (!(_coefs[i] == other._coefs[i])) {
        return false;
      }
    }
    return true;
  }

  T eval(T x) const {
    T result = T(0.0);
    for (int i_ = 0; i_ < CoefCount; i_++) {
      int i = CoefCount-1 - i_;
      result = result*x + _coefs[i];
    }
    return result;
  }
private:
  T _coefs[CoefCount];
};

template <typename T, int CoefCount>
Polynomial<T, CoefCount> operator*(T a, const Polynomial<T, CoefCount> &b) {
  return b.scaleBy(a);
}

template <typename Ewise, typename T, int M, int N>
struct EwisePoly {
  static const int K = cmax(M, N);
  static Polynomial<T, K> apply(
      const Polynomial<T, M> &a, const Polynomial<T, N> &b) {
    return EwisePoly<Ewise, T, K, K>::apply(
        a.template resize<K>(),
        b.template resize<K>());
  }
};

template <typename Ewise, typename T, int M>
struct EwisePoly<Ewise, T, M, M> {
  static Polynomial<T, M> apply(
      const Polynomial<T, M> &a,
      const Polynomial<T, M> &b) {
    Polynomial<T, M> c;
    for (int i = 0; i < M; i++) {
      c[i] = Ewise::template apply<T>(i, a[i], b[i]);
    }
    return c;
  }
};

struct CoefAdd {
  template <typename T>
  static T apply(int i, T a, T b) {
    return a + b;
  }
};

struct CoefSub {
  template <typename T>
  static T apply(int i, T a, T b) {
    return a - b;
  }
};


template <typename T, int M, int N>
Polynomial<T, cmax(M, N)> operator+(
    const Polynomial<T, M> &a,
    const Polynomial<T, N> &b) {
  return EwisePoly<CoefAdd, T, M, N>::apply(a, b);
}

template <typename T, int M, int N>
Polynomial<T, cmax(M, N)> operator-(
    const Polynomial<T, M> &a,
    const Polynomial<T, N> &b) {
  return EwisePoly<CoefSub, T, M, N>::apply(a, b);
}

// Given two polynomials, P(x) and Q(x), compute the polynomial
// R(x) = P(Q(x)).
template <typename T, int M, int N>
Polynomial<T, (M-1)*(N-1) + 1> eval(
    const Polynomial<T, M> &a,
    const Polynomial<T, N> &b) {
  const int K = (M-1)*(N-1) + 1;
  auto result = Polynomial<T, K>::zero();
  for (int i_ = 0; i_ < M; i_++) {
    int i = M - 1 - i_;
    auto k = (b*result) + a[i];
    result = k.template resize<K>();
  }
  return result;
}

template <typename T, int N>
std::ostream &operator<<(std::ostream &s,
    const Polynomial<T, N> &p) {
  for (int i = 0; i < N; i++) {
    s << "+ " << p[i] << "*x^" << i;
  }
  return s;
}

}
#endif /* SERVER_MATH_POLYNOMIAL_H_ */
