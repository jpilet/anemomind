/*
 * DgbsvWrapper.cpp
 *
 *  Created on: Apr 22, 2016
 *      Author: jonas
 */

#include <server/math/lapack/DgbsvWrapper.h>
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
}

namespace sail {

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
