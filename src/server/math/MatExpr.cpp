/*
 *  Created on: 13 févr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "MatExpr.h"

namespace sail {

arma::mat MatExprSparse::mulWithDense(arma::mat X) {
  return _M*X;
}

arma::mat MatExprDense::mulWithDense(arma::mat X) {
  return _M*X;
}

arma::mat MatExprProduct::mulWithDense(arma::mat X) {
  return _A->mulWithDense(_B->mulWithDense(X));
}

arma::mat MatExprSum::mulWithDense(arma::mat X) {
  return _A->mulWithDense(X) + _B->mulWithDense(X);
}

} /* namespace sail */
