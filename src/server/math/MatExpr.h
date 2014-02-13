/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MATEXPR_H_
#define MATEXPR_H_

namespace sail {


/*
 * MatExpr class. To store matrix expressions implicitly.
 *
 * Why? :
 * Suppose we want to precompute a matrix A*B and then multiply it with a column vector X.
 *
 * If A is n*1 and B is 1*n and X is n*1, precomputing (A*B) and then multiplying (A*B)*X is O(n²)
 * whereas A*(B*X) is O(n). With this class A*B is not evaluated explicitly.
 */
class MatExpr {
public:
  MatExpr() {}
  arma::mat mulWithDense(arma::mat X) = 0;
  virtual ~MatExpr() {}
};

} /* namespace sail */

#endif /* MATEXPR_H_ */
