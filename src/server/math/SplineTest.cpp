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

TEST(RawSplineBasisTest, TestBasis) {
  {
    auto x = RawSplineBasis<double, 0>(1);
    EXPECT_EQ(-0.5, x.lowerDataBound());
    EXPECT_EQ(0.5, x.upperDataBound());
    EXPECT_EQ(1, x.coefCount());
    EXPECT_NEAR(0.0, x.basisLocation(0), 1.0e-6);
  }{
    auto x = RawSplineBasis<double, 0>(2);
    EXPECT_EQ(-0.5, x.lowerDataBound());
    EXPECT_EQ(1.5, x.upperDataBound());
    EXPECT_EQ(2, x.coefCount());
    EXPECT_NEAR(1.0, x.basisLocation(1), 1.0e-6);
  }{
    auto x = RawSplineBasis<double, 1>(1);
    EXPECT_EQ(-0.5, x.lowerDataBound());
    EXPECT_EQ(0.5, x.upperDataBound());
    EXPECT_EQ(2, x.coefCount());
    EXPECT_NEAR(0.5, x.basisLocation(1), 1.0e-6);

    {
      auto k = x.build(0.0);
      EXPECT_EQ(k.inds[0], 0);
      EXPECT_EQ(k.inds[1], 1);
      EXPECT_EQ(k.weights[0], 0.5);
      EXPECT_EQ(k.weights[1], 0.5);
    }{
      auto l = x.build(0.500001);
      EXPECT_EQ(l.inds[0], 0);
      EXPECT_EQ(l.inds[1], 1);
      EXPECT_NEAR(l.weights[0], 0.0, 1.0e-4);
      EXPECT_NEAR(l.weights[1], 1.0, 1.0e-4);
    }{
      auto l = x.build(40.0);
      EXPECT_EQ(l.inds[0], 0);
      EXPECT_EQ(l.inds[1], 1);
      EXPECT_EQ(l.weights[0], 0.0);
      EXPECT_EQ(l.weights[1], 0.0);
    }
  }
}
