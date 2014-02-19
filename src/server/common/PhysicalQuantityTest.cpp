/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cmath>
#include "PhysicalQuantity.h"
#include <gtest/gtest.h>

using namespace sail;

TEST(PhysQuantTest, CircumferenceTest) {
  Length<double> circumference = Length<double>::meters(4.0e7);
  double minutes = 60.0*Angle<double>::radians(2.0*M_PI).degrees();
  double oneNauticalMileMeters = circumference.meters()/minutes;
  EXPECT_NEAR(oneNauticalMileMeters, Length<double>::nauticalMiles(1.0).meters(), 30.0);
  EXPECT_NEAR(circumference.nauticalMiles(), minutes, 40);
}

TEST(PhysQuantTest, TimeTest) {
  double n = 34.0;
  EXPECT_NEAR(Time<double>::seconds(n).seconds(), n, 1.0e-9);
  EXPECT_NEAR(Time<double>::seconds(60).minutes(), 1.0, 1.0e-9);
}

TEST(PhysQuantTest, HydroptereTest) {
  // « sustaining a speed of 52.86 knots (97.90 km/h; 60.83 mph) »
  Velocity<double> va = Velocity<double>::knots(52.86);
  Velocity<double> vb = Velocity<double>::kilometersPerHour(97.90);
  Velocity<double> vc = Velocity<double>::milesPerHour(60.83);
  EXPECT_NEAR(va.metersPerSecond(), vb.metersPerSecond(), 0.1);
  EXPECT_NEAR(va.metersPerSecond(), vc.metersPerSecond(), 0.1);
}

TEST(PhysQuantTest, WalkTest) {
  // When I am in a hurry, I walk
  Length<double> length = Length<double>::kilometers(1.0);
  // in
  Time<double> time = Time<double>::minutes(10);

  // Then my speed is
  Velocity<double> velA = Velocity<double>::kilometersPerHour(length.kilometers()/time.hours());
  Velocity<double> velB = Velocity<double>::metersPerSecond(length.meters()/time.seconds());

  EXPECT_NEAR(velA.knots(), velB.knots(), 1.0e-5);
}

TEST(PhysQuantTest, AngleTest) {
  Angle<double> a = Angle<double>::degrees(30);
  Angle<double> b = Angle<double>::degrees(60);
  Angle<double> a2 = Angle<double>::radians(M_PI/6.0);
  Angle<double> b2 = Angle<double>::radians(M_PI/3.0);
  EXPECT_NEAR(a.degrees(), a2.degrees(), 1.0e-5);
  EXPECT_NEAR(b.degrees(), b2.degrees(), 1.0e-5);
  EXPECT_NEAR(sin(a), 1.0/2.0, 1.0e-6);
  EXPECT_NEAR(sin(b), sqrt(3)/2.0, 1.0e-6);
  EXPECT_NEAR(sin(a)*sin(a) + cos(a)*cos(a), 1.0, 1.0e-6);
  EXPECT_NEAR(cos(a2)*cos(b2) - sin(a2)*sin(b2), cos(a.radians() + b.radians()), 1.0e-5);
}

// Try the operators
TEST(PhysQuantTest, OperatorTest) {
  Mass<double> a = Mass<double>::kilograms(30.0);
  Mass<double> b = Mass<double>::kilograms(34.0);

  // The + operator is inherited
  Mass<double> c = a + b;

  EXPECT_NEAR(c.kilograms(), 64.0, 1.0e-6);
}

TEST(PhysQuantTest, OperatorTest2) {
  Mass<double> sum = Mass<double>::lispund(0.0);
  for (int i = 0; i < 20; i++) {
    sum = sum + Mass<double>::lispund(1.0);
  }
  EXPECT_NEAR(sum.skeppund(), 1.0, 1.0e-6);
}

TEST(PhysQuantTest, OperatorTest3) {
  Mass<double> a = Mass<double>::skeppund(1.0);
  Mass<double> b = Mass<double>::lispund(20.0);
  Mass<double> c = Mass<double>::kilograms(170);
  EXPECT_NEAR(a/b, 1.0, 1.0e-6);
  EXPECT_NEAR(a/c, 1.0, 1.0e-6);
}

TEST(PhysQuantTest, OperatorTest4) {
  Mass<double> a = Mass<double>::lispund(1.0);
  Mass<double> b = Factor<double>(20)*a;
  Mass<double> c = a*Factor<double>(20);
  EXPECT_NEAR(Mass<double>::skeppund(1.0).kilograms(), b.kilograms(), 1.0e-6);
  EXPECT_NEAR(c.kilograms(), 170.0, 1.0e-6);
}

TEST(PhysQuantTest, OperatorTest5) {
  Mass<double> a = Mass<double>::lispund(1.0);
  Mass<double> b = -a;
  EXPECT_NEAR((a + b).kilograms(), 0.0, 1.0e-6);
}

TEST(PhysQuantTest, OperatorTest6) {
  Mass<double> a = Mass<double>::lispund(1.0);
  Mass<double> b = a;
  EXPECT_NEAR((a - b).kilograms(), 0.0, 1.0e-6);
}
