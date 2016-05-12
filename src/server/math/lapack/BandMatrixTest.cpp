/*
 * BandMatrixTest.cpp
 *
 *  Created on: Apr 22, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/lapack/BandMatrix.h>
#include <cmath>

using namespace sail;

namespace {
  double a(int i, int j) {
    return std::cos(234.34*i + 234.34) + std::sin(324.4*j + 3443667.0);
  }
}


TEST(BandMatrixTest, www_netlib_org_slash_lapack_slash_lug_slash_node124_html) {
  BandMatrix<double> mat(5, 5, 2, 1);
  for (int i = 1; i <= 5; i++) {
    int i0 = i - 1;
    for (int j = 1; j <= 5; j++) {
      int j0 = j - 1;
      int diff = i0 - j0;
      bool expectedValidity = -1 <= diff && diff <= 2;
      EXPECT_EQ(expectedValidity, mat.valid(i0, j0));

      if (mat.valid(i0, j0)) {
        auto x = a(i, j);
        mat(i0, j0) = x;
        EXPECT_EQ(x, mat(i0, j0));
      }
    }
  }

  EXPECT_EQ(mat.get_m(), 5);
  EXPECT_EQ(mat.get_n(), 5);
  EXPECT_EQ(mat.get_kl(), 2);
  EXPECT_EQ(mat.get_ku(), 1);

  EXPECT_EQ(mat.rows(), 5);
  EXPECT_EQ(mat.cols(), 5);
  EXPECT_EQ(mat.subdiagonalCount(), 2);
  EXPECT_EQ(mat.superdiagonalCount(), 1);
}

TEST(BandMatrixTest, NonSquare) {
  BandMatrix<double> mat(3, 4, 1, 1);
  EXPECT_EQ(mat.get_m(), 3);
  EXPECT_EQ(mat.get_n(), 4);
  EXPECT_EQ(mat.get_kl(), 1);
  EXPECT_EQ(mat.get_ku(), 1);

  EXPECT_EQ(mat.rows(), 3);
  EXPECT_EQ(mat.cols(), 4);
}


TEST(BandMatrixTest, PtrAccessAndDense) {
  auto mat = BandMatrix<double>::zero(5, 5, 1, 1);

  mat(2, 2) = 1.0;
  mat(2, 3) = 2.0;
  mat(3, 2) = 3.0;
  mat(3, 3) = 4.0;

  double *offset = mat.ptr(2, 2);
  int step = mat.horizontalStride();

  std::function<double*(int, int)> at = [&](int i, int j) {
    return offset + i + j*step;
  };

  EXPECT_EQ(1.0, (*at(0, 0)));
  EXPECT_EQ(2.0, (*at(0, 1)));
  EXPECT_EQ(3.0, (*at(1, 0)));
  EXPECT_EQ(4.0, (*at(1, 1)));


  auto dense = mat.makeDense();
  EXPECT_EQ(dense.rows(), 5);
  EXPECT_EQ(dense.cols(), 5);
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      auto x = dense(i, j);
      if ((i == 2 || i == 3) && (j == 2 || j == 3)) {
        EXPECT_LT(0, x);
      } else {
        EXPECT_EQ(0, x);
      }
    }
  }
}


