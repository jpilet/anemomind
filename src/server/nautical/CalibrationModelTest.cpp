/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/CalibrationModel.h>
#include <gtest/gtest.h>
#include <server/common/Array.h>

using namespace sail;

TEST(CalibModelTest, CountTest) {
  DefaultCorrectorSet<double> set;
  EXPECT_EQ(set.magneticHeadingCorrector().paramCount(), 1);
  EXPECT_EQ(set.awaCorrector().paramCount(), 1);
  EXPECT_EQ(set.awsCorrector().paramCount(), 4);
  EXPECT_EQ(set.waterSpeedCorrector().paramCount(), 4);
  EXPECT_EQ(set.waterSpeedCorrector().paramCount(), 4);
  EXPECT_EQ(set.driftAngle().paramCount(), 2);
  EXPECT_EQ(set.paramCount(), 1 + 1 + 4 + 4 + 2);
}


TEST(CalibModelTest, InitTest) {
  DefaultCorrectorSet<double> set;
  Array<double> params = Array<double>::fill(set.paramCount(), 1.0e6);
  set.initialize(params.ptr());
  for (double p : params) {
    EXPECT_LT(std::abs(p), 1.0e2);
  }
}



