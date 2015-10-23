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
#include <server/math/PowerMethod.h>

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
  static Array<Nav> data;
  if (data.empty()) {
    auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets")
      .pushDirectory("psaros33_Banque_Sturdza")
      .pushDirectory("2014")
      .pushDirectory("20140821").get();
    data = scanNmeaFolder(p, Nav::debuggingBoatId());
    return data;
  }
  return data;
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
  {
    Eigen::MatrixXd a(2, 1), b(2, 1);
    a(0, 0) = 1.0;
    a(1, 0) = 0.0;
    b(0, 0) = 0.0;
    b(1, 0) = 1.0;
    EXPECT_NEAR(subspaceDistance(a, b), 1.0, 1.0e-6);
  }
}

TEST(LinearOptCalib, AddFlowColumnsTest) {
  DataFit::CoordIndexer::Factory rows;
  DataFit::CoordIndexer::Factory cols;
  Spani span(0, 3);
  auto rowIndexer = rows.make(span.width(), 2);

  auto colIndexer = cols.make(1, 2);
  std::vector<DataFit::Triplet> triplets;
  Eigen::VectorXd B(rows.count());
  for (int i = 0; i < rows.count(); i++) {
    B[i] = sin(exp(0.34*i));
  }
  addFlowColumns(rowIndexer, colIndexer.span(0),
    &triplets, &B, rowIndexer);

  auto bCol = cols.make(1, 1);
  for (auto i: rows) {
    triplets.push_back(DataFit::Triplet(i, bCol[0], B[i]));
  }

  Eigen::SparseMatrix<double> mat(rows.count(), cols.count());
  mat.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::MatrixXd D = mat.toDense();
  EXPECT_EQ(rows.count(), 6);
  EXPECT_EQ(cols.count(), 3);
  for (auto i: rows) {
    for (auto j: cols) {
      if (j < 2) {
        if (i % 2 != j % 2) {
          EXPECT_EQ(D(i, j), 0.0);
        } else {
          EXPECT_LE(0.001, std::abs(D(i, j)));
        }
      } else {
        EXPECT_LE(0.001, std::abs(D(i, j)));
      }
    }
  }
  Eigen::MatrixXd DtD = D.transpose()*D;
  EXPECT_EQ(DtD.rows(), cols.count());
  EXPECT_EQ(DtD.cols(), cols.count());
  for (auto i: cols/*yes cols, not rows*/) {
    for (auto j: cols) {
      EXPECT_EQ(i == j, 1.0e-6 < std::abs(DtD(i, j)));
    }
  }
  EXPECT_NEAR(DtD(0, 0), 1.0, 1.0e-6);
  EXPECT_NEAR(DtD(1, 1), 1.0, 1.0e-6);
}

TEST(LinearOptCalib, CopyAndPasteVectorThenInsert) {
  Eigen::VectorXd src(3);
  src[0] = 1;
  src[1] = 2;
  src[2] = 3;

  auto srcSpans = Array<Spani>{Spani(0, 2), Spani(1, 3)};

  DataFit::CoordIndexer::Factory elements;

  auto dstIndexers = srcSpans.map2([&](Spani s) {
    return elements.make(s.width(), 1);
  });

  EXPECT_EQ(elements.count(), 4);


  Arrayi indexMap = assembleIndexMap(elements.count(), dstIndexers, srcSpans);
  auto dst = assembleVector(indexMap, src);

  EXPECT_EQ(dst.size(), 4);
  EXPECT_EQ(dst(0), 1);
  EXPECT_EQ(dst(1), 2);
  EXPECT_EQ(dst(2), 2);
  EXPECT_EQ(dst(3), 3);

  std::vector<DataFit::Triplet> triplets;
  insertDenseVectorIntoSparseMatrix(3.0, dst, Spani(9, 13), 3, &triplets);
  EXPECT_EQ(triplets.size(), 4);
  for (int i = 0; i < triplets.size(); i++) {
    auto t = triplets[i];
    EXPECT_EQ(t.col(), 3);
    EXPECT_EQ(t.row(), 9 + i);
    EXPECT_NEAR(t.value(), 3.0*dst(i), 1.0e-6);
  }
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

bool equallySized(Array<Spani> A, Array<Spani> B) {
  if (A.size() == B.size()) {
    int n = A.size();
    for (int i = 0; i < n; i++) {
      if (A[i].width() != B[i].width()) {
        return false;
      }
    }
    return true;
  }
  return false;
}

TEST(LinearOptCalib, ProblemTest) {
  auto edata = toEData(getPsarosTestData().sliceTo(20));
  auto spans = makeOverlappingSpans(edata.n, 10, 0.5);
  auto problem = makeProblem(edata.A, edata.B, spans);
  auto denseFull = problem.Qab.toDense();
  EXPECT_EQ(denseFull.rows(), 60);
  EXPECT_EQ(denseFull.cols(), 4 + 7);
  EXPECT_EQ(Spani(0, 4), problem.qaColSpan);
  EXPECT_EQ(Spani(4, 11), problem.qbColSpan);

  Eigen::MatrixXd lhs = denseFull.block(0, 0, 60, 4);
  Eigen::MatrixXd rhs = denseFull.block(0, 4, 60, 7);
  EXPECT_TRUE(isOrthonormal(lhs));
  EXPECT_TRUE(isOrthonormal(rhs));
  EXPECT_FALSE(isOrthonormal(denseFull));
  EXPECT_EQ(problem.Rparam.rows(), problem.Rparam.cols());
  EXPECT_EQ(problem.Rparam.rows(), 4);

  Eigen::SparseMatrix<double> Q = problem.Qab.transpose()*problem.Qab;

  PowerMethod::Settings pms;
  auto results = PowerMethod::computeMin([&](const Eigen::VectorXd &X) {
    return Q*X;
  }, Eigen::VectorXd::Ones(problem.Qab.cols()), pms);

  auto parameters = problem.computeParametersFromSolutionVector(results.X);

  auto QabWithSlackDense = problem.QabWithSlack.toDense();
  EXPECT_TRUE(equallySized(problem.rowSpansToFit, problem.slackARowSpans));
  EXPECT_TRUE(equallySized(problem.rowSpansToFit, problem.slackBRowSpans));

  //auto params = results.
}

TEST(LinearOptCalib, OverlappingSpanTest) {
  auto spans = makeOverlappingSpans(4, 2, 0.5);
  EXPECT_EQ(spans.size(), 3);
  EXPECT_EQ(spans[0], Spani(0, 2));
  EXPECT_EQ(spans[1], Spani(1, 3));
  EXPECT_EQ(spans[2], Spani(2, 4));
}




