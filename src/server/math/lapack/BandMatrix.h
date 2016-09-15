/*
 * BandMatrix.h
 *
 *  Created on: Apr 22, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_LAPACK_BANDMATRIX_H_
#define SERVER_MATH_LAPACK_BANDMATRIX_H_

#include <server/common/MDArray.h>

namespace sail {

// Is the Eigen matrix compatible?
// Only seems partially implemented... And don't know if
// its storage scheme compatible with that of LAPACK

// A class that abstracts the band matrix used by LAPACK
// as explained here: http://www.netlib.org/lapack/lug/node124.html

/*
 * According to docs:
 *
 * To be precise, aij is stored in AB(ku+1+i-j,j) for
 *   max(1, j - ku) <= i <= min(m, j + kl)
 *
 * But the docs are for Fortran that does 1-based indexing, so we need to rewrite it:
 *
 *   Replace i = i0 + 1, j = j0 + 1, AB(i, j) = AB0(i - 1, j - 1)
 *
 * So we get
 *
 *   ai0j0 = AB0(ku + i0 - j0, j0)
 *
 * with
 *
 *   max(0, j0 - ku) <= i0 <= min(m - 1, j + kl)
 *
 */

template <typename T>
class BandMatrix {
public:
  BandMatrix() : _kl(0), _ku(0), _m(0), _n(0) {}
  BandMatrix(int m, int n, int kl, int ku) : _m(m), _n(n), _kl(kl), _ku(ku),

//     Also allocate some extra space used by the dgbsv,
//     see docs on its AB parameter:
//
//      "AB is DOUBLE PRECISION array, dimension (LDAB,N)
//          On entry, the matrix A in band storage, in rows KL+1 to
//          2*KL+KU+1; rows 1 to KL of the array need not be set.
//          The j-th column of A is stored in the j-th column of the
//          array AB as follows:
//          AB(KL+KU+1+i-j,j) = A(i,j) for max(1,j-KU)<=i<=min(N,j+KL)
//          On exit, details of the factorization: U is stored as an
//          upper triangular band matrix with KL+KU superdiagonals in
//          rows 1 to KL+KU+1, and the multipliers used during the
//          factorization are stored in rows KL+KU+2 to 2*KL+KU+1.
//          See below for further details."
//
// TODO: Since we currently are only using BandMatrix as a way of communicating
//       data to dgbsv, I think it is OK to do allocate extra space here, for now.
//       Maybe we want to refactor this later to a struct, for instance
//       struct DgbsvData {BandMatrix<double> matrix; MDArray2d ABstorage;} or something.

      _storageArrayAB(MDArray<T, 2>(2*kl+ku+1, n)) {
    _AB = _storageArrayAB.sliceRowsFrom(kl);
  }

  void setAll(T value) {
    _storageArrayAB.setAll(value);
  }

  static BandMatrix<T> zero(int m, int n, int kl, int ku) {
    BandMatrix<T> dst(m, n, kl, ku);
    dst.setAll(0.0);
    return dst;
  }

  bool valid(int i0, int j0) const {
    return std::max(0, j0 - _ku) <= i0 && i0 <= std::min(_m - 1, j0 + _kl);
  }

  const T operator() (int i0, int j0) const {
    return _AB(computeI(i0, j0), j0);
  }

  T &operator() (int i0, int j0) {
    return _AB(computeI(i0, j0), j0);
  }

  // Access parameters by their LAPACK names
  int get_m() const {
    return _m;
  }

  int get_n() const {
    return _n;
  }

  int get_kl() const {
    return _kl;
  }

  int get_ku() const {
    return _ku;
  }

  T *get_full_AB() {
    return _storageArrayAB.ptr();
  }

  const T *get_full_AB() const {
    return _storageArrayAB.ptr();
  }

  int get_ldab() const {
    return _storageArrayAB.getStep();
  }

  // Access parameters by their meaningful names
  int rows() const {
    return _m;
  }

  int cols() const {
    return _n;
  }

  int subdiagonalCount() const {
    return _kl;
  }

  int superdiagonalCount() const {
    return _ku;
  }

  bool isSquare() const {
    return _m == _n;
  }

  int computeStride(int rowStep, int colStep) const {
    int inds1[2] = {computeI(rowStep, colStep), colStep};
    int inds0[2] = {computeI(0, 0), 0};
    return _storageArrayAB.calcIndex(inds1)
        - _storageArrayAB.calcIndex(inds0);
  }

  // When taking a horizontal step, from (i, j) to (i, j + 1),
  // what step does it correspond to in the underlying array?
  int horizontalStride() const {
    return computeStride(0, 1);
  }

  int verticalStride() const {
    return computeStride(1, 0);
  }

  T *ptr(int i, int j) {
    int inds[2] = {computeI(i, j), j};
    return _AB.getPtrAt(inds);
  }

  MDArray<T, 2> makeDense() const {
    MDArray<T, 2> dst(_m, _n);
    for (int i = 0; i < _m; i++) {
      for (int j = 0; j < _n; j++) {
        dst(i, j) = valid(i, j)? (*this)(i, j) : 0.0;
      }
    }
    return dst;
  }

  int computeI(int i0, int j0) const {
    return _ku + i0 - j0;
  }
private:

  int _m, _n; // An "m x n" matrix, that is _m rows and _n columns.
  int _kl, _ku; // _kl subdiagonals, _ku superdiagonals


  // The full storage, used by dgbsv
  MDArray<T, 2> _storageArrayAB;

  // A view of that storage where the matrix coefficients are kept
  MDArray<T, 2> _AB;
};

template <typename T>
class SymmetricBandMatrixL {
public:
  static const char uplo = 'L';
  SymmetricBandMatrixL(int n, int kd) : _A(kd+1, n) {}

  SymmetricBandMatrixL<T> zero(int n, int k) {
    auto mat = SymmetricBandMatrixL<T>(n, k);
    mat._A.setAll(T(0.0));
    return mat;
  }

  T &atUnsafe(int i, int j) {
    assert(i >= j);
    return _A(i - j, i);
  }

  const T &atUnsafe(int i, int j) const {
    assert(i >= j);
    return _A(i - j, i);
  }

  const T &getSafe(int i, int j) const {
    if (0 <= i && i < size() && 0 <= j && j < size()) {
      if (abs(i - j) <= kd()) {
        return i < j? atUnsafe(j, i) : atUnsafe(i, j);
      }
    }
    return T(0.0);
  }

  MDArray<T, 2> toDense() const {
    int n = size();
    MDArray<T, 2> dst(n, n);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        dst(i, j) = getSafe(i, j);
      }
    }
    return dst;
  }

  int size() const {
    return _A.cols();
  }

  int kd() const {
    return _A.rows()-1;
  }

  // Useful when building normal equations
  void add(int i, int j, T x) const {
    if (i >= j) {
      atUnsafe(i, j) += x;
    }
  }
private:
  MDArray<T, 2> _A;
};

} /* namespace sail */

#endif /* SERVER_MATH_LAPACK_BANDMATRIX_H_ */
