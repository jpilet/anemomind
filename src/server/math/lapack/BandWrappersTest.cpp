/*
 * BandedWrappersTest.cpp
 *
 *  Created on: 15 Sep 2016
 *      Author: jonas
 */

#include <server/math/lapack/BandWrappers.h>
#include <server/math/lapack/BandWrappersJet.h>
#include <gtest/gtest.h>
#include <server/common/ArrayIO.h>

using namespace sail;

TEST(DgbsvWrapperTest, TestSolve1x1) {

  BandMatrix<double> A(1, 1, 0, 0);
  A(0, 0) = 2.0;

  MDArray2d B(1, 1);
  B(0, 0) = 9.0;

  EXPECT_TRUE(easyGbsv(&A, &B));
  EXPECT_NEAR(B(0, 0), 4.5, 1.0e-6);
}

TEST(DgbsvWrapperTest, TestSolveDiagMultiRHS) {

  BandMatrix<double> A(3, 3, 0, 0);
  A(0, 0) = 2.0;
  A(1, 1) = 4.0;
  A(2, 2) = 8.0;

  MDArray2d B(3, 2);
  B(0, 0) = 9.0;
  B(1, 0) = 89.0;
  B(2, 0) = -30.0;

  B(0, 1) = 1.0;
  B(1, 1) = 2.0;
  B(2, 1) = 3.0;


  EXPECT_TRUE(easyGbsv(&A, &B));

  EXPECT_NEAR(B(0, 0), 4.5, 1.0e-6);
  EXPECT_NEAR(B(1, 0), 22.25, 1.0e-6);
  EXPECT_NEAR(B(2, 0), -3.75, 1.0e-6);

  EXPECT_NEAR(B(0, 1), 0.5, 1.0e-6);
  EXPECT_NEAR(B(1, 1), 0.5, 1.0e-6);
  EXPECT_NEAR(B(2, 1), 0.375, 1.0e-6);
}

TEST(DgbsvWrapperTest, ThickBand) {
  BandMatrix<double> A(3, 3, 1, 1);
  A(0, 0) = 1; A(0, 1) = 2;
  A(1, 0) = 4; A(1, 1) = 3; A(1, 2) = 5;
               A(2, 1) = 7; A(2, 2) = 3;

  MDArray2d B(3, 1);
  B(0, 0) = 5;
  B(1, 0) = 25;
  B(2, 0) = 23;


  EXPECT_TRUE(easyGbsv(&A, &B));

  EXPECT_NEAR(B(0, 0), 1.0, 1.0e-6);
  EXPECT_NEAR(B(1, 0), 2.0, 1.0e-6);
  EXPECT_NEAR(B(2, 0), 3.0, 1.0e-6);
}

TEST(PbsvTest, SymMat) {
  auto A = SymmetricBandMatrixL<double>::zero(4, 2);
  A.atUnsafe(0, 0) = 2.0;
  A.atUnsafe(1, 1) = 3.0;
  A.atUnsafe(2, 2) = 4.0;
  A.atUnsafe(3, 3) = 5.0;
  auto Ad = A.makeDense();
  EXPECT_EQ(Ad.rows(), 4);
  EXPECT_EQ(Ad.cols(), 4);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      EXPECT_EQ(i == j? i+2.0 : 0, Ad(i, j));
    }
  }
  A.atUnsafe(3, 2) = 119;
  EXPECT_EQ(A.getSafe(3, 2), 119.0);
  EXPECT_EQ(A.getSafe(2, 3), 119.0);

  A.add(3, 2, 4.0);
  EXPECT_EQ(A.getSafe(3, 2), 123.0);

  A.add(2, 3, 4.0);
  EXPECT_EQ(A.getSafe(3, 2), 123.0); // (no change)

  EXPECT_EQ(A.kd(), 2);
  EXPECT_EQ(A.size(), 4);
}

template <typename T>
void addReg(int at, SymmetricBandMatrixL<T> *A) {
  double coeffs[3] = {1, -2, 1};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      A->add(at + i, at + j, T(coeffs[i]*coeffs[j]));
    }
  }
}

template <typename T>
void addDataFit(T weight, int index, T value,
    SymmetricBandMatrixL<T> *A, MDArray<T, 2> *B) {
  T w2 = weight*weight;
  A->add(index, index, w2);
  (*B)(index, 0) += w2*value;
}

template <typename T>
MDArray<T, 2> computeLineFit(int n, T middleWeight) {
  auto A = SymmetricBandMatrixL<T>::zero(n, 2);
  MDArray<T, 2> B(n, 1);
  B.setAll(T(0.0));
  for (int i = 0; i < n-2; i++) {
    addReg(i, &A);
  }
  T w = T(1000.0);
  addDataFit<T>(w, 0, T(3.0), &A, &B);
  addDataFit<T>(middleWeight, n/2, T(9.0), &A, &B);
  addDataFit<T>(w, n-1, T(3.0), &A, &B);
  EXPECT_TRUE(Pbsv<T>::apply(&A, &B));
  return B;
}

TEST(PbsvTest, FitLine) {
  int n = 31;
  auto B = computeLineFit<double>(n, 1000);
  EXPECT_NEAR(B(0, 0), 3.0, 1.0e-3);
  EXPECT_NEAR(B(n/2, 0), 9.0, 1.0e-3);
  EXPECT_NEAR(B(n-1, 0), 3.0, 1.0e-3);
}

MDArray2d differentiateLineFit(int n, double middleWeight) {
  double h = 1.0e-5;
  MDArray2d Bplus = computeLineFit(n, middleWeight+h);
  MDArray2d Bminus = computeLineFit(n, middleWeight-h);
  MDArray2d B(n, 1);
  double f = 1.0/(2.0*h);
  for (int i = 0; i < n; i++) {
    B(i, 0) = f*(Bplus(i, 0) - Bminus(i, 0));
  }
  return B;
}

TEST(PbsvTest, LineFitWithDerivative) {
  int n = 31;

  double weight = 1.0;
  auto X = computeLineFit(n, weight);
  auto dX = differentiateLineFit(n, weight);

  ceres::Jet<double, 1> adWeight(weight);
  adWeight.v[0] = 1.0;
  auto adX = computeLineFit(n, adWeight);

  for (int i = 0; i < n; i++) {
    auto x = X(i, 0);
    auto dx = dX(i, 0);
    auto adx = adX(i, 0);
    EXPECT_NEAR(x, adx.a, 1.0e-4);
    EXPECT_NEAR(dx, adx.v[0], 1.0e-4);
  }
}

TEST(PbsvTest, LineFitWithDerivative2) {
  int n = 31;

  double a = 3.4;
  double b = 9.4;

  double A = 12;
  double B = 13;

  double weight = A*a + B*b;
  auto X = computeLineFit(n, weight);
  auto dX = differentiateLineFit(n, weight);

  ceres::Jet<double, 2> adA(a);
  adA.v[0] = 1.0;
  ceres::Jet<double, 2> adB(b);
  adA.v[1] = 1.0;

  ceres::Jet<double, 2> adWeight = A*adA + B*adB;

  auto adX = computeLineFit(n, adWeight);

  for (int i = 0; i < n; i++) {
    auto x = X(i, 0);
    auto dx = dX(i, 0);
    auto adx = adX(i, 0);
    EXPECT_NEAR(x, adx.a, 1.0e-4);
  }
}
