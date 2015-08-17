/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nnls.h>
#include <cmath>
#include <iostream>
#include <server/common/string.h>

extern "C" {
// Declaration of function from nnls.h
typedef int integer;
typedef double doublereal;
/* Subroutine */ int nnls_(doublereal *a, integer *mda,
    integer *m, integer *n, doublereal *b, doublereal *x,
    doublereal *rnorm, doublereal *w, doublereal *zz,
    integer *index, integer *mode);
}

namespace sail {

namespace {
  NNLS::StatusCode modeCodeToStatusCode(int mode) {
    assert(1 <= mode && mode <= 3);
    return NNLS::StatusCode(mode - 1);
  }

  template <typename T>
  T dup(T x, bool perform) {
    return (perform? x.dup() : x);
  }
}

NNLS NNLS::solve(MDArray2d A, Arrayd B, bool safe) {
  // When using safe mode, input arrays are not modified.
  A = dup(A, safe);
  B = dup(B, safe);

  int MDA = A.getInternalSize(0);
  int M = A.rows();
  int N = A.cols();

  if (M != B.size()) {
    return NNLS(Arrayd(), BadDimensions);
  }

  Arrayd X(N);
  double RNORM = NAN;
  Arrayd W(N);
  Arrayd ZZ(M);
  Arrayi INDEX(N);
  int MODE = 0;
  nnls_(A.ptr(), &MDA, &M, &N, B.ptr(), X.ptr(), &RNORM, W.ptr(), ZZ.ptr(), INDEX.ptr(), &MODE);
  return NNLS(X, modeCodeToStatusCode(MODE));
}

}
