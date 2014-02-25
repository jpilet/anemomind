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

MatExprProduct::MatExprProduct(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B) : _A(A), _B(B) {
  assert(A->cols() == B->rows());
}

arma::mat MatExprProduct::mulWithDense(arma::mat X) {
  return _A->mulWithDense(_B->mulWithDense(X));
}

MatExprSum::MatExprSum(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B) : _A(A), _B(B) {
  assert(_A->rows() == _B->rows());
  assert(_A->cols() == _B->cols());
}

arma::mat MatExprSum::mulWithDense(arma::mat X) {
  return _A->mulWithDense(X) + _B->mulWithDense(X);
}

namespace {
  int countRows(Array<std::shared_ptr<MatExpr> > M) {
    assert(M.hasData());
    int cols = M[0]->cols();
    int rows = 0;
    for (int i = 0; i < M.size(); i++) {
      rows += M[i]->rows();
      assert(cols == M[i]->cols());
    }
    return rows;
  }
}


MatExprVCat::MatExprVCat(std::shared_ptr<MatExpr> A, std::shared_ptr<MatExpr> B) : _A(A), _B(B) {
  assert(_A->cols() == _B->cols());
}

arma::mat MatExprVCat::mulWithDense(arma::mat X) {
  int m = rows();
  int n = X.n_cols;
  arma::mat Y = arma::zeros(m, n);
  Y.rows(0, _A->rows()-1) = _A->mulWithDense(X);
  Y.rows(_A->rows(), m-1) = _B->mulWithDense(X);
  return Y;
}

MatExprBuilder::MatExprBuilder(int initStackSize) {
  _stack.reserve(initStackSize);
}

void MatExprBuilder::push(arma::mat M) {
  _stack.push_back(std::shared_ptr<MatExpr>(new MatExprDense(M)));
}

void MatExprBuilder::push(arma::sp_mat M) {
  _stack.push_back(std::shared_ptr<MatExpr>(new MatExprSparse(M)));
}

void MatExprBuilder::push(std::shared_ptr<MatExpr> M) {
  _stack.push_back(M);
}

void MatExprBuilder::mul() {
  std::shared_ptr<MatExpr> B = pop();
  std::shared_ptr<MatExpr> A = pop();
  _stack.push_back(std::shared_ptr<MatExpr>(new MatExprProduct(A, B)));
}

void MatExprBuilder::add() {
  std::shared_ptr<MatExpr> B = pop();
  std::shared_ptr<MatExpr> A = pop();
  _stack.push_back(std::shared_ptr<MatExpr>(new MatExprSum(A, B)));
}

void MatExprBuilder::vcat() {
  std::shared_ptr<MatExpr> B = pop();
  std::shared_ptr<MatExpr> A = pop();
  _stack.push_back(std::shared_ptr<MatExpr>(new MatExprVCat(A, B)));
}

std::shared_ptr<MatExpr> MatExprBuilder::top() {
  return _stack.back();
}


std::shared_ptr<MatExpr> MatExprBuilder::pop() {
  std::shared_ptr<MatExpr> x = top();
  _stack.pop_back();
  return x;
}

} /* namespace sail */
