/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include "TimeStamp.h"

using namespace sail;

TEST(TimeStampTest, Arithmetics) {
  TimeStamp a(1986, 5, 14, 13, 5, 2.0);
  TimeStamp b(1986, 5, 14, 13, 5, 2.3);
  TimeStamp c(1986, 5, 14, 13, 5, 3.0);
  EXPECT_NEAR((c - a).seconds(), 1.0, 1.0e-6);
  EXPECT_NEAR((b - a).seconds(), 0.3, 1.0e-6);
  EXPECT_NEAR((a - c).seconds(), -1.0, 1.0e-6);
  EXPECT_NEAR((a - b).seconds(), -0.3, 1.0e-6);

  EXPECT_NEAR((c - (a + Duration<double>::seconds(1.0))).seconds(), 0.0, 1.0e-6);
  EXPECT_NEAR((a - (c - Duration<double>::seconds(1.0))).seconds(), 0.0, 1.0e-6);

  EXPECT_TRUE(a < b);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a >= b);
  EXPECT_TRUE(a <= a);
  EXPECT_TRUE(a >= a);
}



