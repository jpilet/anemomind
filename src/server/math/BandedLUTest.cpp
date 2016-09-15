/*
 * BandedLUTest.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include <iostream>
#include <server/common/ArrayIO.h>
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

#define EAVAS(X) std::cout << #X << ": " << X << std::endl;

TEST(BandedLU, GeneralSolve) {
  auto A = BandMatrix<double>::zero(4, 4, 1, 2);
  EAVAS(A.computeI(0, 0));
  EAVAS(A.computeI(1, 0));
  EAVAS(A.computeI(0, 1));
  EAVAS(A.computeI(1, 1));
  A(0, 0) = 1.0; A(0, 1) = 2.0;
  A(1, 0) = 2.4; A(1, 1) = 5.0; A(1, 2) = 7.0;
  A(2, 1) = 3.0; A(2, 2) = 3.0; A(2, 3) = 1.4;
  A(3, 2) = 4.5; A(3, 3) = 1.0;


  MDArray2d B(4, 2);
  B(0, 0) = 3.0; B(0, 1) = 2.0;
  B(1, 0) = 9.0; B(1, 1) = 2.0;
  B(2, 0) = 4.0; B(2, 1) = 5.0;
  B(3, 0) = 3.0; B(3, 1) = 5.0;

  std::cout << "A before is\n" << A.makeDense() << std::endl;
  std::cout << "B before is\n" << B << std::endl;

  EXPECT_TRUE(solveInPlace(&A, &B));
  //EXPECT_TRUE(forwardEliminate(&A, &B));

  std::cout << "A after is\n" << A.makeDense() << std::endl;
  std::cout << "B after is\n" << B << std::endl;

  double expected[4*2] = {
      2.58079,   4.14589,
      0.20960,  -1.07295,
      0.25115,  -0.36934,
      1.86981,   6.66205};


  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 2; j++) {
      EXPECT_NEAR(expected[2*i + j], B(i, j), 1.0e-3);
    }
  }
}
