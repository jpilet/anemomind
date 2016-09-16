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
void fillDerivatives(
    const SymmetricBandMatrixL<ceres::Jet<T, N> > &A,
    const MDArray<T, 2> &X,
    const MDArray<ceres::Jet<T, N>, 2> &B,
    MDArray<T, 2> *dst) {
  int n = A.size();
  assert(n == X.rows());
  assert(n == B.rows());
  assert(X.cols() == 1);
  assert(B.cols() == 1);
  assert(dst->cols() == N);
  int kd = A.kd();
  for (int derivativeIndex = 0; derivativeIndex < N; derivativeIndex++) {
    for (int i = 0; i < n; i++) {
      int upper = std::min(n, i + kd + 1);
      T sum = B(i, 0).v[derivativeIndex];
      for (int j = i; j < upper; j++) {
        sum -= A.atUnsafe(j, i).v[derivativeIndex]*X(j, 0);
      }
      (*dst)(i, derivativeIndex) = sum;
    }
  }
}

template <typename T, int N>
void putBackResult(
    const MDArray<T, 2> &X,
    const MDArray<T, 2> &DX,
    MDArray<ceres::Jet<T, N>, 2> *dst) {
  int n = DX.rows();
  assert(1 == X.cols());
  assert(N == DX.cols());

  T *a = X.ptr();
  T *v[N];
  for (int i = 0; i < N; i++) {
    v[i] = DX.getPtrAt(0, i);
  }

  for (int i = 0; i < n; i++) {
    ceres::Jet<T, N> x(a[i]);
    for (int j = 0; j < N; j++) {
      x.v[j] = v[j][i];
    }
    (*dst)(i, 0) = x;
  }
}

template <typename T, int N>
SymmetricBandMatrixL<T> getScalarBandMatrix(
    const SymmetricBandMatrixL<ceres::Jet<T, N>> &A) {
  return SymmetricBandMatrixL<T>(A.storage().map(
      [](const ceres::Jet<T, N> &x) {return x.a;}));
}

template <typename T, int N>
MDArray<T, 2> getScalarRhs(const MDArray<ceres::Jet<T, N>, 2> &A) {
  return A.map([](const ceres::Jet<T, N> &x) {return x.a;});
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

     // TODO: Multiple right-hand-sides should work, but has not been tested:
    assert(rhs->cols() == 1);

    typedef ceres::Jet<T, N> ADType;
    int n = lhs->size();
    assert(rhs->rows() == n);
    int kd = lhs->kd();
    int cols = rhs->cols();

    // Prepare the matrices
    auto X = getScalarRhs(*rhs);
    {
      auto A = getScalarBandMatrix(*lhs);
      if (!Pbsv<T>::apply(&A, &X)) {
        return false;
      }
    }
    MDArray<T, 2> DX(n, cols*N);
    for (int j = 0; j < cols; j++) {
      ADType *col = rhs->getPtrAt(0, j);
      auto dxSub = DX.sliceColBlock(j, N);
      fillDerivatives<T, N>(*lhs, X.sliceCol(j),
          rhs->sliceCol(j), &dxSub);
    }
    {
      auto A = getScalarBandMatrix(*lhs);
      if (!Pbsv<T>::apply(&A, &DX)) {
        return false;
      }
    }
    putBackResult<T, N>(X, DX, rhs);

    return true;
  }
};

}




#endif /* SERVER_MATH_LAPACK_BANDWRAPPERSJET_H_ */
