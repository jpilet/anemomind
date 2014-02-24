/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef MATEXPR_H_
#define MATEXPR_H_

#include <armadillo>
#include <memory>
#include <server/common/Array.h>

namespace sail {


/*
 * MatExpr class. To store matrix expressions at runtime.
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
  virtual int rows() const = 0;
  virtual int cols() const = 0;
  virtual ~MatExpr() {}
};

class MatExprSparse : public MatExpr {
 public:
  MatExprSparse(arma::sp_mat M) : _M(M) {}
  arma::mat mulWithDense(arma::mat X);
  int rows() const {return _M.n_rows;}
  int cols() const {return _M.n_cols;}
 private:
  arma::sp_mat _M;
};

class MatExprDense : public MatExpr {
 public:
  MatExprDense(arma::mat M) : _M(M) {}
  arma::mat mulWithDense(arma::mat X);
  int rows() const {return _M.n_rows;}
  int cols() const {return _M.n_cols;}
 private:
  arma::mat _M;
};

class MatExprProduct : public MatExpr {
 public:
  MatExprProduct(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B);
  arma::mat mulWithDense(arma::mat X);
  int rows() const {return _A->rows();}
  int cols() const {return _B->cols();}
 private:
  std::shared_ptr<MatExpr> _A, _B;
};

class MatExprSum : public MatExpr {
 public:
  MatExprSum(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B);
  arma::mat mulWithDense(arma::mat X);
  int rows() const {return _A->rows();}
  int cols() const {return _A->cols();}
 private:
  std::shared_ptr<MatExpr> _A, _B;
};

// Vertical concatenation of matrices
class MatExprVCat : public MatExpr {
 public:
  MatExprVCat(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B);
  arma::mat mulWithDense(arma::mat X);
  int rows() const {return _A->rows() + _B->rows();}
  int cols() const {return _A->cols();}
 private:
  std::shared_ptr<MatExpr> _A, _B;
};

// Factory for MatExpr to make it easier to build them
class MatExprBuilder {
 public:
  MatExprBuilder(int initStackSize = 30);
  void push(arma::mat M);
  void push(arma::sp_mat M);
  void push(std::shared_ptr<MatExpr> M);
  void mul();
  void add();
  void vcat();
  std::shared_ptr<MatExpr> top();
  int size() {return _stack.size();}
  bool single() {return size() == 1;}
  bool empty() {return _stack.empty();}
 private:
  std::vector<std::shared_ptr<MatExpr> > _stack;
  std::shared_ptr<MatExpr> pop();
};

} /* namespace sail */

#endif /* MATEXPR_H_ */
