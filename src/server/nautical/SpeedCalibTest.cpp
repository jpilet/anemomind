/*
 *  Created on: 24 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <cmath>
#include <server/nautical/SpeedCalib.h>
#include <server/common/string.h>


using namespace sail;

TEST(SpeedCalibTest, TestNoError) {
  SpeedCalib<double> c(0.0, 0.0, 0.0, 0.0);
  EXPECT_NEAR(c.eval(0.1), 0.1, 1.0e-6);
  EXPECT_NEAR(c.eval(1.0), 1.0, 1.0e-6);
  EXPECT_NEAR(c.evalDeriv(0.1), 1.0, 1.0e-6);
  EXPECT_NEAR(c.evalDeriv(1.0), 1.0, 1.0e-6);
}

TEST(SpeedCalibTest, DerivTest) {
  SpeedCalib<double> c(1.4, 0.5, 3.0, 1.8);
  double h = 1.0e-6;
  for (int i = 1; i < 30; i++) {
    EXPECT_NEAR((0.5/h)*(c.eval(i + h) - c.eval(i - h)), c.evalDeriv(i), 1.0e-3);
  }
}


