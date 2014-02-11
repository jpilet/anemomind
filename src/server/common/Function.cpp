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

double Function::calcSquaredNorm(double *X, double *Fscratch) {
  eval(X, Fscratch, nullptr);
  return norm2<double>(outDims(), Fscratch);
}

} /* namespace sail */
