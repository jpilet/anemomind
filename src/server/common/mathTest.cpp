/*
 *  Created on: 2014-03-31
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/math.h>
#include <limits>

using namespace sail;

TEST(MathTest, NanBehavesAsExpected) {
  double a = std::numeric_limits<double>::signaling_NaN();
  double b = std::numeric_limits<double>::signaling_NaN();
  double c = 1.4;
  EXPECT_FALSE(a == b);
  EXPECT_FALSE(a == c);

  EXPECT_TRUE(strictEquality(a, b));
  EXPECT_FALSE(strictEquality(a, c));
  EXPECT_TRUE(strictEquality(c, c));
}

TEST(MathTest, PositiveModTest) {
  EXPECT_EQ(5 % 6, 5);
  EXPECT_EQ(11 % 6, 5);
  EXPECT_EQ(positiveMod(5, 6), 5);
  EXPECT_EQ(positiveMod(11, 6), 5);
  EXPECT_NE(-1 % 6, 5);
  EXPECT_EQ(positiveMod(-1, 6), 5);
  EXPECT_EQ(positiveMod(0, 6), 0);
}

TEST(MathTest, PosModFloatingPoint) {
  EXPECT_EQ(positiveMod(71.5, 360.0), 71.5);
  EXPECT_EQ(positiveMod(-3.4, 360.0), 360.0 - 3.4);
}

TEST(MathTest, QuadTest) {
  double a = 3.34;
  double b = 9.18;

  double ax, bx;
  solveQuadratic(1.0, -a-b, a*b, &ax, &bx);
  EXPECT_NEAR(a, std::min(ax, bx), 1.0e-6);
  EXPECT_NEAR(b, std::max(ax, bx), 1.0e-6);
}

TEST(MathTest, TriBasis) {

  // Some high dimension, just for fun...
  const int N = 9;

  double a[N], b[N];
  makeTriBasisVector(N, 3, a);
  makeTriBasisVector(N, 7, b);
  EXPECT_NEAR(norm(9, a), 1.0, 1.0e-6);
  EXPECT_NEAR(norm(9, b), 1.0, 1.0e-6);
  EXPECT_NEAR((normdif<double, 9>(a, b)), 1.0, 1.0e-6);
}

