/*
 * MotionsFromPairsTest.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <iostream>
#include <server/nautical/filters/MotionsFromPairs.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {
  auto offset = sail::TimeStamp::UTC(2015, 5, 16, 11, 0, 0);

  auto seconds = Duration<double>::seconds(1.0);
  auto degrees = Angle<double>::degrees(1.0);
  auto knots = Velocity<double>::knots(1.0);

  typedef TimedValue<Angle<double> > TimedAngle;
  typedef TimedValue<Velocity<double> > TimedVelocity;

  TimedAngle ta(double t, double a) {
    return TimedAngle(offset + t*seconds, a*degrees);
  }

  TimedVelocity tv(double t, double v) {
    return TimedVelocity(offset + t*seconds, v*knots);
  }

}

TEST(MotionsFromPairsTest, TestSmall) {
  Array<TimedAngle> angles{
    ta(0.1, 34.0),
    ta(0.5, 39.0)
  };

  Array<TimedVelocity> velocities{
    tv(0.3, 9.0)
  };

  auto motions = makeMotionsFromVelocityAnglePairs(
      velocities.begin(), velocities.end(),
      angles.begin(), angles.end(),
      Duration<double>::seconds(12.0));

  auto me0 = HorizontalMotion<double>::polar(9.0*knots, 34.0*degrees);
  auto me1 = HorizontalMotion<double>::polar(9.0*knots, 39.0*degrees);

  EXPECT_EQ(motions.size(), 2);
  auto m0 = motions[0];
  auto m1 = motions[1];
  EXPECT_NEAR((m0.time - offset).seconds(), 0.2, 1.0e-2);
  EXPECT_NEAR((m1.time - offset).seconds(), 0.4, 1.0e-2);
  EXPECT_NEAR(m0.value[0].knots(), me0[0].knots(), 1.0e-9);
  EXPECT_NEAR(m0.value[1].knots(), me0[1].knots(), 1.0e-9);
  EXPECT_NEAR(m1.value[0].knots(), me1[0].knots(), 1.0e-9);
  EXPECT_NEAR(m1.value[1].knots(), me1[1].knots(), 1.0e-9);
}
