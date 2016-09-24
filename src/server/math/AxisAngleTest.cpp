/*
 * AxisAngleTest.cpp
 *
 *  Created on: 24 Sep 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/AxisAngle.h>
#include <server/common/Array.h>

using namespace sail;

TEST(AxisAngleTest, BasicRotations) {
  double k = 1.0/sqrt(2.0);
  {
    Eigen::Vector3d omega(0.0, 0.0, 0.0);
    auto R = computeRotationFromOmega(omega);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        EXPECT_NEAR(R(i, j), i == j? 1.0 : 0.0, 1.0e-6);
      }
    }
  }{
    Eigen::Vector3d omega(0.0, 0.0, 0.25*M_PI);
    auto R = computeRotationFromOmega(omega);

    // Image of X (1 0 0)
    EXPECT_NEAR(R(0, 0), k, 1.0e-6);
    EXPECT_NEAR(R(1, 0), k, 1.0e-6);
    EXPECT_NEAR(R(2, 0), 0.0, 1.0e-6);

    // Image of Y (0 1 0)
    EXPECT_NEAR(R(0, 1), -k, 1.0e-6);
    EXPECT_NEAR(R(1, 1), k, 1.0e-6);
    EXPECT_NEAR(R(2, 1), 0.0, 1.0e-6);

    // Image of Z (0 0 1)
    EXPECT_NEAR(R(0, 2), 0.0, 1.0e-6);
    EXPECT_NEAR(R(1, 2), 0.0, 1.0e-6);
    EXPECT_NEAR(R(2, 2), 1.0, 1.0e-6);
  }
}

Eigen::Matrix3d differentiate(Eigen::Vector3d omega, int wrt) {
  double h = 1.0e-5;
  Eigen::Vector3d omegaPlus = omega;
  omegaPlus(wrt) += h;
  auto Rplus = computeRotationFromOmega(omegaPlus);
  Eigen::Vector3d omegaMinus = omega;
  omegaMinus(wrt) -= h;
  auto Rminus = computeRotationFromOmega(omegaMinus);
  return (1.0/(2.0*h))*(Rplus - Rminus);
}

Array<Eigen::Matrix3d> differentiateNumerically(Eigen::Vector3d omega) {
  Array<Eigen::Matrix3d> dst(3);
  for (int i = 0; i < 3; i++) {
    dst[i] = differentiate(omega, i);
  }
  return dst;
}

Array<Eigen::Matrix3d> differentiateAD(Eigen::Vector3d omega) {
  typedef ceres::Jet<double, 3> AD;
  Eigen::Matrix<AD, 3, 1> omegaAD;
  for (int i = 0; i < 3; i++) {
    AD x = AD(omega(i));
    x.v[i] = 1.0;
    omegaAD(i) = x;
  }
  Eigen::Matrix<AD, 3, 3> rotation =
      computeRotationFromOmega(omegaAD);

  Array<Eigen::Matrix3d> dst(3);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        dst[k](i, j) = rotation(i, j).v[k];
      }
    }
  }
  return dst;
}

void compareMats(const Eigen::Matrix3d &A,
                 const Eigen::Matrix3d &B) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_NEAR(A(i, j), B(i, j), 1.0e-5);
    }
  }
}

void compareDifs(const Eigen::Vector3d &omega) {
  auto a = differentiateNumerically(omega);
  auto b = differentiateAD(omega);
  EXPECT_EQ(a.size(), 3);
  EXPECT_EQ(b.size(), 3);
  for (int i = 0; i < 3; i++) {
    compareMats(a[i], b[i]);
  }
}

TEST(AxisAngleTest, DifTest) {
  compareDifs(Eigen::Vector3d(0, 0, 0));
  compareDifs(Eigen::Vector3d(0, 4.5, 0));
  compareDifs(Eigen::Vector3d(0, 4.5, 98.3));
  compareDifs(Eigen::Vector3d(1003, 4.5, 98.3));
}


