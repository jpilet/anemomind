/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Logo.h"

namespace sail {

Bender::Bender(double x[3]) {
  arma::mat A(3, 3);
  arma::mat B(3, 1);
  double y[3] = {0, 0.5, 1.0};
  for (int i = 0; i < 3; i++) {
    A(i, 0) = sqr(x[i]);
    A(i, 1) = x[i];
    A(i, 2) = 1.0;
    B(i, 0) = y[i];
  }
  arma::mat X = solve(A, B);
  _a = X(0, 0);
  _b = X(1, 0);
  _c = X(2, 0);
}

Logo::Logo(ASettings as,
           GSettings gs) : _A(as), _G(gs),
           _aLine(as.makeALine()) {
  _bender = Bender(-_aLine.getM(), -_aLine.getK(), 0.0);
}





} /* namespace mmm */
