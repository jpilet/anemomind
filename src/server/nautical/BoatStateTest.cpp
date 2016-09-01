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

using namespace sail;

typedef BoatState<double> BS;
typedef ceres::Jet<double, 4> ADType;
typedef BoatState<ADType> BSad;

/*TEST(BoatStateTest, Orthonormality) {

  auto expectedMast = BS::starboardVector().cross(BS::onlyHeadingVector());
  EXPECT_NEAR((expectedMast - BS::mastVector()).norm(), 0.0, 1.0e-6);

  Eigen::Vector3d v[3] = {BS::starboardVector(), BS::onlyHeadingVector(), BS::mastVector()};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_NEAR(v[i].dot(v[j]), i == j? 1.0 : 0.0, 1.0e-6);
    }
  }
}*/

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


TEST(BoatStateTest, OrientationOrthonormality) {
  std::default_random_engine rng;
  std::uniform_real_distribution<double>
    distrib(0.0, 2.0*M_PI);
  auto randomAngle = [&]() {
    auto value = Angle<double>::radians(distrib(rng));
    return value;
  };

  for (int i = 0; i < 30; i++) {
    AbsoluteOrientation o{
      randomAngle(), randomAngle(), randomAngle()
    };

    Eigen::Matrix<double, 3, 3> R = orientationToMatrix(o);
    Eigen::Matrix3d RtR = R.transpose()*R;

    EXPECT_TRUE(
        (isZero<double, 3, 3>(
            RtR - Eigen::Matrix3d::Identity(),
            1.0e-5)));
  }
}

TEST(BoatStateTest, ElementaryOrientations) {
  {
    double k = 1.0/sqrt(2.0);
    AbsoluteOrientation o{45.0_deg, 0.0_deg, 0.0_deg};
    auto R = orientationToMatrix(o);
    EXPECT_LT(
        (R*Eigen::Vector3d(1, 0, 0)
           - Eigen::Vector3d(k, -k, 0.0)).norm(), 1.0e-6);
    EXPECT_LT(
        (R*Eigen::Vector3d(0, 1, 0)
           - Eigen::Vector3d(k, k, 0.0)).norm(), 1.0e-6);
  }
}

TEST(BoatStateTest, VariousProperties) {
  GeographicPosition<double> pos(45.0_deg, 45.0_deg);

  auto gps = hm(0.0_kn, 0.0_kn);
  auto wind = hm(0.0_kn, 0.0_kn);
  auto current = hm(0.0_kn, 0.0_kn);
  auto angle = 0.0_deg;

  BS bs0(pos, gps, wind,
      current, AbsoluteOrientation::onlyHeading(angle));

  EXPECT_TRUE(eq(bs0.windOverGround(), hm(0.0_kn, 0.0_kn)));
  EXPECT_TRUE(eq(bs0.currentOverGround(), hm(0.0_kn, 0.0_kn)));
  EXPECT_TRUE(eq(bs0.boatOverGround(), hm(0.0_kn, 0.0_kn)));

  EXPECT_TRUE(bs0.valid());

  /*{
    Eigen::Vector3d wh = bs0.worldHeadingVector();
    EXPECT_NEAR(wh(0), 0.0, 1.0e-6);
    EXPECT_NEAR(wh(1), 1.0, 1.0e-6);
    EXPECT_NEAR(wh(2), 0.0, 1.0e-6);
  }*/

  auto angle2 = 45.0_deg;
  {
    BS bs2(pos, gps, wind, current,
        AbsoluteOrientation::onlyHeading(angle2));
    /*Eigen::Vector3d wh = bs2.worldHeadingVector();
    EXPECT_NEAR(wh(0), -1.0/sqrt(2.0), 1.0e-6);
    EXPECT_NEAR(wh(1), 1.0/sqrt(2.0), 1.0e-6);
    EXPECT_NEAR(wh(2), 0.0, 1.0e-6);*/
    EXPECT_NEAR(bs2.heading().degrees(), 45.0, 1.0e-6);
  }
}

TEST(BoatStateTest, WithAD) {
  BSad state;


}

Eigen::MatrixXd computeRotWithAD(Eigen::Vector4d params) {
  typedef ceres::Jet<double, 4> AD4;
  AD4 angle(params(0));
  angle.v[0] = 1.0;
  Eigen::Matrix<AD4, 3, 1> axis(
      AD4(params(1)),
      AD4(params(2)),
      AD4(params(3)));
  for (int i = 0; i < 3; i++) {
    axis(i).v[1 + i] = 1.0;
  }

  Eigen::AngleAxis<AD4> aa(angle, axis);
  Eigen::Matrix<AD4, 3, 3> mat = aa.toRotationMatrix();

  Eigen::MatrixXd dst(9, 5);
  int row = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      AD4 x = mat(i, j);
      EXPECT_TRUE(isFinite(x.a));
      dst(row, 0) = x.a;
      for (int k = 0; k < 4; k++) {
        auto y = x.v[k];
        EXPECT_TRUE(isFinite(y));
        dst(row, 1+k) = y;
      }
      row++;
    }
  }
  EXPECT_EQ(row, 9);
  return dst;
}

Eigen::VectorXd computeFlatRot(Eigen::Vector4d params) {
  double angle = params(0);
  Eigen::Vector3d axis(params(1), params(2), params(3));
  Eigen::AngleAxis<double> aa(angle, axis);
  Eigen::VectorXd dst(9);
  Eigen::Matrix3d rot = aa.toRotationMatrix();
  int row = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      dst(row) = rot(i, j);
      row++;
    }
  }
  EXPECT_EQ(row, 9);
  return dst;
}

Eigen::MatrixXd computeRotWithNumDerives(Eigen::Vector4d params) {
  Eigen::MatrixXd dst(9, 5);
  double h = 1.0e-5;
  double f = 1.0/(2.0*h);
  dst.block(0, 0, 9, 1) = computeFlatRot(params);
  for (int i = 0; i < 4; i++) {
    Eigen::Vector4d plusParams = params;
    plusParams(i) += h;
    Eigen::Vector4d minusParams = params;
    minusParams(i) -= h;
    dst.block(0, 1+i, 9, 1) =
        f*(computeFlatRot(plusParams) - computeFlatRot(minusParams));
  }
  return dst;
}


void testDerivatives(Eigen::Vector4d params) {
  Eigen::MatrixXd mat0 = computeRotWithAD(params);
  Eigen::MatrixXd mat1 = computeRotWithNumDerives(params);
  EXPECT_EQ(mat0.rows(), 9);
  EXPECT_EQ(mat1.rows(), 9);
  EXPECT_EQ(mat0.cols(), 5);
  EXPECT_EQ(mat1.cols(), 5);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 5; j++) {
      EXPECT_NEAR(
          mat0(i, j),
          mat1(i, j), 1.0e-3);
    }
  }
}

TEST(BoatStateTest, AxisAngleAD) {
  testDerivatives(Eigen::Vector4d(1, 0, 0, 1));
}

TEST(BoatStateTest, LeewayTestNoDrift) {
  GeographicPosition<double> pos(45.0_deg, 45.0_deg);
  HorizontalMotion<double> gps(1.0_kn, 1.0_kn);
  HorizontalMotion<double> wind(0.0_kn, -1.0_kn);
  HorizontalMotion<double> current(0.0_kn, 0.0_kn);
  auto angle = 45.0_deg;

  BS bs(pos, gps, wind, current,
      AbsoluteOrientation::onlyHeading(angle));

  EXPECT_TRUE(eq(hm(-1.0_kn, -2.0_kn), bs.apparentWind()));
  EXPECT_NEAR(bs.computeLeewayError(0.0_deg).knots(), 0.0, 1.0e-6);
  auto expectedAWA = (45.0_deg) - Angle<double>::radians(atan(2.0));
  EXPECT_NEAR(bs.AWA().radians(),
      expectedAWA.radians(), 1.0e-6);
  EXPECT_LT(bs.AWA().radians(), 0.0);
  EXPECT_NEAR(bs.computeAWAError(expectedAWA).knots(), 0.0, 1.0e-6);
}

