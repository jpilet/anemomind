/*
 * commonTest.cpp
 *
 *  Created on: Apr 29, 2016
 *      Author: jonas
 */

#include <server/nautical/common.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(NauticalCommon, Current) {
  double boatMotionOverGround = 3.0;
  double current = 2.0;
  double boatMotionOverWater = 1.0;
  EXPECT_NEAR(current,
      computeCurrentFromBoatMotionOverWaterAndGround(boatMotionOverWater,
          boatMotionOverGround), 1.0e-6);
}

TEST(NauticalCommon, Wind) {
  double windOverGround = 4.0;
  double boatMotionOverGround = 3.4;
  double apparentWind = 0.6;

  EXPECT_NEAR(windOverGround, computeWindOverGroundFromApparentWindAndBoatMotion(
      apparentWind, boatMotionOverGround), 1.0e-6);
}

TEST(NauticalCommon, Twdir) {
  auto windOverGround = HorizontalMotion<double>::polar(
      Velocity<double>::knots(3.4), Angle<double>::degrees(119.4));

  EXPECT_NEAR(computeTwdirFromWindOverGround(windOverGround)
      .normalizedAt0().degrees(), -60.6, 1.0e-6);
}

TEST(NauticalCommon, Twa) {
  auto heading = Angle<double>::degrees(315.0);
  auto twdir = Angle<double>::degrees(20.0);
  auto twa = Angle<double>::degrees(65.0);

  EXPECT_NEAR(twa.degrees(),
      computeTwaFromTwdirAndHeading(twdir, heading)
      .normalizedAt0().degrees(), 1.0e-6);
}
