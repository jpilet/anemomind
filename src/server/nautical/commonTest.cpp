/*
 * commonTest.cpp
 *
 *  Created on: Apr 29, 2016
 *      Author: jonas
 */

#include <server/nautical/common.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {
  auto kt = Velocity<double>::knots(1.0);
}

TEST(NauticalCommon, Current) {
  auto boatMotionOverGround = HorizontalMotion<double>(3.0*kt, 0.0*kt);
  auto current = HorizontalMotion<double>(2.0*kt, 0.0*kt);
  auto boatMotionOverWater = HorizontalMotion<double>(1.0*kt, 0.0*kt);
  EXPECT_NEAR(current[0].knots(),
      computeCurrentFromBoatMotionOverWaterAndGround(boatMotionOverWater,
          boatMotionOverGround)[0].knots(), 1.0e-6);
}

TEST(NauticalCommon, Wind) {
  auto windOverGround = HorizontalMotion<double>(4.0*kt, 0.0*kt);
  auto boatMotionOverGround = HorizontalMotion<double>(3.4*kt, 0.0*kt);
  auto apparentWind = HorizontalMotion<double>(0.6*kt, 0.0*kt);

  EXPECT_NEAR(windOverGround[0].knots(),
      computeWindOverGroundFromApparentWindAndBoatMotion(
          apparentWind, boatMotionOverGround)[0].knots(), 1.0e-6);
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
