/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SIGNALFIT_H_
#define SIGNALFIT_H_

#include <server/math/Grid.h>
#include <server/math/BandMat.h>
#include <memory>

namespace sail {



template <typename T>
Array<T> fitLineStrip(LineStrip strip, Array<T> regWeights,
    Arrayd X, Arrayd Y) {
  int count = X.size();
  int n = strip.getVertexCount();
  BandMat<T> A(n, n, 2, 2);
  A.setAll(0.0);
  A.addRegs(regWeights);
  MDArray<T, 2> B(n, 1);
  B.setAll(0.0);
  for (int i = 0; i < count; i++) {
    int I[2];
    double W[2];
    strip.makeVertexLinearCombination(X.ptr(i), I, W);
    A.addNormalEq(2, I, W);
    double y = Y[i];
    B(I[0], 0) += W[0]*y;
    B(I[1], 0) += W[1]*y;
  }
  bandMatGaussElimDestructive<T>(&A, &B);
  assert(B.isContinuous());
  return B.getStorage();
}

//Arrayd fitLineStripAutoTune(LineStrip strip, Arrayd initRegs, );

} /* namespace sail */

#endif /* SIGNALFIT_H_ */
