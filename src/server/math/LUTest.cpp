/*
 *  Created on: 2014-03-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/string.h>
#include "LU.h"

using namespace sail;

TEST(LUTest, SimpleLUTest) {
  double adata[3*3] = { 0.9649  ,  0.1576  ,  0.9706   , 0.9572   , 0.4854  ,  0.8003  ,  0.1419  ,  0.4218   , 0.9157};
  arma::mat A(adata, 3, 3, false, true);
  arma::mat X(3, 1);

  for (int i = 0; i < 3; i++) {
    X(i, 0) = i;
  }

  arma::mat B = A*X;

  arma::mat X2 = solveLU(A, B);
  EXPECT_NEAR(arma::norm(A*X2 - B, 2), 0.0, 1.0e-6);
}

TEST(LUTest, ManyColRightHandSide) {
  double adata[3*3] = { 0.9649  ,  0.1576  ,  0.9706   , 0.9572   , 0.4854  ,  0.8003  ,  0.1419  ,  0.4218   , 0.9157};
  arma::mat A(adata, 3, 3, false, true);
  arma::mat X(3, 2);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      X(i, j) = i + j;
    }
  }

  arma::mat B = A*X;

  arma::mat X2 = solveLU(A, B);
  EXPECT_NEAR(arma::norm(A*X2 - B, 2), 0.0, 1.0e-6);
  for (int i = 0; i < X.n_rows; i++) {
    for (int j = 0; j < X.n_cols; j++) {
      EXPECT_NEAR(X(i, j), X2(i, j), 1.0e-6);
    }
  }
}
