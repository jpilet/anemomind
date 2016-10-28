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
  EXPECT_EQ(inds.varDim(), 4);
  EXPECT_EQ(inds.totalDim(), 7);
  for (int i = 0; i < 7; i++) {
    EXPECT_EQ(i < 2, inds.isLeftIndex(i));
    EXPECT_EQ(2 <= i && i < 5, inds.isInnerIndex(i));
    EXPECT_EQ(5 <= i, inds.isRightIndex(i));
  }
  EXPECT_EQ(0, inds.computeACol(0));
  EXPECT_EQ(1, inds.computeACol(1));
  EXPECT_EQ(2, inds.computeACol(5));
  EXPECT_EQ(3, inds.computeACol(6));

  EXPECT_EQ(0, inds.computeBCol(2));
  EXPECT_EQ(1, inds.computeBCol(3));
  EXPECT_EQ(2, inds.computeBCol(4));
}

TEST(SplineBasisTest, TestIndices2) {
  BoundaryIndices inds(Spani(0, 5), Spani(7, 12), 2);
  EXPECT_EQ(inds.totalDim(), 10);
  EXPECT_EQ(0, inds.computeACol(0));
  EXPECT_EQ(1, inds.computeACol(1));
  EXPECT_EQ(2, inds.computeACol(10));
  EXPECT_EQ(3, inds.computeACol(11));

  EXPECT_EQ(0, inds.computeBCol(2));
  EXPECT_EQ(1, inds.computeBCol(3));
  EXPECT_EQ(2, inds.computeBCol(4));
  EXPECT_EQ(3, inds.computeBCol(7));
  EXPECT_EQ(4, inds.computeBCol(8));
  EXPECT_EQ(5, inds.computeBCol(9));
}

TEST(SplineBasisTest, SmoothBoundary) {
  {
    SmoothBoundarySplineBasis<double, 0> basis(1);
    EXPECT_EQ(basis.coefCount(), 1);
  }{
    SmoothBoundarySplineBasis<double, 1> basis(1);
    auto left = basis.leftMat();
    EXPECT_EQ(left.rows(), 1);
    EXPECT_EQ(left.cols(), 1);
    EXPECT_NEAR(left(0, 0), 1.0, 1.0e-6);
  }{
    SmoothBoundarySplineBasis<double, 3> basis(1);
    {
      double coefs[1] = {1.0};
      for (int i = 0; i < basis.internalCoefCount(); i++) {
        std::cout << "  Coef " << i << ": "
            << basis.getInternalCoef(i, coefs) << std::endl;
      }
    }{
      double coefs[1] = {3.4};
      EXPECT_NEAR(basis.evaluate(coefs, 0.1), 3.4, 1.0e-6);
      EXPECT_NEAR(basis.evaluate(coefs, -0.4), 3.4, 1.0e-6);
    }
  }{
    SmoothBoundarySplineBasis<double, 3> basis(2);
    {
      double coefs[2] = {2.0, 3.0};
      EXPECT_NEAR(basis.evaluate(coefs, -0.5), 1.5, 1.0e-6);
      EXPECT_NEAR(basis.evaluate(coefs, 0.0), 2.0, 1.0e-6);
      EXPECT_NEAR(basis.evaluate(coefs, 0.5), 2.5, 1.0e-6);
      EXPECT_NEAR(basis.evaluate(coefs, 1.0), 3.0, 1.0e-6);
      EXPECT_NEAR(basis.evaluate(coefs, 1.5), 3.5, 1.0e-6);
    }
  }{
    SmoothBoundarySplineBasis<double, 1> basis(2);
    double coefs[2] = {2.0, 3.0};
    EXPECT_NEAR(basis.evaluate(coefs, -0.5), 2.0, 1.0e-6);
    EXPECT_NEAR(basis.evaluate(coefs, 0.0), 2.0, 1.0e-6);
    EXPECT_NEAR(basis.evaluate(coefs, 0.5), 2.5, 1.0e-6);
    EXPECT_NEAR(basis.evaluate(coefs, 1.0), 3.0, 1.0e-6);
    EXPECT_NEAR(basis.evaluate(coefs, 1.5), 3.0, 1.0e-6);

    auto weights = basis.build(0.5);
  }
}
