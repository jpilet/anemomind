/*
 * Spline.h
 *
 *  Created on: 12 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_SPLINE_H_
#define SERVER_MATH_SPLINE_H_

#include <assert.h>
#include <initializer_list>

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

  Polynomial(const std::initializer_list<T> &coefs) {
    int i = 0;
    for (auto c: coefs) {
      _coefs[i] = c;
      i++;
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

  Polynomial<T, CoefCount+1> primitive() const {
    Polynomial<T, CoefCount+1> dst;
    dst[0] = 0.0;
    for (int i = 0; i < CoefCount; i++) {
      int nextDeg = i+1;
      dst[nextDeg] = _coefs[i]/nextDeg;
    }
    return dst;
  }

  T &operator[] (int i) {return _coefs[i];}
  const T &operator[] (int i) const {return _coefs[i];}

  template <int M>
  Polynomial<T, M + CoefCount - 1> operator*(const Polynomial<T, M> &r) const {
    auto dst = Polynomial<T, M + CoefCount>::zero();
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
  Polynomial<T, K> apply(
      const Polynomial<T, M> &a, const Polynomial<T, N> &b) {
    return EwisePoly<Ewise, T, K, K>::apply(a.resize(K), b.resize(K));
  }
};

template <typename Ewise, typename T, int M>
struct EwisePoly<Ewise, T, M, M> {
  Polynomial<T, M> apply(
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

/*template <typename T, int M, int N>
Polynomial<T, M*N> eval(const Polynomial<T, M> &a,
    const Polynomial<T, N> &b) const {
  auto result = Polynomial<T, M*N>::zero();
  for (int i = 0; i < M; i++) {
    result +=
  }
}*/


} /* namespace sail */

#endif /* SERVER_MATH_SPLINE_H_ */
