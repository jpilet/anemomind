/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include "TimeStamp.h"
#include <sstream>
#include <server/common/string.h>

using namespace sail;

TEST(TimeStampTest, Arithmetics) {
  TimeStamp a = TimeStamp::UTC(1986, 5, 14, 13, 5, 2.0);
  TimeStamp b = TimeStamp::UTC(1986, 5, 14, 13, 5, 2.25);
  TimeStamp c = TimeStamp::UTC(1986, 5, 14, 13, 5, 3.0);
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
  const int64_t ms = 516459902000;
  TimeStamp a = TimeStamp::UTC(1986, 5, 14, 13, 5, 2.0);
  EXPECT_EQ(a.toMilliSecondsSince1970(), ms);
  EXPECT_EQ(TimeStamp::fromMilliSecondsSince1970(ms).toMilliSecondsSince1970(), ms);
}

TEST(TimeStampTest, ToStreamTest) {

  int year = 2014;
  int month = 04;
  int day = 23;
  int hour = 14;
  int minute = 52;
  int second = 23;
  TimeStamp ts = TimeStamp::UTC(year, month, day,
      hour, minute, second);
  std::string expected = stringFormat("%04d-%02d-%02dT%02d:%02d:%02d", year, month, day,
      hour, minute, second);

  std::stringstream ss;
  ss << ts;
  EXPECT_EQ(expected, ss.str());
}

TEST(TimeStampTest, Sleep) {
  TimeStamp before = TimeStamp::now();
  Duration<> delta = Duration<>::seconds(.1);
  sleep(delta);
  TimeStamp after = TimeStamp::now();
  EXPECT_LE(before + delta, after);
}

#ifdef HAVE_CLOCK_GETTIME
TEST(TimeStampTest, MonotonicClock) {
  TimeStamp before = MonotonicClock::now();
  Duration<> delta = Duration<>::seconds(.1);
  sleep(delta);
  TimeStamp after = MonotonicClock::now();
  EXPECT_NEAR((after - before).seconds(), .1, .01);
}
#endif
