/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cmath>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(PhysQuantTest, InitialValue) {
  EXPECT_TRUE(std::isnan(InitialValue<double>::get()));
  EXPECT_TRUE(std::isnan(InitialValue<float>::get()));
  EXPECT_EQ(0i, InitialValue<int>::get());

  struct MyLittleType {
    int value = 34;
  };
  EXPECT_EQ(34, InitialValue<MyLittleType>::get().value);
}

TEST(PhysQuantTest, ScaleByDimensionless) {
  Dimensionless<double> factor = Dimensionless<double>::dimensionless(2.0);
  Angle<double> angle = Angle<double>::degrees(34.5);
  Angle<double> product = factor*angle;
  EXPECT_NEAR(product.degrees(), 69.0, 1.0e-6);

  Angle<double> divided = product/factor;
  EXPECT_NEAR(divided.degrees(), 34.5, 1.0e-6);
}

TEST(PhysQuantTest, DimensionlessTest) {
  Dimensionless<double> x = Dimensionless<double>::dimensionless(34.4);
  double y = x;
  EXPECT_EQ(y, 34.4);
}

TEST(PhysQuantTest, MixingQuantities) {
  Velocity<double> vel = Velocity<double>::knots(2.4);
  EXPECT_NEAR(vel.metersPerSecond(), 1.2346666666666667, 1.0e-6);
  // 2.4 knots = 1.23466667 meters per second

  Duration<double> dur = Duration<double>::hours(3.4);
  EXPECT_NEAR(dur.seconds(), 12240, 1.0e-6);
  // 3.4 hours = 12240 seconds

  Length<double> dist = Length<double>::nauticalMiles(8.16);
  EXPECT_NEAR(dist.meters(), 15112.32, 1.0e-6);
  // 15112.32 meters

  Length<double> computedDist = vel*dur;
  EXPECT_NEAR(computedDist.meters(), 15112.3200408, 0.1);

  Velocity<double> computedVelocity = dist/dur;
  EXPECT_NEAR(computedVelocity.metersPerSecond(), 1.23466667, 0.001);

  Duration<double> computedDuration = dist/vel;
  EXPECT_NEAR(computedDuration.seconds(), 12240, 0.1);
}

TEST(PhysQuantTest, CircumferenceTest) {
  Length<double> circumference = Length<double>::meters(4.0e7);
  double minutes = 60.0*Angle<double>::radians(2.0*M_PI).degrees();
  double oneNauticalMileMeters = circumference.meters()/minutes;
  EXPECT_NEAR(oneNauticalMileMeters, Length<double>::nauticalMiles(1.0).meters(), 30.0);
  EXPECT_NEAR(circumference.nauticalMiles(), minutes, 40);
}

TEST(PhysQuantTest, DurationTest) {
  double n = 34.0;
  EXPECT_NEAR(Duration<double>::seconds(n).seconds(), n, 1.0e-9);
  EXPECT_NEAR(Duration<double>::seconds(60).minutes(), 1.0, 1.0e-9);
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
  Duration<double> time = Duration<double>::minutes(10);

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

TEST(PhysQuantTest, OperatorTest6) {
  Mass<double> a = Mass<double>::lispund(1.0);
  Mass<double> b = a;
  EXPECT_NEAR((a - b).kilograms(), 0.0, 1.0e-6);
}

TEST(PhysQuantTest, DurationStr) {
  Duration<> d(
      Duration<>::weeks(3)
      + Duration<>::days(2)
      + Duration<>::hours(7)
      + Duration<>::minutes(3)
      + Duration<>::seconds(5));
  EXPECT_EQ("3 weeks, 2 days, 7 hours, 3 minutes, 5 seconds", d.str());
}

TEST(PhysQuantTest, CastTest) {
    Mass<float> flt = Mass<float>::kilograms(3.0f);
    Mass<double> dbl = flt.cast<double>();
    EXPECT_EQ(Mass<double>::kilograms(3.0), dbl);
}

TEST(PhysQuantTest, HorizontalMotionTest) {
    HorizontalMotion<double> a(
        Velocity<double>::metersPerSecond(3),
        Velocity<double>::metersPerSecond(2));
    auto b(a);
    HorizontalMotion<double> twice(
        Velocity<double>::metersPerSecond(6),
        Velocity<double>::metersPerSecond(4));
    EXPECT_EQ(a, b);
    EXPECT_EQ(twice, HorizontalMotion<double>(a + b));
    EXPECT_EQ(a, HorizontalMotion<double>(
            a + HorizontalMotion<double>::zero()));
}

TEST(PhysQuantTest, HorizontalMotionPolarTest) {
    HorizontalMotion<double>toWestA(
        Velocity<double>::metersPerSecond(-1),
        Velocity<double>::metersPerSecond(0));
    auto toWestB = HorizontalMotion<double>::polar(
            Velocity<double>::metersPerSecond(1),
            Angle<double>::degrees(270));
    EXPECT_NEAR(toWestA[0].metersPerSecond(), toWestB[0].metersPerSecond(), 1e-8);
    EXPECT_NEAR(toWestA[1].metersPerSecond(), toWestB[1].metersPerSecond(), 1e-8);

    HorizontalMotion<double>toSouthA(
        Velocity<double>::metersPerSecond(0),
        Velocity<double>::metersPerSecond(-1));
    auto toSouthB = HorizontalMotion<double>::polar(
            Velocity<double>::metersPerSecond(1),
            Angle<double>::degrees(180));
    EXPECT_NEAR(toSouthA[0].metersPerSecond(), toSouthB[0].metersPerSecond(), 1e-8);
    EXPECT_NEAR(toSouthA[1].metersPerSecond(), toSouthB[1].metersPerSecond(), 1e-8);
}

TEST(PhysQuantTest, HorizontalMotionAngleNormTest) {
    for (double angle = -180; angle<=180; angle += 15) {
        auto motion = HorizontalMotion<double>::polar(
                Velocity<double>::knots(10),
                Angle<double>::degrees(angle));
        EXPECT_NEAR(angle, motion.angle().degrees(), 1e-5);
        EXPECT_NEAR(10, motion.norm().knots(), 1e-5);
    }
}

TEST(PhysQuantTest, isNaN) {
  EXPECT_FALSE(isNaN(0));
  EXPECT_FALSE(isNaN(0.0));
  EXPECT_TRUE(isNaN(std::numeric_limits<double>::signaling_NaN()));
  EXPECT_TRUE(isNaN(std::numeric_limits<float>::signaling_NaN()));
  EXPECT_TRUE(isNaN(std::numeric_limits<float>::quiet_NaN()));
  float inf = 1.0f / 0.0f;
  EXPECT_EQ(isnan(inf), sail::isNaN(inf));
}

TEST(PhysQuantTest, Literals) {
  EXPECT_NEAR((134.0_deg).degrees(), 134, 1.0e-6);
}

TEST(PhysQuantTest, MapObjectValues) {
  auto mapper = [](double hiddenValue) {
        return static_cast<float>(hiddenValue);
      };
  {
    Angle<double> a = Angle<double>::radians(0.25*M_PI);
    Angle<float> b = a.mapObjectValues(mapper);
    EXPECT_NEAR(a.radians(), b.radians(), 1.0e-5);
  }{
    HorizontalMotion<double> motion{
      3.4_kn, 5.6_kn
    };
    HorizontalMotion<float> fmotion = motion.mapObjectValues(mapper);
    EXPECT_NEAR(fmotion[0].knots(), motion[0].knots(), 1.0e-5);
    EXPECT_NEAR(fmotion[1].knots(), motion[1].knots(), 1.0e-5);
  }
}


