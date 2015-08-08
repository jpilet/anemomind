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
 // The fitting cost, as a function
 // of coefficients
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
    int index = int(floor(xToSample(X[i])));
    if (0 <= index && index < segmentCount) {
      qfs[index] += QuadForm<N, 1>::fitPolynomial(X[i], Y[i]);
    }
  }
  return qfs;
}

template <int N>
void optimize(Arrayd X, Arrayd Y, int sampleCount, LineKM sampleToX) {
  Array<QuadForm<N, 1> > qfs = buildQfs<N>(X, Y, sampleCount, sampleToX);
  Integral1d<QuadForm<N, 1> > itg(qfs);

}









}}



#endif /* SERVER_MATH_PIECEWISEPOLYNOMIALS_H_ */
