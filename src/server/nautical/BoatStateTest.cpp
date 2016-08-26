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
typedef ceres::Jet<double, 4> ADType;
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
  std::cout << "INitliazed" << std::endl;

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
  std::cout << "COmputing it" << std::endl;
  Eigen::MatrixXd mat0 = computeRotWithAD(params);
  std::cout << "Mat0 = " << mat0 << std::endl;
  Eigen::MatrixXd mat1 = computeRotWithNumDerives(params);
  std::cout << "Mat1 = " << mat1 << std::endl;
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



