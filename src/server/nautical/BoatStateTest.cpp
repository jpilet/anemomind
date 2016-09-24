/*
 * BoatStateTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/BoatState.h>
#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <ceres/jet.h>
#include <Eigen/Geometry>
#include <random>
#include <server/math/JetUtils.h>

using namespace sail;

typedef BoatState<double> BS;
typedef ceres::Jet<double, 4> ADType;

namespace {
  bool eq(
      const HorizontalMotion<double> &a,
      const HorizontalMotion<double> &b) {
    return std::abs(a[0].knots() - b[0].knots()) < 1.0e-6
        && std::abs(a[1].knots() - b[1].knots()) < 1.0e-6;
  }

  HorizontalMotion<double> hm(
      const Velocity<double> &x,
      const Velocity<double> &y) {
    return HorizontalMotion<double>(x, y);
  }
}

TEST(BoatStateTest, VariousProperties) {
  GeographicPosition<double> pos(45.0_deg, 45.0_deg);

  auto gps = hm(0.0_kn, 0.0_kn);
  auto wind = hm(0.0_kn, 0.0_kn);
  auto current = hm(0.0_kn, 0.0_kn);
  auto angle = 0.0_deg;

  BS bs0(pos, gps, wind,
      current, AbsoluteBoatOrientation<double>::onlyHeading(angle));

  EXPECT_EQ(bs0, bs0);

  EXPECT_TRUE(eq(bs0.windOverGround(), hm(0.0_kn, 0.0_kn)));
  EXPECT_TRUE(eq(bs0.currentOverGround(), hm(0.0_kn, 0.0_kn)));
  EXPECT_TRUE(eq(bs0.boatOverGround(), hm(0.0_kn, 0.0_kn)));

  EXPECT_TRUE(bs0.valid());

  auto angle2 = 45.0_deg;
  {
    BS bs2(pos, gps, wind, current,
        AbsoluteBoatOrientation<double>::onlyHeading(angle2));
    /*Eigen::Vector3d wh = bs2.worldHeadingVector();
    EXPECT_NEAR(wh(0), -1.0/sqrt(2.0), 1.0e-6);
    EXPECT_NEAR(wh(1), 1.0/sqrt(2.0), 1.0e-6);
    EXPECT_NEAR(wh(2), 0.0, 1.0e-6);*/
    EXPECT_NEAR(bs2.heading().degrees(), 45.0, 1.0e-6);
  }
}

TEST(BoatStateTest, LeewayTestNoDrift) {
  GeographicPosition<double> pos(45.0_deg, 45.0_deg);
  HorizontalMotion<double> gps(1.0_kn, 1.0_kn);
  HorizontalMotion<double> wind(0.0_kn, -1.0_kn);
  HorizontalMotion<double> current(0.0_kn, 0.0_kn);
  auto angle = 45.0_deg;

  BS bs(pos, gps, wind, current,
      AbsoluteBoatOrientation<double>::onlyHeading(angle));


  EXPECT_TRUE(eq(hm(-1.0_kn, -2.0_kn), bs.apparentWind()));
  EXPECT_NEAR(bs.computeLeewayError(0.0_deg).knots(), 0.0, 1.0e-6);
  auto expectedAWA = (45.0_deg) - Angle<double>::radians(atan(2.0));
  EXPECT_NEAR(bs.AWA().radians(),
      expectedAWA.radians(), 1.0e-6);
  EXPECT_LT(bs.AWA().radians(), 0.0);
  EXPECT_NEAR(bs.computeAWAError(expectedAWA).knots(), 0.0, 1.0e-6);
}

TEST(BoatStateTest, InterpolationTest) {
  BS a, b;
  {
    GeographicPosition<double> pos(45.0_deg, 45.0_deg);
    HorizontalMotion<double> gps(1.0_kn, 1.0_kn);
    HorizontalMotion<double> wind(0.0_kn, -1.0_kn);
    HorizontalMotion<double> current(0.0_kn, 0.0_kn);
    auto angle = 45.0_deg;
    a = BS(pos, gps, wind, current,
        AbsoluteBoatOrientation<double>::onlyHeading(angle));
  }{
    GeographicPosition<double> pos(-45.0_deg, 135.0_deg);
    HorizontalMotion<double> gps(4.0_kn, 5.0_kn);
    HorizontalMotion<double> wind(-12.0_kn, -9.0_kn);
    HorizontalMotion<double> current(3.0_kn, 4.0_kn);
    auto angle = 70.0_deg;
    b = BS(pos, gps, wind, current,
        AbsoluteBoatOrientation<double>::onlyHeading(angle));
  }
  EXPECT_EQ(a, interpolate(0.0, a, b));
  EXPECT_EQ(b, interpolate(1.0, a, b));
}

TEST(BoatStateTest, MapToJetAndBack) {
  typedef ceres::Jet<ceres::Jet<double, 2>, 3> ADType;

  GeographicPosition<double> pos(45.0_deg, 45.0_deg);
  HorizontalMotion<double> gps(1.0_kn, 1.0_kn);
  HorizontalMotion<double> wind(0.0_kn, -1.0_kn);
  HorizontalMotion<double> current(0.0_kn, 0.0_kn);
  auto angle = 45.0_deg;
  BS a(pos, gps, wind, current,
      AbsoluteBoatOrientation<double>::onlyHeading(angle));

  BoatState<ADType> b = a.mapObjectValues([](double x) {
    return MakeConstant<ADType>::apply(x);
  });

  BS c = b.mapObjectValues([](ADType x) {
    return x.a.a;
  });

  EXPECT_EQ(a, c);
}

