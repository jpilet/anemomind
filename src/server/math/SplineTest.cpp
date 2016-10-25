/*
 * SplineTest.cpp
 *
 *  Created on: 12 Oct 2016
 *      Author: jonas
 */

#include <server/math/Spline.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(SplineBasisFunctionTest, TestIt) {
  SplineBasisFunction<double, 1> s1;
  EXPECT_EQ(s1.polynomialBoundary(0), -0.5);
  EXPECT_EQ(s1.polynomialBoundary(1), 0.5);

  EXPECT_EQ(s1.pieceIndex(-0.6), -1);
  EXPECT_EQ(s1.pieceIndex(-0.4), 0);
  EXPECT_EQ(s1.pieceIndex(0.6), 1);

  EXPECT_EQ(s1(-0.6), 0.0);
  EXPECT_EQ(s1(-0.4), 1.0);
  EXPECT_EQ(s1(0.4), 1.0);
  EXPECT_EQ(s1(0.6), 0.0);

  SplineBasisFunction<double, 2> s2;
  EXPECT_NEAR(s2(-30.0), 0.0, 1.0e-6);
  EXPECT_NEAR(s2(-1.0), 0.0, 1.0e-6);
  EXPECT_NEAR(s2(-0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s2(0.0), 1.0, 1.0e-6);
  EXPECT_NEAR(s2(0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s2(1.0), 0.0, 1.0e-6);
  EXPECT_NEAR(s2(30.0), 0.0, 1.0e-6);

  SplineBasisFunction<double, 3> s3;
  EXPECT_NEAR(s3(0.0), 0.5*1 + 2*((0.5*0.5)/2.0), 1.0e-6);
  EXPECT_NEAR(s3(-0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s3(0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s3(-1.0), 0.5*0.5*0.5, 1.0e-6);
}

TEST(SplineBasisTest, TestBasis) {
  {
    auto x = SplineBasis<double, 0>(1);
    EXPECT_EQ(0, x.leftMostCoefIndex());
    EXPECT_EQ(1, x.rightMostCoefIndex());
    EXPECT_EQ(2, x.coefCount());
  }{
    auto x = SplineBasis<double, 0>(2);
    EXPECT_EQ(0, x.leftMostCoefIndex());
    EXPECT_EQ(2, x.rightMostCoefIndex());
    EXPECT_EQ(3, x.coefCount());
  }
}
