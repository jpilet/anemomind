/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PhysicalQuantity.h"

#include <gtest/gtest.h>

TEST(PhysQuantTest, CircumferenceTest) {
  Length<double> circumference = Length<double>::fromMetres(4.0e7);
  double minutes = 60.0*Angle<double>::fromRadians(2.0*M_PI).toDegrees();
  double oneNauticalMileMetres = circumference.toMetres()/minutes;
  EXPECT_NEAR(oneNauticalMileMetres, Length<double>::fromNauticalMiles(1.0).toMetres(), 30.0);
  EXPECT_NEAR(circumference.toNauticalMiles(), minutes, 40);
}

TEST(PhysQuantTest, TimeTest) {
  double n = 34.0;
  EXPECT_NEAR(Time<double>::fromSeconds(n).toSeconds(), n, 1.0e-9);
  EXPECT_NEAR(Time<double>::fromSeconds(60).toMinutes(), 1.0, 1.0e-9);
}

TEST(PhysQuantTest, HydroptereTest) {
  // « sustaining a speed of 52.86 knots (97.90 km/h; 60.83 mph) »
  Velocity<double> va = Velocity<double>::fromKnots(52.86);
  Velocity<double> vb = Velocity<double>::fromKilometresPerHour(97.90);
  Velocity<double> vc = Velocity<double>::fromMilesPerHour(60.83);
  EXPECT_NEAR(va.toMetresPerSecond(), vb.toMetresPerSecond(), 0.1);
  EXPECT_NEAR(va.toMetresPerSecond(), vc.toMetresPerSecond(), 0.1);
}

TEST(PhysQuantTest, WalkTest) {
  // When I am in a hurry, I walk
  Length<double> length = Length<double>::fromKilometres(1.0);
  // in
  Time<double> time = Time<double>::fromMinutes(10);

  // Then my speed is
  Velocity<double> velA = Velocity<double>::fromKilometresPerHour(length.toKilometres()/time.toHours());
  Velocity<double> velB = Velocity<double>::fromMetresPerSecond(length.toMetres()/time.toSeconds());

  EXPECT_NEAR(velA.toKnots(), velB.toKnots(), 1.0e-5);
}
