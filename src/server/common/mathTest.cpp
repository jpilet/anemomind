/*
 *  Created on: 2014-03-31
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/math.h>


using namespace sail;

TEST(MathTest, AngleTest) {
  EXPECT_NEAR(localizeAngleRadians(1.5*M_PI, 0.0), -0.5*M_PI, 1.0e-6);
  EXPECT_NEAR(localizeAngleRadians(1.5*M_PI, 1.0*M_PI), 1.5*M_PI, 1.0e-6);
  EXPECT_NEAR(localizeAngleRadians(2.5*M_PI, 1.0*M_PI), 0.5*M_PI, 1.0e-6);
}


TEST(MathTest, NanBehavesAsExpected) {
  double a = std::numeric_limits<double>::signaling_NaN();
  double b = std::numeric_limits<double>::signaling_NaN();
  double c = 1.4;
  EXPECT_FALSE(a == b);
  EXPECT_FALSE(a == c);

  EXPECT_TRUE(strictEquality(a, b));
  EXPECT_FALSE(strictEquality(a, c));
  EXPECT_TRUE(strictEquality(c, c));
}





