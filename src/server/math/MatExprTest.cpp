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

TEST(MatExprTest, TestVCat) {
  arma::mat A(2, 3);
  arma::mat B(2, 3);
  for (int i = 0; i < 6; i++) {
    A[i] = i;
    B[i] = 3.4*i - 0.99;
  }
  std::shared_ptr<MatExpr> cat(new MatExprVCat(
     std::shared_ptr<MatExpr>(new MatExprDense(A)),
     std::shared_ptr<MatExpr>(new MatExprDense(B))));
  arma::mat X = arma::ones(3, 1);

  arma::mat Y0 = (arma::join_cols(A, B))*X;
  arma::mat Y1 = cat->mulWithDense(X);

  MatExprBuilder builder;
  builder.push(A);
  builder.push(B);
  builder.vcat();
  arma::mat Y2 = builder.top()->mulWithDense(X);

  const double tol = 1.0e-9;
  EXPECT_NEAR(arma::norm(Y0 - Y1, 2), 0.0, tol);
  EXPECT_NEAR(arma::norm(Y0 - Y2, 2), 0.0, tol);
}

namespace {
  void makePowerOf2(MatExprBuilder &builder, int deg) {
    if (deg == 0) {
      builder.push(arma::speye(5, 5));
    } else {
      builder.push(arma::sp_mat(2.0*arma::speye(5, 5)));
      makePowerOf2(builder, deg-1);
      builder.mul();
    }
  }
}

TEST(MatExprTest, Power2) {
  int deg = 8;
  MatExprBuilder builder;
  makePowerOf2(builder, deg);
  EXPECT_EQ(builder.size(), 1);
  arma::mat Y = builder.top()->mulWithDense(arma::ones(5, 1));
  for (int i = 0; i < 5; i++) {
    EXPECT_NEAR(Y[i], 256.0, 1.0e-9);
  }
}

namespace {
  void makeSumOf2(MatExprBuilder &builder, int deg) {
    if (deg > 0) {
      builder.push(arma::sp_mat(2.0*arma::speye(5, 5)));
      builder.add();
      makeSumOf2(builder, deg-1);
    }
  }
}

TEST(MatExprTest, Sum2) {
  int deg = 8;
  MatExprBuilder builder;

  builder.push(arma::zeros(5, 5));
  makeSumOf2(builder, deg);

  EXPECT_EQ(builder.size(), 1);
  arma::mat Y = builder.top()->mulWithDense(arma::ones(5, 1));
  for (int i = 0; i < 5; i++) {
    EXPECT_NEAR(Y[i], 16.0, 1.0e-9);
  }
}

namespace {
  void makeRepeater(MatExprBuilder &builder, int count, int dim) {
    builder.push(arma::sp_mat(1.0*arma::speye(dim, dim)));
    if (count > 1) {
      makeRepeater(builder, count-1, dim);
      builder.vcat();
    }
  }
}

TEST(MatExprTest, Repeat) {
  int count = 3;
  const int dim = 3;

  MatExprBuilder builder;
  makeRepeater(builder, count, dim);

  double xdata[dim] = {0.0, 1.0, 2.0};
  arma::mat X(xdata, dim, 1, false, true);

  EXPECT_EQ(builder.size(), 1);
  arma::mat Y = builder.top()->mulWithDense(X);
  int elemCount = count*dim;
  EXPECT_EQ(Y.n_rows, elemCount);

  for (int i = 0; i < elemCount; i++) {
    EXPECT_NEAR(Y[i], i % dim, 1.0e-9);
  }
}
