/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MATEXPR_H_
#define MATEXPR_H_

#include <armadillo>
#include <memory>

namespace sail {


/*
 * MatExpr class. To store matrix expressions implicitly.
 *
 * Why? :
 * Suppose we want to precompute a matrix A*B and then multiply it with a column vector X.
 *
 * If A is n*1 and B is 1*n and X is n*1, precomputing (A*B) and then multiplying (A*B)*X is O(n²)
 * whereas A*(B*X) is O(n). With this class A*B is not evaluated explicitly which lets us evaluate
 * A*(B*X) internally.
 */
class MatExpr {
public:
  virtual arma::mat mulWithDense(arma::mat X) = 0;
  virtual ~MatExpr() {}
};

class MatExprSparse : public MatExpr {
 public:
  MatExprSparse(arma::sp_mat M) : _M(M) {}
  arma::mat mulWithDense(arma::mat X);
 private:
  arma::sp_mat _M;
};

class MatExprDense : public MatExpr {
 public:
  MatExprDense(arma::mat M) : _M(M) {}
  arma::mat mulWithDense(arma::mat X);
 private:
  arma::mat _M;
};

class MatExprProduct : public MatExpr {
 public:
  MatExprProduct(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B) : _A(A), _B(B) {}
  arma::mat mulWithDense(arma::mat X);
 private:
  std::shared_ptr<MatExpr> _A, _B;
};

class MatExprSum : public MatExpr {
 public:
  MatExprSum(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B) : _A(A), _B(B) {}
  arma::mat mulWithDense(arma::mat X);
 private:
  std::shared_ptr<MatExpr> _A, _B;
};

} /* namespace sail */

#endif /* MATEXPR_H_ */
