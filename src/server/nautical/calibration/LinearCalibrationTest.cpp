/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/calibration/LinearCalibration.h>
#include <server/nautical/Nav.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/logging.h>
#include <armadillo>
#include <server/common/string.h>
#include <Eigen/SparseCore>
#include <Eigen/SparseQR>
#include <server/common/ArrayIO.h>
#include <Eigen/Cholesky>
#include <server/math/Random.h>
#include <server/math/EigenUtils.h>


using namespace sail;
using namespace LinearCalibration;
using namespace EigenUtils;


namespace {
  auto rng = makeRngForTests();

  bool eq(const FlowFiber &a, const FlowFiber &b, double tol = 1.0e-6) {
    return EigenUtils::eq(a.Q, b.Q, tol) && EigenUtils::eq(a.B, b.B, tol);
  }
}



Array<Nav> getTestDataset() {
  auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("Irene")
    .pushDirectory("2013").get();
  auto allNavs = scanNmeaFolder(p, Nav::debuggingBoatId());
  auto from = TimeStamp::UTC(2013, 8, 2, 10, 8, 0);
  auto to = TimeStamp::UTC(2013, 8, 2, 13, 41, 44);
  auto navs = allNavs.slice([=](const Nav &x) {
    auto t = x.time();
    return from < t && t < to;
  });
  LOG(INFO) << "Selected " << navs.size() << " navs";
  return navs;
}

TEST(LinearCalibrationTest, TestWind) {
  Nav nav;
  nav.setAwa(Angle<double>::degrees(129));
  nav.setAws(Velocity<double>::knots(9));
  nav.setGpsBearing(Angle<double>::degrees(301));
  nav.setGpsSpeed(Velocity<double>::knots(7.0));
  nav.setWatSpeed(Velocity<double>::knots(6.3));
  nav.setMagHdg(Angle<double>::degrees(307));
  Corrector<double> corr;
  auto cnav = corr.correct(nav);
  arma::mat X(4, 1);
  LinearCalibration::initializeParameters(true, X.memptr());
  LinearCalibration::FlowSettings s;
  {
    arma::mat A(2, 4);
    arma::mat B(2, 1);
    LinearCalibration::makeTrueWindMatrixExpression(nav, s, &A, &B);
    arma::mat wind = A*X + B;
    EXPECT_NEAR(wind(0, 0), cnav.trueWindOverGround()[0].knots(), 1.0e-6);
    EXPECT_NEAR(wind(1, 0), cnav.trueWindOverGround()[1].knots(), 1.0e-6);
  }
  arma::mat A(2, 4);
  arma::mat B(2, 1);
  LinearCalibration::makeTrueCurrentMatrixExpression(nav, s, &A, &B);
  arma::mat current = A*X + B;
  EXPECT_NEAR(current(0, 0), cnav.trueCurrentOverGround()[0].knots(), 1.0e-6);
  EXPECT_NEAR(current(1, 0), cnav.trueCurrentOverGround()[1].knots(), 1.0e-6);
}

TEST(LinearCalibrationTest, Sparse) {
  typedef Eigen::Triplet<double> T;
  std::vector<T> triplets(3);
  triplets[0] = T(0, 0, 1.0);
  triplets[1] = T(1, 1, 2.0);
  triplets[2] = T(2, 2, 4.0);
  Eigen::SparseMatrix<double> A(3, 3);
  A.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::MatrixXd B(3, 1);
  B(0, 0) = 9;
  B(1, 0) = 8;
  B(2, 0) = 64;
  // http://eigen.tuxfamily.org/dox/group__OrderingMethods__Module.html
  Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int> > decomp(A);
  Eigen::MatrixXd X = decomp.solve(B);
  EXPECT_NEAR(X(0, 0), 9.0, 1.0e-9);
  EXPECT_NEAR(X(1, 0), 4.0, 1.0e-9);
  EXPECT_NEAR(X(2, 0), 16.0, 1.0e-9);
}

TEST(LinearCalibrationTest, ExtractRows) {
  Eigen::MatrixXd A(6, 1);
  for (int i = 0; i < 3; i++) {
    int offset = 2*i;
    for (int j = 0; j < 2; j++) {
      A(offset + j, 0) = i+1;
    }
  }
  auto e = extractRows(A, Arrayi{0, 2}, 2);
  EXPECT_EQ(e.rows(), 4);
  EXPECT_EQ(e.cols(), 1);
  double expected[4] = {1, 1, 3, 3};
  for (int i = 0; i < 4; i++) {
    double a = e(i, 0);
    double b = expected[i];
    EXPECT_NEAR(a, b, 1.0e-6);
  }
}

TEST(LinearCalibrationTest, Split) {
  auto split = makeRandomSplit(13, 3);
  EXPECT_EQ(split.size(), 3);
  for (auto s: split) {
    EXPECT_EQ(s.size(), 4);
  }

  Spani indexSpan(0, 12);
  auto marked = Arrayb::fill(12, false);
  for (auto s: split) {
    for (auto i: s) {
      EXPECT_TRUE(indexSpan.contains(i));
      EXPECT_FALSE(marked[i]);
      marked[i] = true;
    }
  }
}



void initialize(Arrayd *X) {
  X->setTo(0.0);
  (*X)[0] = 1.0;
}

TEST(LinearCalibrationTest, SubtractMean) {
  Eigen::MatrixXd A(6, 2);
  for (int i = 0; i < 6; i++) {
    A(i, 0) = i;
    A(i, 1) = i;
  }
  double expected[6] = {-2, -2, 0, 0, 2, 2};
  Eigen::MatrixXd B = subtractMean(A, 2);
  for (int i = 0; i < 6; i++) {
    EXPECT_NEAR(B(i, 0), expected[i], 1.0e-6);
    EXPECT_NEAR(B(i, 1), expected[i], 1.0e-6);
  }
}

TEST(LinearCalibrationTest, FlowFiberOps) {
  auto A = FlowFiber{makeRandomMatrix(10, 3, &rng), makeRandomMatrix(10, 1, &rng)};
  auto Aitg = A.integrate();
  EXPECT_FALSE(eq(A.Q, Aitg.Q));
  EXPECT_FALSE(eq(A.B, Aitg.B));
  auto A2 = Aitg.differentiate();
  EXPECT_TRUE(eq(A.Q, A2.Q));
  EXPECT_TRUE(eq(A.B, A2.B));
  EXPECT_FALSE(eq(A, A.dropConstant()));
  EXPECT_FALSE(eq(A, A.dropVariable()));
  EXPECT_TRUE(eq(A, A.dropConstant() + A.dropVariable()));
  EXPECT_TRUE(eq(A.dropConstant(), A - A.dropVariable()));
  EXPECT_TRUE(eq(2.0*A, A + A));
}

TEST(LinearCalibrationTest, Spans) {
  auto spans = makeContiguousSpans(9, 4);
  EXPECT_EQ(spans.size(), 2);
  EXPECT_EQ(spans, (Array<Spani>{Spani(0, 4), Spani(4, 8)}));
}

TEST(LinearCalibrationTest, ConstantFlowTrajectory) {
  std::vector<DataFit::Triplet> triplets;
  auto rows = DataFit::CoordIndexer::Factory();
  auto cols = DataFit::CoordIndexer::Factory();
  auto flowRows = rows.make(3, 2);
  auto flowCols = cols.make(2, 2);
  makeConstantFlowTrajectoryMatrix(flowRows, flowCols, &triplets);
  Eigen::SparseMatrix<double> A(rows.count(), cols.count());
  A.setFromTriplets(triplets.begin(), triplets.end());
  Eigen::MatrixXd A2 = Eigen::MatrixXd::Zero(6, 4);
  for (int i = 0; i < 3; i++) {
    int row = 2*i;
    A2(row + 0, 0) = i;
    A2(row + 1, 1) = i;
    A2(row + 2, 0) = 1.0;
    A2(row + 3, 1) = 1.0;
  }
  EXPECT_TRUE(eq(A.toDense(), A2));
}

void plotConstantFlows(Eigen::VectorXd B, int splitSize) {
  int n = getObservationCount(B);
  auto splits = makeContiguousSpans(n, splitSize);
  TrajectoryPlotter plot;
  plot.plot(B, 1, false);

  for (auto s1: splits) {
    Spani s = 2*s1;
    auto traj = sliceRows(B, s);
    auto fitted = fitConstantFlow(traj);
    plot.plot(fitted, 2, true);
  }

  plot.show();
}

TEST(LinearCalibrationTest, RealData) {
  auto navs = getTestDataset();
  Duration<double> dif = navs.last().time() - navs.first().time();

  FlowSettings flowSettings;
  auto trueWind = makeTrueWindMatrices(navs, flowSettings);
  auto trueCurrent = makeTrueCurrentMatrices(navs, flowSettings);

  auto flow = trueWind;

  Eigen::MatrixXd Avelocities =
      Eigen::Map<Eigen::MatrixXd>(flow.A.ptr(), flow.rows(), flow.A.cols());

  Eigen::MatrixXd Bvelocities =
      Eigen::Map<Eigen::MatrixXd>(flow.B.ptr(), flow.rows(), 1);

  Eigen::MatrixXd Atrajectory = integrateFlowData(Avelocities);
  Eigen::MatrixXd Btrajectory = integrateFlowData(Bvelocities);

  int n = getObservationCount(Atrajectory);
  int splitSize = 100;
  Array<Spani> splits = makeContiguousSpans(n, splitSize);
  plotConstantFlows(Btrajectory, splitSize);

  auto results = optimizeLocallyConstantFlows(
      Atrajectory, Btrajectory,
      splits, irls::Settings());
  std::cout << EXPR_AND_VAL_AS_STRING(results.inliers) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(results.parameters) << std::endl;
}
