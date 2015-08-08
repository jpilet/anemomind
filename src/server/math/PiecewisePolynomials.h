/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_PIECEWISEPOLYNOMIALS_H_
#define SERVER_MATH_PIECEWISEPOLYNOMIALS_H_

#include <server/math/Integral1d.h>
#include <server/common/Span.h>
#include <server/math/QuadForm.h>

namespace sail { namespace PiecewisePolynomials {




template <int N>
struct Piece {
 // The quadratic cost of fitting to data
 // w.r.t. polynomial coefficients.
 QuadForm<N, 1> quadCost;

 // The span over which it is fitted
 Spani span;
};

template <int N>
Array<QuadForm<N, 1> > buildQfs(Arrayd X, Arrayd Y, int sampleCount, LineKM sampleToX) {
  int n = X.size();
  assert(n == Y.size());
  int segmentCount = sampleCount - 1;
  LineKM xToSample = sampleToX.makeInvFun();
  Array<QuadForm<N, 1> > qfs(segmentCount);
  for (int i = 0; i < n; i++) {

  }
}

// Fit piecewise polynomials of degree N to the data.
template <int N>
void optimize(Arrayd X, Arrayd Y, int sampleCount, LineKM sampleToX) {
  Array<QuadForm<N, 1> > qfs = buildQfs(X, Y, sampleCount, sampleToX);
}









}}



#endif /* SERVER_MATH_PIECEWISEPOLYNOMIALS_H_ */
