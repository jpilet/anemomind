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
  auto s1 = SplineBasisFunction<double, 1>::make();
  EXPECT_EQ(s1.polynomialBoundary(0), -0.5);
  EXPECT_EQ(s1.polynomialBoundary(1), 0.5);

  EXPECT_EQ(s1.pieceIndex(-0.6), -1);
  EXPECT_EQ(s1.pieceIndex(-0.4), 0);
  EXPECT_EQ(s1.pieceIndex(0.6), 1);

  EXPECT_EQ(s1(-0.6), 0.0);
  EXPECT_EQ(s1(-0.4), 1.0);
  EXPECT_EQ(s1(0.4), 1.0);
  EXPECT_EQ(s1(0.6), 0.0);

  auto s2 = SplineBasisFunction<double, 2>::make();
  EXPECT_NEAR(s2(-30.0), 0.0, 1.0e-6);
  EXPECT_NEAR(s2(-1.0), 0.0, 1.0e-6);
  EXPECT_NEAR(s2(-0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s2(0.0), 1.0, 1.0e-6);
  EXPECT_NEAR(s2(0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s2(1.0), 0.0, 1.0e-6);
  EXPECT_NEAR(s2(30.0), 0.0, 1.0e-6);

  auto s3 = SplineBasisFunction<double, 3>::make();
  EXPECT_NEAR(s3(0.0), 0.5*1 + 2*((0.5*0.5)/2.0), 1.0e-6);
  EXPECT_NEAR(s3(-0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s3(0.5), 0.5, 1.0e-6);
  EXPECT_NEAR(s3(-1.0), 0.5*0.5*0.5, 1.0e-6);
}

TEST(RawSplineBasisTest, TestBasis) {
  {
    auto b = RawSplineBasis<double, 0>(1);
    EXPECT_EQ(b.coefCount(), 1);
  }{
    auto b = RawSplineBasis<double, 1>(1);
    EXPECT_EQ(b.coefCount(), 3);
    EXPECT_NEAR(b.basisLocation(0), -1.0, 1.0e-6);
    EXPECT_NEAR(b.basisLocation(1),  0.0, 1.0e-6);
    EXPECT_NEAR(b.basisLocation(2),  1.0, 1.0e-6);
    {
      auto w = b.build(0.0);
      EXPECT_EQ(w.inds[0], 0);
      EXPECT_EQ(w.inds[1], 1);
      EXPECT_EQ(w.inds[2], 2);
      EXPECT_NEAR(w.weights[0], 0.0, 1.0e-6);
      EXPECT_NEAR(w.weights[1], 1.0, 1.0e-6);
      EXPECT_NEAR(w.weights[2], 0.0, 1.0e-6);
    }{
      auto w = b.build(-0.6);
      EXPECT_EQ(w.inds[0], 0);
      EXPECT_EQ(w.inds[1], 1);
      EXPECT_EQ(w.inds[2], 2);
      EXPECT_NEAR(w.weights[0], 0.6, 1.0e-3);
      EXPECT_NEAR(w.weights[1], 0.4, 1.0e-3);
      EXPECT_NEAR(w.weights[2], 0.0, 1.0e-3);
    }{
      auto w = b.build(-1.0);
      EXPECT_EQ(w.inds[0], 0);
      EXPECT_EQ(w.inds[1], 1);
      EXPECT_EQ(w.inds[2], 2);
      EXPECT_NEAR(w.weights[0], 1.0, 1.0e-3);
      EXPECT_NEAR(w.weights[1], 0.0, 1.0e-3);
      EXPECT_NEAR(w.weights[2], 0.0, 1.0e-3);
    }{
      auto w = b.build(-1.5);
      EXPECT_EQ(w.inds[0], 0);
      EXPECT_EQ(w.inds[1], 1);
      EXPECT_EQ(w.inds[2], 2);
      EXPECT_NEAR(w.weights[0], 0.5, 1.0e-3);
      EXPECT_NEAR(w.weights[1], 0.0, 1.0e-3);
      EXPECT_NEAR(w.weights[2], 0.0, 1.0e-3);
    }{
      auto w = b.build(1.5);
      EXPECT_EQ(w.inds[0], 0);
      EXPECT_EQ(w.inds[1], 1);
      EXPECT_EQ(w.inds[2], 2);
      EXPECT_NEAR(w.weights[0], 0.0, 1.0e-3);
      EXPECT_NEAR(w.weights[1], 0.0, 1.0e-3);
      EXPECT_NEAR(w.weights[2], 0.5, 1.0e-3);
    }{
      double coefs[3] = {0, 1, 2};
      EXPECT_NEAR(b.evaluate(coefs, -1.0), 0.0, 1.0e-6);
      EXPECT_NEAR(b.evaluate(coefs, -0.5), 0.5, 1.0e-6);
      EXPECT_NEAR(b.evaluate(coefs, 0.0), 1.0, 1.0e-6);
      EXPECT_NEAR(b.evaluate(coefs, 0.5), 1.5, 1.0e-6);
      EXPECT_NEAR(b.evaluate(coefs, 1.0), 2.0, 1.0e-6);
    }
  }{
    auto b = RawSplineBasis<double, 2>(1);
    EXPECT_EQ(b.coefCount(), 3);
  }{
    auto b = RawSplineBasis<double, 3>(1);
    EXPECT_EQ(b.coefCount(), 5);
  }{
    auto b = RawSplineBasis<double, 3>(2);
    EXPECT_EQ(b.coefCount(), 6);
  }
}


TEST(SplineBasisTest, TestIndices) {
  BoundaryIndices inds(Spani(0, 5), Spani(2, 7), 2);
  EXPECT_EQ(inds.leftDim(), 4);
  EXPECT_EQ(inds.totalDim(), 7);
}
