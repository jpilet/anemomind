/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/CalibrationModel.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(CalibModelTest, CountTest) {
  DefaultCorrectorSet<double> set;
  EXPECT_EQ(set.magneticHeadingCorrector().paramCount(), 1);
  EXPECT_EQ(set.awaCorrector().paramCount(), 1);
  EXPECT_EQ(set.awsCorrector().paramCount(), 4);
  EXPECT_EQ(set.waterSpeedCorrector().paramCount(), 4);
}


TEST(CalibModelTest, InitTest) {

}


