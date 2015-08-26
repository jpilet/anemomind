/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/calibration/LinearCalibration.h>
#include <server/nautical/Nav.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <armadillo>

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
  {
    arma::mat A(2, 4);
    arma::mat B(2, 1);
    LinearCalibration::makeTrueWindMatrixExpression(nav, true, &A, &B);
    arma::mat wind = A*X + B;
    EXPECT_NEAR(wind(0, 0), cnav.trueWindOverGround()[0].knots(), 1.0e-6);
    EXPECT_NEAR(wind(1, 0), cnav.trueWindOverGround()[1].knots(), 1.0e-6);
  }
  arma::mat A(2, 4);
  arma::mat B(2, 1);
  LinearCalibration::makeTrueCurrentMatrixExpression(nav, true, &A, &B);
  arma::mat current = A*X + B;
  EXPECT_NEAR(current(0, 0), cnav.trueCurrentOverGround()[0].knots(), 1.0e-6);
  EXPECT_NEAR(current(1, 0), cnav.trueCurrentOverGround()[1].knots(), 1.0e-6);

  static_assert(LinearCalibration::calcXOffset(true) == 4, "Wrong offset");
  static_assert(LinearCalibration::calcXOffset(false) == 2, "Wrong offset");
  static_assert(LinearCalibration::calcYOffset(false, 1) == 3, "Wrong offset");
  static_assert(LinearCalibration::calcYOffset(false, 2) == 4, "Wrong offset");
  static_assert(LinearCalibration::calcYOffset(true, 2) == 6, "Wrong offset");
  static_assert(LinearCalibration::calcQuadFormParamCount(false, 2) == 6, "Wrong offset");
  static_assert(LinearCalibration::calcQuadFormParamCount(true, 2) == 8, "Wrong offset");

  auto Q = LinearCalibration::makeQuadForm<true, 1, arma::mat>(39.9, A, B);

}
