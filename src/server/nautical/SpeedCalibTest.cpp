/*
 *  Created on: 24 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <cmath>
#include <device/Arduino/libraries/Corrector/SpeedCalib.h>
#include <server/common/string.h>


using namespace sail;

TEST(SpeedCalibTest, TestNoError) {
  SpeedCalib<double> c(0.0, 0.0, 0.0, 0.0);
  EXPECT_NEAR(c.eval(Velocity<double>::knots(0.1)).knots(), 0.1, 1.0e-6);
  EXPECT_NEAR(c.eval(Velocity<double>::knots(1.0)).knots(), 1.0, 1.0e-6);
}
