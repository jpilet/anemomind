/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include "TimeStamp.h"

using namespace sail;

TEST(TimeStampTest, Arithmetics) {
  TimeStamp a(1986, 5, 14, 13, 5, 2.0);
  TimeStamp b(1986, 5, 14, 13, 5, 2.25);
  TimeStamp c(1986, 5, 14, 13, 5, 3.0);
  EXPECT_NEAR((c - a).seconds(), 1.0, 1.0e-6);
  EXPECT_NEAR((b - a).seconds(), 0.25, 1.0e-6);
  EXPECT_NEAR((a - c).seconds(), -1.0, 1.0e-6);
  EXPECT_NEAR((a - b).seconds(), -0.25, 1.0e-6);

  EXPECT_NEAR((c - (a + Duration<double>::seconds(1.0))).seconds(), 0.0, 1.0e-6);
  EXPECT_NEAR((a - (c - Duration<double>::seconds(1.0))).seconds(), 0.0, 1.0e-6);

  EXPECT_TRUE(a < b);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a >= b);
  EXPECT_TRUE(a <= a);
  EXPECT_TRUE(a >= a);
}

TEST(TimeStampTest, Undef) {
  EXPECT_TRUE(TimeStamp::makeUndefined().undefined());
  EXPECT_TRUE(TimeStamp().undefined());
  EXPECT_FALSE(TimeStamp().defined());
}

TEST(TimeStampTest, Def) {
  EXPECT_TRUE(TimeStamp::now().defined());
}

TEST(TimeStampTest, MilliSeconds) {
  const int64_t ms = 516456302000;
  TimeStamp a(1986, 5, 14, 13, 5, 2.0, 0, 0);
  EXPECT_EQ(a.toMilliSecondsSince1970(), ms);
  EXPECT_EQ(TimeStamp::fromMilliSecondsSince1970(ms).toMilliSecondsSince1970(), ms);
}

