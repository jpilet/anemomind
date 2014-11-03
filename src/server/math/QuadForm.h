/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef QUADFUN_H_
#define QUADFUN_H_

#include <server/common/SymmetricMatrix.h>
#include <algorithm>

namespace sail {

/*
 * Represents a quadratic form
 *
 * X'*P*X - 2*Q*X + R,
 *
 * where P and R are symmetric matrices.
 *
 * This is useful for building least squares problems
 * incrementally. P and R are symmetric matrices.
 */
template <int xDims, int rhsDims, typename T = double>
class QuadForm {
 public:
  typedef QuadForm<xDims, rhsDims, T> ThisType;
  static constexpr int pDims = calcSymmetricMatrixStorageSize(xDims);
  static constexpr int qDims = xDims*rhsDims;
  static constexpr int rDims = calcSymmetricMatrixStorageSize(rhsDims);

  /*
   * Make a QuadForm from
   *
   * |A*X - B|^2
   *
   * where A is a 1*xDims matrix and
   *       B is a 1*rhsDims matrix
   */
  static ThisType makeLsqEq(T *A, T *B) {
    QuadForm dst;
    for (int i = 0; i < xDims; i++) {
      for (int j = 0; j <= i; j++) {
        dst.setP(i, j, A[i]*A[j]);
      }
      for (int j = 0; j < rhsDims; j++) {
        dst.setQ(i, j, A[i]*B[j]);
      }
    }
    for (int i = 0; i < rhsDims; i++) {
      for (int j = 0; j <= i; j++) {
        dst.setR(i, j, B[i]*B[j]);
      }
    }
    return dst;
  }

  ThisType operator+(const ThisType &other) const {
    ThisType dst;
    add(pDims, _P, other._P, dst._P);
    add(qDims, _Q, other._Q, dst._Q);
    add(rDims, _R, other._R, dst._R);
    return dst;
  }

  ThisType scale(T factor) const {
    ThisType dst;
    sail::scale(pDims, factor, _P, dst._P);
    sail::scale(qDims, factor, _Q, dst._Q);
    sail::scale(rDims, factor, _R, dst._R);
    return dst;
  }
 private:
  QuadForm() {}

  void setP(int i, int j, T value) {
    _P[calcSymmetricMatrixIndex(i, j)] = value;
  }

  void setQ(int i, int j, T value) {
    _Q[i + j*xDims] = value;
  }

  void setR(int i, int j, T value) {
    _R[calcSymmetricMatrixIndex(i, j)] = value;
  }

  T _P[pDims];
  T _Q[qDims];
  T _R[rDims];
};


}




#endif /* QUADFUN_H_ */
