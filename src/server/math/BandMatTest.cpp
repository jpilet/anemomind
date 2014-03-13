/*
 *  Created on: 2014-03-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/BandMat.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(BandMatTest, TestAssign) {
  int n = 9;
  double coefs[2] = {-1, 1};
  MDArray2d testmat(n, n);
  testmat.setAll(0.0);
  BandMatd bandmat(n, n, 1, 1);
  bandmat.setAll(0.0);
  EXPECT_EQ(bandmat.leftColIndex(0), 0);
  EXPECT_EQ(bandmat.rightColIndex(0), 2);
  EXPECT_EQ(bandmat.topRowIndex(0), 0);
  EXPECT_EQ(bandmat.bottomRowIndex(0), 2);
  EXPECT_EQ(bandmat.leftColIndex(1), 0);
  EXPECT_EQ(bandmat.rightColIndex(1), 3);
  EXPECT_EQ(bandmat.topRowIndex(1), 0);
  EXPECT_EQ(bandmat.bottomRowIndex(1), 3);
  EXPECT_EQ(bandmat.leftColIndex(2), 1);
  EXPECT_EQ(bandmat.topRowIndex(2), 1);

  for (int k = 0; k < n-1; k++) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        testmat(k+i, k+j) += coefs[i]*coefs[j];
        testmat(k+i, k+j) += coefs[i]*coefs[j];
        bandmat(k+i, k+j) += coefs[i]*coefs[j];
        bandmat(k+i, k+j) += coefs[i]*coefs[j];
      }
    }
  }
  MDArray2d testmat2 = bandmat.toDense();
  EXPECT_EQ(testmat2.rows(), testmat.rows());
  EXPECT_EQ(testmat2.cols(), testmat.cols());
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      EXPECT_NEAR(testmat(i, j), testmat2(i, j), 1.0e-6);
    }
  }
}

TEST(BandMatTest, TestLU) {
  double Adata[9] = {0.5377  ,  0.8622   ,      0 ,   1.8339  ,  0.3188  ,  0.3426  ,       0 ,  -1.3077  ,  3.5784};
  BandMatd A(3, 3, 1, 1);
  MDArray2d B(3, 1);
  int counter = 0;
  for (int i = 0; i < 3; i++) {
    B(i, 0) = i+1;
    for (int j = 0; j < 3; j++) {
      double a = Adata[counter];
      if (a != 0) {
        A(i, j) = a;
      }
      counter++;
    }
  }
  double expectedResult[3] = {0.7693 , 0.6801 ,   1.0869};
  bandMatGaussElim(&A, &B, 1.0e-9);
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(B(i, 0), expectedResult[i], 1.0e-2);
  }
}


