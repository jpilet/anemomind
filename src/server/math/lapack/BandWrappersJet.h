/*
 * BandedWrappersJet.h
 *
 *  Created on: 16 Sep 2016
 *      Author: jonas
 */

#include <server/math/lapack/BandWrappers.h>
#include <ceres/jet.h>

#ifndef SERVER_MATH_LAPACK_BANDWRAPPERSJET_H_
#define SERVER_MATH_LAPACK_BANDWRAPPERSJET_H_

namespace sail {

template <typename T, int N>
void populateBValues(int n, const ceres::Jet<T, N> *src, T *dst) {
  for (int i = 0; i < n; i++) {
    dst[i] = src[i].a;
  }
}

template <typename T, int N>
void fillDerivatives(int n, int derivativeIndex,
    const SymmetricBandMatrixL<ceres::Jet<T, N> > &lhs,
    const ceres::Jet<T, N> *rhs, T *dst) {
  for (int i = 0; i < n; i++) {
    T sum = rhs[i].v[derivativeIndex];
    int upper = i + lhs.kd() + 1;
    for (int j = i; i < upper; i++) {
      sum -= lhs.atUnsafe(j, i).v[derivativeIndex]*rhs[j].a;
    }
    dst[i] = sum;
  }
}

template <typename T, int N>
void putBackResult(const MDArray<T, 2> &src, ceres::Jet<T, N> *dst) {
  int n = src.rows();
  assert(1 + N == src.cols());

  T *a = src.ptr();
  T *v[N];
  for (int i = 0; i < N; i++) {
    v[N] = src.getPtrAt(0, i+1);
  }

  for (int i = 0; i < n; i++) {
    ceres::Jet<T, N> x(a[i]);
    for (int j = 0; j < N; j++) {
      x.v[j] = v[j][i];
    }
    dst[i] = dst;
  }
}

/*
 *
 * Specialization for ceres::Jet<T, N>
 *
 * Since lapack only provides pbsv for some static types, we need
 * this specialization in order to perform pbsv on ceres::Jet. To
 * differentiate the solution of a linear system A*X = B, we do
 *
 * D(A*X) = D(B) <=> D(A)*X + A*D(X) = D(B) <=>
 * A*D(X) = D(B) - D(A)*X <=> D(X) = A\(D(B) - D(A)*X)
 *
 */
template <typename T, int N>
struct Pbsv<ceres::Jet<T, N> > {
   static bool apply(
      SymmetricBandMatrixL<ceres::Jet<T, N> > *lhs,
      MDArray<ceres::Jet<T, N>, 2> *rhs) {
    typedef ceres::Jet<T, N> ADType;
    int n = lhs->size();
    assert(rhs->rows() == n);
    int kd = lhs->kd();
    int cols = rhs->cols();
    int colsPerCol = 1 + N;

    // Prepare the matrices
    auto A = SymmetricBandMatrixL<T>(
        lhs->storage().map([](const ADType &x) {return x.a;}));

    // First solve X:
    MDArray<T, 2> Bx(n, cols);

    MDArray<T, 2> B(n, cols*colsPerCol);
    for (int j = 0; j < cols; j++) {
      ADType *col = rhs->getPtrAt(0, j);
      int colOffset = j*colsPerCol;
      populateBValues<T, N>(
          n, col, B.getPtrAt(0, colOffset));
      for (int i = 0; i < N; i++) {
        fillDerivatives<T, N>(n, i, *lhs, col,
            B.getPtrAt(0, colOffset + 1 + i));
      }
    }

    if (!Pbsv<T>::apply(&A, &B)) {
      return false;
    }

    // Put the result back
    for (int j = 0; j < cols; j++) {
      putBackResult(B.sliceColBlock(j, N+1), rhs->getPtrAt(0, j));
    }

    return true;
  }
};

}




#endif /* SERVER_MATH_LAPACK_BANDWRAPPERSJET_H_ */
