/*
 * BandedWrappers.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include "BandedWrappers.h"
#include <server/common/logging.h>

// Documented here:
// http://www.netlib.org/lapack/explore-html-3.4.2/d3/d49/group__double_g_bsolve.html#gafa35ce1d7865b80563bbed6317050ad7

extern "C" {
typedef int integer;
typedef double doublereal;
extern int dgbsv_(integer *n, integer *kl, integer *ku, integer *
  nrhs, doublereal *ab, integer *ldab, integer *ipiv, doublereal *b,
  integer *ldb, integer *info);

// www.netlib.org/clapack/old/double/dpbsv.c

int dpbsv_(char *uplo, integer *n, integer *kd, integer *
  nrhs, doublereal *ab, integer *ldab, doublereal *b, integer *ldb,
  integer *info);
/*  -- LAPACK driver routine (version 3.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       March 31, 1993


    Purpose
    =======

    DPBSV computes the solution to a real system of linear equations
       A * X = B,
    where A is an N-by-N symmetric positive definite band matrix and X
    and B are N-by-NRHS matrices.

    The Cholesky decomposition is used to factor A as
       A = U**T * U,  if UPLO = 'U', or
       A = L * L**T,  if UPLO = 'L',
    where U is an upper triangular band matrix, and L is a lower
    triangular band matrix, with the same number of superdiagonals or
    subdiagonals as A.  The factored form of A is then used to solve the
    system of equations A * X = B.

    Arguments
    =========

    UPLO    (input) CHARACTER*1
            = 'U':  Upper triangle of A is stored;
            = 'L':  Lower triangle of A is stored.

    N       (input) INTEGER
            The number of linear equations, i.e., the order of the
            matrix A.  N >= 0.

    KD      (input) INTEGER
            The number of superdiagonals of the matrix A if UPLO = 'U',
            or the number of subdiagonals if UPLO = 'L'.  KD >= 0.

    NRHS    (input) INTEGER
            The number of right hand sides, i.e., the number of columns
            of the matrix B.  NRHS >= 0.

    AB      (input/output) DOUBLE PRECISION array, dimension (LDAB,N)
            On entry, the upper or lower triangle of the symmetric band
            matrix A, stored in the first KD+1 rows of the array.  The
            j-th column of A is stored in the j-th column of the array AB
            as follows:
            if UPLO = 'U', AB(KD+1+i-j,j) = A(i,j) for max(1,j-KD)<=i<=j;
            if UPLO = 'L', AB(1+i-j,j)    = A(i,j) for j<=i<=min(N,j+KD).
            See below for further details.

            On exit, if INFO = 0, the triangular factor U or L from the
            Cholesky factorization A = U**T*U or A = L*L**T of the band
            matrix A, in the same storage format as A.

    LDAB    (input) INTEGER
            The leading dimension of the array AB.  LDAB >= KD+1.

    B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)
            On entry, the N-by-NRHS right hand side matrix B.
            On exit, if INFO = 0, the N-by-NRHS solution matrix X.

    LDB     (input) INTEGER
            The leading dimension of the array B.  LDB >= max(1,N).

    INFO    (output) INTEGER
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value
            > 0:  if INFO = i, the leading minor of order i of A is not
                  positive definite, so the factorization could not be
                  completed, and the solution has not been computed.

    Further Details
    ===============

    The band storage scheme is illustrated by the following example, when
    N = 6, KD = 2, and UPLO = 'U':

    On entry:                       On exit:

        *    *   a13  a24  a35  a46      *    *   u13  u24  u35  u46
        *   a12  a23  a34  a45  a56      *   u12  u23  u34  u45  u56
       a11  a22  a33  a44  a55  a66     u11  u22  u33  u44  u55  u66

    Similarly, if UPLO = 'L' the format of A is as follows:

    On entry:                       On exit:

       a11  a22  a33  a44  a55  a66     l11  l22  l33  l44  l55  l66
       a21  a32  a43  a54  a65   *      l21  l32  l43  l54  l65   *
       a31  a42  a53  a64   *    *      l31  l42  l53  l64   *    *

 *
 */
int dpbsv_(char *uplo, integer *n, integer *kd, integer *
    nrhs, doublereal *ab, integer *ldab, doublereal *b, integer *ldb,
    integer *info);
}

namespace sail {

template <typename T>
int pbsv(char *uplo, integer *n, integer *kd, integer *
    nrhs, T *ab, integer *ldab, T *b, integer *ldb,
    integer *info) {return -1;}

template <>
int pbsv<double>(char *uplo, integer *n, integer *kd, integer *nrhs,
    doublereal *ab, integer *ldab, doublereal *b, integer *ldb,
integer *info) {
  return dpbsv_(uplo, n, kd, nrhs, ab, ldab, b, ldb, info);
}

template <typename T>
int performPbsv(SymmetricBandMatrixL<T> *lhs, MDArray<T, 2> *rhs) {
  char uplo = lhs->uplo;
  int n = lhs->size();
  int kd = lhs->kd();
  int nrhs = rhs->cols();
  T *ab = lhs->ab();
  int ldab = lhs->ldab();
  T *b = rhs->ptr();
  int ldb = rhs->getStepAlongDim(1);
  int info = 0;
  pbsv<T>(&uplo, &n, &kd, &nrhs, ab, &ldab, b, &ldb, &info);
  return info;
}

template <typename T>
bool easyPbsv(SymmetricBandMatrixL<T> *lhs, MDArray<T, 2> *rhs) {
  int info = performPbsv(lhs, rhs);
  if (info == 0) {
    return true;
  } else if (info < 0) {
    const char *argNames[] = {"uplo", "n", "kd", "nrhs", "ab", "ldab", "b",
      "ldb", "info"};
    LOG(ERROR) << "The argument '" << argNames[(-info)-1] << "' had an illegal value";
    return false;
  } else if (info > 0) {
    int i = info-1;
    LOG(ERROR) << "the leading minor of order " << i << " of A is not"
                      "positive definite, so the factorization could not be"
                      "completed, and the solution has not been computed.";
    return false;
  }
}

void testCallIt() {
  char c;
  int i;
  double d;
  dpbsv_(&c, &i, &i, &i, &d, &i, &d, &i, &i);
}

template <typename T>
int performDgbsv(BandMatrix<T> *lhs, MDArray<T, 2> *rhs, Arrayi *ipiv) {
  CHECK(lhs != nullptr);
  CHECK(rhs != nullptr);
  CHECK(ipiv != nullptr);
  CHECK(lhs->isSquare());
  CHECK(lhs->rows() == rhs->rows());
  CHECK(lhs->rows() == ipiv->size());

  int info = 0;
  int n = lhs->rows();
  int kl = lhs->get_kl();
  int ku = lhs->get_ku();
  int nrhs = rhs->getCols();
  int ldab = lhs->get_ldab();
  int ldb = rhs->getStep();

  dgbsv_(&n, &kl, &ku, &nrhs, lhs->get_full_AB(),
      &ldab, ipiv->ptr(), rhs->ptr(), &ldb, &info);

  return info;
}

template <typename T>
bool genericEasyDgbsvInPlace(BandMatrix<T> *lhs, MDArray<T, 2> *rhs) {
  Arrayi ipiv = Arrayi::fill(lhs->rows(), -1);
  auto info = performDgbsv(lhs, rhs, &ipiv);

  if (info == 0) {
    return true;
  } else if (info < 0) {
    const char *argNames[] = {
        "n", "kl", "ku", "nrhs", "ab", "ldab", "ipiv", "b",
          "ldb", "info"};
    LOG(ERROR) << "The argument '" << argNames[(-info)-1] << "' had an illegal value";
    return false;
  } else {
    LOG(ERROR) << "U(" << info << ", " << info << ") is exactly zero.  The factorization\n"
          "has been completed, but the factor U is exactly \n"
          "singular, and the solution has not been computed.\n";
    return false;
  }
}

bool easyDgbsvInPlace(BandMatrix<double> *lhs, MDArray2d *rhs) {
  return genericEasyDgbsvInPlace<double>(lhs, rhs);
}

} /* namespace sail */
