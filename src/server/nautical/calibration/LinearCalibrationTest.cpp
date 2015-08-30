/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/calibration/LinearCalibration.h>
#include <server/nautical/Nav.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <armadillo>

#include <Eigen/SparseCore>
#include <Eigen/SparseQR>

using namespace sail;



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
  LinearCalibration::initializeLinearParameters(true, X.memptr());
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

  /*
  static_assert(LinearCalibration::calcXOffset(true) == 4, "Wrong offset");
  static_assert(LinearCalibration::calcXOffset(false) == 2, "Wrong offset");
  static_assert(LinearCalibration::calcYOffset(false, 1) == 3, "Wrong offset");
  static_assert(LinearCalibration::calcYOffset(false, 2) == 4, "Wrong offset");
  static_assert(LinearCalibration::calcYOffset(true, 2) == 6, "Wrong offset");
  static_assert(LinearCalibration::calcQuadFormParamCount(false, 2) == 6,
      "Wrong param count");
  static_assert(LinearCalibration::calcQuadFormParamCount(true, 2) == 8,
      "Wrong param count");

  {
    auto Q = LinearCalibration::makeQuadForm<true, 1, arma::mat>(39.9, A, B);

    // The first 4 values are the calibration parameters.
    // The second last values are the corresponding current components for x and y.
    double x[6] = {1, 0, 0, 0, current(0, 0), current(1, 0)};

    static_assert(6 == LinearCalibration::calcQuadFormParamCount(true, 1), "Wrong count");
    EXPECT_NEAR(Q.eval(x), 0.0, 1.0e-6);
  }{
    auto Q = LinearCalibration::makeQuadForm<true, 2, arma::mat>(39.9, A, B);

    // The first 4 values are the calibration parameters.
    // The values 5 and 6 are the constant and linear coefficients of the
    //   polynomial to represent the x component of the current.
    // And values 7 and 8 are the coefficients for the Y polynomial.
    double x[8] = {1, 0, 0, 0, current(0, 0), 0, current(1, 0), 0};

    static_assert(8 == LinearCalibration::calcQuadFormParamCount(true, 2), "Wrong count");
    EXPECT_NEAR(Q.eval(x), 0.0, 1.0e-6);
  }*/
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



