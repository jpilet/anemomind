/*
 * BandedLUTest.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include <server/math/BandedLU.h>
#include <gtest/gtest.h>

using namespace sail;
using namespace sail::BandedLU;

TEST(BandedLU, DiagonalSolve) {
  const int n = 3;
  double diag[n] = {3, 2, 5};
  double bdata[n] = {4, 7, 11};
  auto A = BandMatrix<double>::zero(n, n, 0, 0);
  for (int i = 0; i < n; i++) {
    A(i, i) = diag[i];
  }
  EXPECT_TRUE(hasValidShape(A));
  {
    auto Ad = A.makeDense();
    EXPECT_EQ(Ad.rows(), n);
    EXPECT_EQ(Ad.cols(), n);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        EXPECT_NEAR(Ad(i, j), i == j? diag[i] : 0.0, 1.0e-6);
      }
    }
  }
  EXPECT_EQ(computeBlockCount(4, 2), 3);
  EXPECT_EQ(computeBlockCount(8, 5), 4);
  EXPECT_EQ(getDiagonalBlockCount(A), 3);

  MDArray2d B(n, 1);
  for (int i = 0; i < n; i++) {
    B(i, 0) = bdata[i];
  }
  EXPECT_TRUE(forwardEliminate(&A, &B));

  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(A(i, i), diag[i], 1.0e-9);
    EXPECT_NEAR(B(i, 0), bdata[i], 1.0e-9);
  }

  EXPECT_TRUE(backwardSubstitute(&A, &B));
  for (int i = 0; i < n; i++) {
    EXPECT_NEAR(A(i, i), 1.0, 1.0e-6);
    EXPECT_NEAR(B(i, 0), bdata[i]/diag[i], 1.0e-6);
  }
}
