/*
 * Function.cpp
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#include "Function.h"
#include  "MDArray.h"
#include "math.h"
#include <cmath>
#include <server/common/invalidate.h>

namespace sail {

void Function::evalNumericJacobian(double *Xin, double *JNumOut, double h) {
  int M = outDims();
  int N = inDims();
  Arrayd Fplus(M), Fminus(M);
  MDArray2d Jnum(M, N, JNumOut);

  double oneOver2h = 1.0/(2.0*h);
  for (int j = 0; j < N; j++) {
    double bak = Xin[j];
    Xin[j] = bak + h;
    eval(Xin, Fplus.getData());
    Xin[j] = bak - h;
    eval(Xin, Fminus.getData());
    Xin[j] = bak;

    int offs = j*M;
    for (int i = 0; i < M; i++) {
      JNumOut[offs + i] = oneOver2h*(Fplus[i] - Fminus[i]);
    }
  }
}

double Function::evalScalar(double *Xin) {
  assert(outDims() == 1);
  double f = 0.0;
  InvalidateScalar(&f);
  eval(Xin, &f, nullptr);
  return f;
}

double Function::maxNumJacDif(double *X, double h) {
  int rows = outDims();
  int cols = inDims();
  Arrayd F(rows);
  int count = rows*cols;
  Arrayd J(count);
  Arrayd Jnum(count);

  eval(X, F.ptr(), J.ptr());
  evalNumericJacobian(X, Jnum.ptr(), h);
  double maxv = 0.0;
  for (int i = 0; i < count; i++) {
    maxv = std::max(maxv, std::abs(J[i] - Jnum[i]));
  }
  return maxv;
}

double Function::calcSquaredNorm(double *X, double *Fscratch) {
  Arrayd s;
  if (Fscratch == nullptr) {
    s.create(outDims());
    Fscratch = s.ptr();
  }
  eval(X, Fscratch, nullptr);
  return norm2<double>(outDims(), Fscratch);
}

} /* namespace sail */
