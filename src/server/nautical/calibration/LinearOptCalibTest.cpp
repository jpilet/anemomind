/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/calibration/LinearOptCalib.h>
#include <server/common/string.h>
#include <iostream>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/nautical/calibration/LinearCalibration.h>

using namespace sail;
using namespace LinearOptCalib;

Array<Nav> getPsarosTestData() {
  auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("psaros33_Banque_Sturdza")
    .pushDirectory("2014")
    .pushDirectory("20140821").get();
  return scanNmeaFolder(p, Nav::debuggingBoatId());
}

Eigen::MatrixXd arrayToMatrix(MDArray2d src) {
  Eigen::MatrixXd dst(src.rows(), src.cols());
  for (int i = 0; i < src.rows(); i++) {
    for (int j = 0; j < src.cols(); j++) {
      dst(i, j) = src(i, j);
    }
  }
  return dst;
}

Eigen::VectorXd arrayToVector(MDArray2d src) {
  assert(src.cols() == 1);
  int n = src.rows();
  Eigen::VectorXd dst(n, 1);
  for (int i = 0; i < n; i++) {
    dst(i) = src(i, 0);
  }
  return dst;
}

struct EData {
  Eigen::MatrixXd A;
  Eigen::VectorXd B;
};

EData toEData(Array<Nav> navs) {
  LinearCalibration::FlowSettings settings;
  auto matrices = LinearCalibration::makeTrueWindMatrices(navs, settings);
  return EData{arrayToMatrix(matrices.A), arrayToVector(matrices.B)};
}

bool isOrthonormal(Eigen::MatrixXd X, double tol = 1.0e-6) {
  Eigen::MatrixXd K = X.transpose()*X;
  if (K.rows() == K.cols()) {
    for (int i = 0; i < K.rows(); i++) {
      for (int j = 0; j < K.cols(); j++) {
        double k = K(i, j);
        double l = (i == j? 1 : 0);
        if (std::abs(k - l) > tol) {
          return false;
        }
      }
    }
    return true;
  }
  return false;
}

TEST(LinearOptCalib, OrthoDense) {
  int m = 30;
  int n = 9;
  Eigen::MatrixXd A(m, n);
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      A(i, j) = sin(exp(0.1*(i + j)));
    }
  }
  auto B = orthonormalBasis(A);
  EXPECT_TRUE(isOrthonormal(B));
}

TEST(LinearOptCalib, MatrixTest) {
  auto edata = toEData(getPsarosTestData());
}

TEST(LinearOptCalib, OverlappingSpanTest) {
  auto spans = makeOverlappingSpans(4, 2, 0.5);
  EXPECT_EQ(spans.size(), 3);
  EXPECT_EQ(spans[0], Spani(0, 2));
  EXPECT_EQ(spans[1], Spani(1, 3));
  EXPECT_EQ(spans[2], Spani(2, 4));
}
