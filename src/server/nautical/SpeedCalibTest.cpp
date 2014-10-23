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
  //SpeedCalib<double> c(0.0, 0.0, 0.0, 0.0);
  SpeedCalib<double> c(SpeedCalib<double>::initKParam(),
      SpeedCalib<double>::initMParam(),
      SpeedCalib<double>::initCParam(),
      SpeedCalib<double>::initAlphaParam());
  EXPECT_NEAR(c.eval(Velocity<double>::knots(0.1)).knots(), 0.1, 1.0e-6);
  EXPECT_NEAR(c.eval(Velocity<double>::knots(1.0)).knots(), 1.0, 1.0e-6);
}
