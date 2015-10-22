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
#include <Eigen/Eigenvalues>
#include <server/common/ArrayIO.h>

using namespace sail;
using namespace LinearOptCalib;

Eigen::MatrixXd projector(Eigen::MatrixXd A) {
  Eigen::MatrixXd AtA = A.transpose()*A;
  return A*AtA.inverse()*A.transpose();
}


double getMinValue(const Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> &s) {
  double value = std::numeric_limits<double>::infinity();
  for (int i = 0; i < s.eigenvalues().size(); i++) {
    auto x = s.eigenvalues()(i);
    if (x < value) {
      value = x;
    }
  }
  return value;
}

// Distance between the orthonormal subspaces spanned by the columns
// of A and B, respectively. A distance of 0 means the subspaces are the same.
double subspaceDistance(Eigen::MatrixXd A, Eigen::MatrixXd B) {
  auto aProj = projector(A);
  auto bProj = projector(B);
  auto dif = aProj - bProj;
  auto K = dif.transpose()*dif;
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> s(K, false);
  return getMinValue(s);
}

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
  int n;
};

EData toEData(Array<Nav> navs) {
  LinearCalibration::FlowSettings settings;
  auto matrices = LinearCalibration::makeTrueWindMatrices(navs, settings);
  assert(isEven(matrices.A.rows()));
  return EData{arrayToMatrix(matrices.A), arrayToVector(matrices.B), matrices.A.rows()/2};
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

Array<Eigen::VectorXd> splitCols(const Eigen::MatrixXd &x) {
  Array<Eigen::VectorXd> y(x.cols());
  for (int j = 0; j < x.cols(); j++) {
    Eigen::VectorXd yj(x.rows());
    for (int i = 0; i < x.rows(); i++) {
      yj(j) = x(i, j);
    }
    y[j] = yj;
  }
  return y;
}

TEST(LinearOptCalib, OrthoDenseAndPerVector) {
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

  auto vecs = splitCols(A);
}

TEST(LinearOptCalib, SparseVectorTest) {
  SparseVector a(4, Array<SparseVector::Entry>{
    SparseVector::Entry{0, 1.0},
    SparseVector::Entry{1, 2.0}
  });
  SparseVector b(4, Array<SparseVector::Entry>{
    SparseVector::Entry{2, 3.0},
    SparseVector::Entry{3, 4.0}
  });
  EXPECT_EQ(dot(a, a), squaredNorm(a));
  EXPECT_EQ(dot(b, b), squaredNorm(b));
  EXPECT_EQ(0, dot(a, b));
  EXPECT_EQ((a + b).nnz(), 4);
  EXPECT_EQ((a - b).nnz(), 4);
}

SparseVector makeTestVector(int index) {
  int dim = 12;
  Array<SparseVector::Entry> data(3);
  int offset = 5*index;
  for (int i = 0; i < 3; i++) {
    data[i] = SparseVector::Entry{(offset + i) % dim, sin(324.324*offset + 43249.324*i + 234.324)};
  }
  std::sort(data.begin(), data.end());
  return SparseVector(dim, data);
}

Array<SparseVector> makeTestVectors() {
  return Spani(0, 9).map<SparseVector>([](int index) {
    return makeTestVector(index);
  });
}

Eigen::MatrixXd toDense(Array<SparseVector> vectors) {
  int cols = vectors.size();
  int rows = vectors.first().dim();
  Eigen::MatrixXd dst = Eigen::MatrixXd::Zero(rows, cols);
  for (int j = 0; j < vectors.size(); j++) {
    for (auto e: vectors[j].entries()) {
      dst(e.index, j) = e.value;
    }
  }
  return dst;
}

TEST(LinearOptCalibTest, SparseGramSchmidtTest) {
  /*auto testVectors = makeTestVectors();
  EXPECT_FALSE(isOrthonormal(toDense(testVectors)));
  auto orthoVectors = gramSchmidt(testVectors);
  EXPECT_TRUE(isOrthonormal(toDense(orthoVectors)));
  std::cout << EXPR_AND_VAL_AS_STRING(toDense(testVectors)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(toDense(orthoVectors)) << std::endl;*/
}


TEST(LinearOptCalib, MatrixTest) {
  auto edata = toEData(getPsarosTestData().sliceTo(100));
  EXPECT_EQ(edata.A.rows(), edata.B.size());
  EXPECT_EQ(edata.A.cols(), 4);
  EXPECT_LT(0, edata.A.rows());
  EXPECT_TRUE(isEven(edata.A.rows()));
  auto spans = makeOverlappingSpans(edata.n, 100, 0.5);
  auto A = makeParameterizedApparentFlowMatrix(edata.A, spans);
  auto orthoA = orthonormalBasis(A);
  EXPECT_TRUE(isOrthonormal(orthoA));
  EXPECT_LE(subspaceDistance(A, orthoA), 1.0e-6);
}

TEST(LinearOptCalib, OverlappingSpanTest) {
  auto spans = makeOverlappingSpans(4, 2, 0.5);
  EXPECT_EQ(spans.size(), 3);
  EXPECT_EQ(spans[0], Spani(0, 2));
  EXPECT_EQ(spans[1], Spani(1, 3));
  EXPECT_EQ(spans[2], Spani(2, 4));
}


