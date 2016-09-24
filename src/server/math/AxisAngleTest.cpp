/*
 * AxisAngleTest.cpp
 *
 *  Created on: 24 Sep 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/AxisAngle.h>

using namespace sail;

TEST(AxisAngleTest, BasicRotations) {
  double k = 1.0/sqrt(2.0);
  {
    Eigen::Vector3d omega(0.0, 0.0, 0.25*M_PI);
    auto R = computeRotationFromOmega(omega);
    std::cout << "R is " << R << std::endl;
    EXPECT_NEAR(R(0, 0), k, 1.0e-6);
    EXPECT_NEAR(R(1, 0), k, 1.0e-6);
  }
}


