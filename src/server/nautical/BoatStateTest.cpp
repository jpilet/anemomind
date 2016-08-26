/*
 * BoatStateTest.cpp
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/BoatState.h>
#include <gtest/gtest.h>
#include <Eigen/Dense>

using namespace sail;

TEST(BoatStateTest, Orthonormality) {
  typedef BoatState<double> BS;

  auto expectedMast = BS::starboardVector().cross(BS::headingVector());
  EXPECT_NEAR((expectedMast - BS::mastVector()).norm(), 0.0, 1.0e-6);

  Eigen::Vector3d v[3] = {BS::starboardVector(), BS::headingVector(), BS::mastVector()};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      EXPECT_NEAR(v[i].dot(v[j]), i == j? 1.0 : 0.0, 1.0e-6);
    }
  }
}



