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

using namespace sail;

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

using namespace LinearCalibration;


bool isOrthonormal(Eigen::MatrixXd Q) {
  Eigen::MatrixXd QtQ = Q.transpose()*Q;
  int n = QtQ.rows();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      auto expected = (i == j? 1.0 : 0.0);
      if (std::abs(QtQ(i, j) - expected) > 1.0e-9) {
        return false;
      }
    }
  }
  return true;
}

Eigen::MatrixXd projector(Eigen::MatrixXd A) {
  //return A*((A.transpose()*A).inverse()*A.transpose());
  Eigen::MatrixXd pInv = A.colPivHouseholderQr().solve(Eigen::MatrixXd::Identity(A.rows(), A.rows()));
  return A*pInv;
}

bool spanTheSameSubspace(Eigen::MatrixXd A, Eigen::MatrixXd B) {
  auto aProj = projector(A);
  auto bProj = projector(B);
  auto D = aProj - bProj;
  for (int i = 0; i < D.rows(); i++) {
    for (int j = 0; j < D.cols(); j++) {
      if (D(i, j) > 1.0e-9) {
        return false;
      }
    }
  }
  return true;
}

TEST(LinearCalibrationTest, SubtractMean) {
  {
    Eigen::MatrixXd itggt = Eigen::MatrixXd::Zero(6, 2);
    itggt(2, 0) = 1;
    itggt(2, 1) = 2;
    itggt(3, 0) = 3;
    itggt(3, 1) = 4;
    itggt(4, 0) = 6;
    itggt(4, 1) = 8;
    itggt(5, 0) = 10;
    itggt(5, 1) = 12;


    Eigen::MatrixXd test(4, 2);
    int counter = 1;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 2; j++) {
        test(i, j) = counter;
        counter++;
      }
    }

    auto itg = integrate(test, 2);
    EXPECT_EQ(itg.rows(), itggt.rows());
    EXPECT_EQ(itg.cols(), itggt.cols());
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j < 2; j++) {
        EXPECT_NEAR(itg(i, j), itggt(i, j), 1.0e-6);
      }
    }


    Eigen::MatrixXd test2 = subtractMean(test, 1);
    double expected[4] = {-3, -1, 1, 3};
    EXPECT_EQ(test2.rows(), 4);
    EXPECT_EQ(test2.cols(), 2);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 2; j++) {
        double a = test2(i, j);
        double b = expected[i];
        EXPECT_NEAR(a, b, 1.0e-6);
      }
    }
  }{
    Eigen::MatrixXd K(4, 1);
    K(0, 0) = 9;
    K(1, 0) = 30;
    K(2, 0) = 7;
    K(3, 0) = 40;
    auto B = subtractMean(K, 2);
    double expected[4] = {1, -5, -1, 5};
    for (int i = 0; i < 4; i++) {
      EXPECT_NEAR(B(i, 0), (expected[i]), 1.0e-6);
    }
  }
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

TEST(LinearCalibrationTest, RealData) {
  auto navs = getTestDataset();
  Duration<double> dif = navs.last().time() - navs.first().time();

  FlowSettings flowSettings;
  auto trueWind = makeTrueWindMatrices(navs, flowSettings);
  //auto trueCurrent = makeTrueCurrentMatrices(navs, flowSettings);

  auto flow = trueWind;

  Eigen::MatrixXd Aeigen =
      Eigen::Map<Eigen::MatrixXd>(flow.A.ptr(), flow.rows(), flow.A.cols());

  Eigen::MatrixXd Beigen =
      Eigen::Map<Eigen::MatrixXd>(flow.B.ptr(), flow.rows(), 1);

  {
    Eigen::MatrixXd Asub = Aeigen.block(0, 0, 30, flow.A.cols());
    Eigen::MatrixXd Q = orthonormalBasis<Eigen::MatrixXd>(Asub);
    EXPECT_TRUE(isOrthonormal(Q));
    EXPECT_TRUE(spanTheSameSubspace(Q, Asub));
  }
  auto splits = makeRandomSplit(flow.count(), 2);
  auto normed = assembleNormedData(Aeigen, Beigen, splits);

  Eigen::VectorXd X = Eigen::VectorXd::Zero(4, 1);
  plotTrajectories(normed, X);
}


