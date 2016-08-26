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

using namespace sail;

typedef BoatState<double> BS;
typedef ceres::Jet<double, 1> ADType;
typedef BoatState<ADType> BSad;

TEST(BoatStateTest, Orthonormality) {

  auto expectedMast = BS::starboardVector().cross(BS::headingVector());
  EXPECT_NEAR((expectedMast - BS::mastVector()).norm(), 0.0, 1.0e-6);

  Eigen::Vector3d v[3] = {BS::starboardVector(), BS::headingVector(), BS::mastVector()};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_NEAR(v[i].dot(v[j]), i == j? 1.0 : 0.0, 1.0e-6);
    }
  }
}

namespace {
  bool eq(
      const HorizontalMotion<double> &a,
      const HorizontalMotion<double> &b) {
    return std::abs(a[0].knots() - b[0].knots()) < 1.0e-6
        && std::abs(a[1].knots() - b[1].knots()) < 1.0e-6;
  }
}

TEST(BoatStateTest, VariousProperties) {
  GeographicPosition<double> pos(45.0_deg, 45.0_deg);

  HorizontalMotion<double> gps(0.0_kn, 0.0_kn);
  HorizontalMotion<double> wind(0.0_kn, 0.0_kn);
  HorizontalMotion<double> current(0.0_kn, 0.0_kn);
  auto angle = 0.0_deg;
  Eigen::Vector3d axis(0.0, 0.0, 1.0);

  BS bs0(pos, gps, wind, current, angle, axis);

  EXPECT_TRUE(eq(bs0.windOverGround(),
      HorizontalMotion<double>(0.0_kn, 0.0_kn)));
  EXPECT_TRUE(eq(bs0.currentOverGround(),
      HorizontalMotion<double>(0.0_kn, 0.0_kn)));
  EXPECT_TRUE(eq(bs0.boatOverGround(),
      HorizontalMotion<double>(0.0_kn, 0.0_kn)));

  EXPECT_TRUE(bs0.valid());
}

TEST(BoatStateTest, WithAD) {
  BSad state;


}

TEST(BoatStateTest, AxisAngleAD) {
  ADType angle(0.3);
  angle.v[0] = 1.0;
  Eigen::Matrix<ADType, 3, 1> axis(
      ADType(0.0), ADType(0.0), ADType(1.0));
  Eigen::AngleAxis<ADType> aa(angle, axis);
  Eigen::Matrix<ADType, 3, 3> mat = aa.toRotationMatrix();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_TRUE(isFinite(mat(i, j)));
    }
  }
}



