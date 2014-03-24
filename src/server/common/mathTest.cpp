/*
 *  Created on: 24 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/math.h>

using namespace sail;

TEST(MathTest, AngleTest) {
  EXPECT_NEAR(angleAtRadians(1.5*M_PI, 0.0), -0.5*M_PI, 1.0e-6);
  EXPECT_NEAR(angleAtRadians(1.5*M_PI, 1.0*M_PI), 1.5*M_PI, 1.0e-6);
  EXPECT_NEAR(angleAtRadians(2.5*M_PI, 1.0*M_PI), 0.5*M_PI, 1.0e-6);
}
