/*
 *  Created on: 2014-02-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include "MatExpr.h"

using namespace sail;

TEST(MatExprTest, TestMul) {
  arma::mat A(2, 3);
  arma::mat B(3, 2);
  for (int i = 0; i < 6; i++) {
    A[i] = i;
    B[i] = 3.4*i - 0.99;
  }
  std::shared_ptr<MatExpr> prod(new MatExprProduct(
     std::shared_ptr<MatExpr>(new MatExprDense(A)),
     std::shared_ptr<MatExpr>(new MatExprDense(B))));
  arma::mat X = arma::ones(2, 1);

  arma::mat Y0 = A*B*X;
  arma::mat Y1 = prod->mulWithDense(X);

  MatExprBuilder builder;
  builder.push(A);
  builder.push(B);
  builder.mul();
  arma::mat Y2 = builder.top()->mulWithDense(X);

  const double tol = 1.0e-9;
  EXPECT_NEAR(arma::norm(Y0 - Y1, 2), 0.0, tol);
  EXPECT_NEAR(arma::norm(Y0 - Y2, 2), 0.0, tol);
}


TEST(MatExprTest, TestAdd) {
  arma::mat A(2, 3);
  arma::mat B(2, 3);
  for (int i = 0; i < 6; i++) {
    A[i] = i;
    B[i] = 3.4*i - 0.99;
  }
  std::shared_ptr<MatExpr> sum(new MatExprSum(
     std::shared_ptr<MatExpr>(new MatExprDense(A)),
     std::shared_ptr<MatExpr>(new MatExprDense(B))));
  arma::mat X = arma::ones(3, 1);

  arma::mat Y0 = (A + B)*X;
  arma::mat Y1 = sum->mulWithDense(X);

  MatExprBuilder builder;
  builder.push(A);
  builder.push(B);
  builder.add();
  arma::mat Y2 = builder.top()->mulWithDense(X);

  const double tol = 1.0e-9;
  EXPECT_NEAR(arma::norm(Y0 - Y1, 2), 0.0, tol);
  EXPECT_NEAR(arma::norm(Y0 - Y2, 2), 0.0, tol);
}
